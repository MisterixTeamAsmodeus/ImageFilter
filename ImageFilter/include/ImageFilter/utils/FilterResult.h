#pragma once

#include <utils/ErrorCodes.h>
#include <string>
#include <optional>
#include <cstring>

/**
 * @brief Контекст ошибки для детальной диагностики
 * 
 * Содержит дополнительную информацию об ошибке:
 * - Имя файла (если применимо)
 * - Размеры изображения
 * - Параметры фильтра
 * - Системный код ошибки (errno)
 */
struct ErrorContext
{
    /**
     * @brief Имя файла, связанного с ошибкой
     */
    std::optional<std::string> filename;

    /**
     * @brief Ширина изображения
     */
    std::optional<int> image_width;

    /**
     * @brief Высота изображения
     */
    std::optional<int> image_height;

    /**
     * @brief Количество каналов изображения
     */
    std::optional<int> image_channels;

    /**
     * @brief Параметры фильтра (например, "radius=5.0, factor=1.2")
     */
    std::optional<std::string> filter_params;

    /**
     * @brief Системный код ошибки (errno)
     */
    std::optional<int> system_error_code;

    /**
     * @brief Конструктор по умолчанию
     */
    ErrorContext() = default;

    /**
     * @brief Создает контекст с информацией об изображении
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов
     * @return ErrorContext с информацией об изображении
     */
    static ErrorContext withImage(int width, int height, int channels)
    {
        ErrorContext ctx;
        ctx.image_width = width;
        ctx.image_height = height;
        ctx.image_channels = channels;
        return ctx;
    }

    /**
     * @brief Создает контекст с именем файла
     * @param filename Имя файла
     * @return ErrorContext с именем файла
     */
    static ErrorContext withFilename(const std::string& filename)
    {
        ErrorContext ctx;
        ctx.filename = filename;
        return ctx;
    }

    /**
     * @brief Создает контекст с системной ошибкой
     * @param errno_code Код системной ошибки (errno)
     * @return ErrorContext с системной ошибкой
     */
    static ErrorContext withSystemError(int errno_code)
    {
        ErrorContext ctx;
        ctx.system_error_code = errno_code;
        return ctx;
    }

    /**
     * @brief Добавляет параметр фильтра в контекст ошибки
     * 
     * Если параметры уже существуют, новый параметр добавляется через запятую.
     * 
     * @param param_name Имя параметра
     * @param param_value Значение параметра
     * @return Ссылка на себя для цепочки вызовов
     */
    ErrorContext& withFilterParam(const std::string& param_name, const std::string& param_value)
    {
        const std::string param_str = param_name + "=" + param_value;
        if (filter_params.has_value())
        {
            filter_params = filter_params.value() + ", " + param_str;
        }
        else
        {
            filter_params = param_str;
        }
        return *this;
    }

    /**
     * @brief Добавляет параметр фильтра (строковый литерал) в контекст ошибки
     * 
     * @param param_name Имя параметра
     * @param param_value Значение параметра (строковый литерал)
     * @return Ссылка на себя для цепочки вызовов
     */
    ErrorContext& withFilterParam(const std::string& param_name, const char* param_value)
    {
        return withFilterParam(param_name, std::string(param_value));
    }

    /**
     * @brief Добавляет параметр фильтра (числовой) в контекст ошибки
     * 
     * @param param_name Имя параметра
     * @param param_value Значение параметра
     * @return Ссылка на себя для цепочки вызовов
     */
    template<typename T>
    ErrorContext& withFilterParam(const std::string& param_name, T param_value)
    {
        return withFilterParam(param_name, std::to_string(param_value));
    }

    /**
     * @brief Форматирует контекст в строку для сообщения об ошибке
     * @return Строковое представление контекста
     */
    [[nodiscard]] std::string toString() const
    {
        std::string result;
        bool first = true;

        if (filename.has_value())
        {
            result += "Файл: " + filename.value();
            first = false;
        }

        if (image_width.has_value() && image_height.has_value() && image_channels.has_value())
        {
            if (!first) result += ", ";
            result += "Размер: " + std::to_string(image_width.value()) + "x" +
                      std::to_string(image_height.value()) + ", каналов: " +
                      std::to_string(image_channels.value());
            first = false;
        }

        if (filter_params.has_value())
        {
            if (!first) result += ", ";
            result += "Параметры: " + filter_params.value();
            first = false;
        }

        if (system_error_code.has_value())
        {
            if (!first) result += ", ";
            result += "Системная ошибка: " + std::to_string(system_error_code.value());
            const char* errno_str = std::strerror(system_error_code.value());
            if (errno_str != nullptr)
            {
                result += " (" + std::string(errno_str) + ")";
            }
        }

        return result;
    }
};

