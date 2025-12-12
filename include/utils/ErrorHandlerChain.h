#pragma once

#include <utils/FilterResult.h>
#include <memory>
#include <vector>

/**
 * @brief Базовый интерфейс для обработчика ошибок в цепочке
 * 
 * Реализует паттерн Chain of Responsibility для обработки ошибок.
 * Каждый обработчик может либо обработать ошибку, либо передать её следующему в цепочке.
 */
class IErrorHandler
{
public:
    /**
     * @brief Виртуальный деструктор
     */
    virtual ~IErrorHandler() = default;

    /**
     * @brief Обрабатывает ошибку
     * @param result Результат операции с ошибкой
     * @return true если ошибка была обработана, false если нужно передать следующему обработчику
     * 
     * Если обработчик возвращает true, цепочка прерывается и ошибка считается обработанной.
     * Если возвращает false, ошибка передается следующему обработчику в цепочке.
     */
    virtual bool handle(const FilterResult& result) = 0;

    /**
     * @brief Устанавливает следующий обработчик в цепочке
     * @param next Следующий обработчик
     */
    void setNext(std::shared_ptr<IErrorHandler> next)
    {
        next_ = next;
    }

    /**
     * @brief Получает следующий обработчик в цепочке
     * @return Следующий обработчик или nullptr
     */
    [[nodiscard]] std::shared_ptr<IErrorHandler> getNext() const
    {
        return next_;
    }

protected:
    std::shared_ptr<IErrorHandler> next_;
};

/**
 * @brief Обработчик ошибок, который логирует ошибки
 * 
 * Логирует все ошибки с соответствующим уровнем логирования.
 */
class LoggingErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает ошибку, логируя её
     * @param result Результат операции с ошибкой
     * @return false (всегда передает ошибку следующему обработчику)
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Обработчик ошибок, который обрабатывает ошибки валидации
 * 
 * Специализируется на обработке ошибок валидации параметров и данных.
 */
class ValidationErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает ошибки валидации
     * @param result Результат операции с ошибкой
     * @return true если ошибка валидации обработана, false иначе
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Обработчик ошибок, который обрабатывает ошибки файловых операций
 * 
 * Специализируется на обработке ошибок чтения/записи файлов.
 */
class FileOperationErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает ошибки файловых операций
     * @param result Результат операции с ошибкой
     * @return true если ошибка файловой операции обработана, false иначе
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Обработчик ошибок, который обрабатывает ошибки памяти
 * 
 * Специализируется на обработке ошибок выделения памяти.
 */
class MemoryErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает ошибки памяти
     * @param result Результат операции с ошибкой
     * @return true если ошибка памяти обработана, false иначе
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Обработчик ошибок, который обрабатывает системные ошибки
 * 
 * Специализируется на обработке системных ошибок (errno).
 */
class SystemErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает системные ошибки
     * @param result Результат операции с ошибкой
     * @return true если системная ошибка обработана, false иначе
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Обработчик ошибок по умолчанию (fallback)
 * 
 * Обрабатывает все ошибки, которые не были обработаны предыдущими обработчиками.
 */
class DefaultErrorHandler : public IErrorHandler
{
public:
    /**
     * @brief Обрабатывает любую ошибку
     * @param result Результат операции с ошибкой
     * @return true (всегда обрабатывает ошибку)
     */
    bool handle(const FilterResult& result) override;
};

/**
 * @brief Менеджер цепочки обработчиков ошибок
 * 
 * Упрощает создание и использование цепочки обработчиков ошибок.
 */
class ErrorHandlerChain
{
public:
    /**
     * @brief Конструктор - создает цепочку обработчиков по умолчанию
     * 
     * Создает цепочку: Logging -> Validation -> FileOperation -> Memory -> System -> Default
     */
    ErrorHandlerChain();

    /**
     * @brief Конструктор с пользовательской цепочкой
     * @param handlers Вектор обработчиков (порядок важен)
     */
    explicit ErrorHandlerChain(std::vector<std::shared_ptr<IErrorHandler>> handlers);

    /**
     * @brief Обрабатывает ошибку через цепочку обработчиков
     * @param result Результат операции с ошибкой
     * @return true если ошибка была обработана, false если нет
     */
    bool process(const FilterResult& result);

    /**
     * @brief Добавляет обработчик в конец цепочки
     * @param handler Обработчик для добавления
     */
    void addHandler(std::shared_ptr<IErrorHandler> handler);

    /**
     * @brief Создает цепочку обработчиков по умолчанию
     * @return Цепочка обработчиков
     */
    static ErrorHandlerChain createDefault();

private:
    std::shared_ptr<IErrorHandler> first_handler_;
    std::vector<std::shared_ptr<IErrorHandler>> handlers_;
};