/**
 * @brief Результат применения фильтра
 * 
 * Содержит код ошибки, опциональное сообщение об ошибке и контекст ошибки.
 * Используется для структурированной обработки ошибок вместо простого bool.
 * 
 * @note Поля упорядочены для минимизации padding: сначала объекты с большим выравниванием
 * (std::string, std::optional), затем enum с меньшим выравниванием.
 */
struct FilterResult
{
    /**
     * @brief Опциональное сообщение об ошибке
     */
    std::string message;

    /**
     * @brief Контекст ошибки для детальной диагностики
     */
    std::optional<ErrorContext> context;

    /**
     * @brief Код ошибки
     */
    FilterError error;

    /**
     * @brief Конструктор для успешного результата
     */
    FilterResult() : message(), context(std::nullopt), error(FilterError::Success) {}

    /**
     * @brief Конструктор для результата с ошибкой
     * @param err Код ошибки
     * @param msg Сообщение об ошибке (опционально)
     * @param ctx Контекст ошибки (опционально)
     */
    FilterResult(FilterError err, const std::string& msg = "", 
                 const std::optional<ErrorContext>& ctx = std::nullopt) 
        : message(msg), context(ctx), error(err) {}

    /**
     * @brief Проверяет, успешен ли результат
     * @return true если операция успешна
     */
    [[nodiscard]] bool isSuccess() const noexcept
    {
        return error == FilterError::Success;
    }

    /**
     * @brief Проверяет, есть ли ошибка
     * @return true если есть ошибка
     */
    [[nodiscard]] bool hasError() const noexcept
    {
        return error != FilterError::Success;
    }

    /**
     * @brief Получает полное сообщение об ошибке с контекстом
     * @return Полное сообщение об ошибке
     */
    [[nodiscard]] std::string getFullMessage() const
    {
        if (isSuccess())
        {
            return "Успешно";
        }

        std::string full_msg = message.empty() ? "" : message;
        
        if (context.has_value())
        {
            const std::string ctx_str = context.value().toString();
            if (!ctx_str.empty())
            {
                if (!full_msg.empty())
                {
                    full_msg += ". ";
                }
                full_msg += ctx_str;
            }
        }

        return full_msg;
    }

    /**
     * @brief Создает успешный результат
     * @return FilterResult с кодом Success
     */
    static FilterResult success()
    {
        return FilterResult(FilterError::Success);
    }

    /**
     * @brief Создает результат с ошибкой
     * @param err Код ошибки
     * @param msg Сообщение об ошибке
     * @param ctx Контекст ошибки (опционально)
     * @return FilterResult с ошибкой
     */
    static FilterResult failure(FilterError err, const std::string& msg = "",
                                const std::optional<ErrorContext>& ctx = std::nullopt)
    {
        return FilterResult(err, msg, ctx);
    }

    /**
     * @brief Создает результат с ошибкой и контекстом изображения
     * @param err Код ошибки
     * @param msg Сообщение об ошибке
     * @param width Ширина изображения
     * @param height Высота изображения
     * @param channels Количество каналов
     * @return FilterResult с ошибкой и контекстом
     */
    static FilterResult failureWithImage(FilterError err, const std::string& msg,
                                         int width, int height, int channels)
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, channels);
        return FilterResult(err, msg, ctx);
    }

    /**
     * @brief Создает результат с ошибкой и именем файла
     * @param err Код ошибки
     * @param msg Сообщение об ошибке
     * @param filename Имя файла
     * @return FilterResult с ошибкой и контекстом
     */
    static FilterResult failureWithFilename(FilterError err, const std::string& msg,
                                            const std::string& filename)
    {
        ErrorContext ctx = ErrorContext::withFilename(filename);
        return FilterResult(err, msg, ctx);
    }

    /**
     * @brief Создает результат с системной ошибкой
     * @param err Код ошибки
     * @param msg Сообщение об ошибке
     * @param errno_code Код системной ошибки (errno)
     * @return FilterResult с ошибкой и системным кодом
     */
    static FilterResult failureWithSystemError(FilterError err, const std::string& msg,
                                               int errno_code)
    {
        ErrorContext ctx = ErrorContext::withSystemError(errno_code);
        return FilterResult(err, msg, ctx);
    }
};

/**
 * @brief Преобразует FilterError в строковое описание
 * @param error Код ошибки
 * @return Строковое описание ошибки
 */
std::string filterErrorToString(FilterError error);
