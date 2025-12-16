#include <utils/ErrorHandlerChain.h>
#include <utils/Logger.h>
#include <utils/ErrorCodes.h>
#include <cstring>

bool LoggingErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Определяем уровень логирования в зависимости от типа ошибки
    LogLevel level = LogLevel::ERROR;
    
    // Ошибки валидации логируем как WARNING
    if (result.error >= FilterError::InvalidImage && 
        result.error <= FilterError::EmptyImage)
    {
        level = LogLevel::WARNING;
    }

    const std::string full_message = result.getFullMessage();
    const std::string error_code_str = filterErrorToString(result.error);
    
    std::string log_message = "[" + error_code_str + "] " + full_message;
    
    Logger::log(level, log_message);

    // Передаем ошибку следующему обработчику
    if (next_)
    {
        return next_->handle(result);
    }
    
    return false;
}

bool ValidationErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Обрабатываем только ошибки валидации
    if (result.error >= FilterError::InvalidImage && 
        result.error <= FilterError::EmptyImage)
    {
        // Можно добавить дополнительную логику обработки ошибок валидации
        // Например, сбор статистики, отправка уведомлений и т.д.
        
        // Передаем следующему обработчику
        if (next_)
        {
            return next_->handle(result);
        }
        
        return true; // Ошибка обработана
    }

    // Не наша ошибка, передаем дальше
    if (next_)
    {
        return next_->handle(result);
    }
    
    return false;
}

bool FileOperationErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Обрабатываем только ошибки файловых операций
    if (result.error >= FilterError::FileOperationFailed && 
        result.error <= FilterError::InvalidFilePath)
    {
        // Можно добавить дополнительную логику обработки ошибок файловых операций
        // Например, повторные попытки, альтернативные пути и т.д.
        
        // Передаем следующему обработчику
        if (next_)
        {
            return next_->handle(result);
        }
        
        return true; // Ошибка обработана
    }

    // Не наша ошибка, передаем дальше
    if (next_)
    {
        return next_->handle(result);
    }
    
    return false;
}

bool MemoryErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Обрабатываем только ошибки памяти
    if (result.error >= FilterError::OutOfMemory && 
        result.error <= FilterError::MemoryOverflow)
    {
        // Можно добавить дополнительную логику обработки ошибок памяти
        // Например, попытки освободить память, сбор мусора и т.д.
        
        Logger::error("Критическая ошибка памяти: " + result.getFullMessage());
        
        // Передаем следующему обработчику
        if (next_)
        {
            return next_->handle(result);
        }
        
        return true; // Ошибка обработана
    }

    // Не наша ошибка, передаем дальше
    if (next_)
    {
        return next_->handle(result);
    }
    
    return false;
}

bool SystemErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Обрабатываем только системные ошибки
    if (result.error >= FilterError::SystemError && 
        result.error <= FilterError::InvalidSystemCall)
    {
        // Можно добавить дополнительную логику обработки системных ошибок
        // Например, логирование системных вызовов, диагностика и т.д.
        
        if (result.context.has_value() && 
            result.context.value().system_error_code.has_value())
        {
            const int errno_code = result.context.value().system_error_code.value();
            const char* errno_str = std::strerror(errno_code);
            if (errno_str != nullptr)
            {
                Logger::error("Системная ошибка (errno=" + std::to_string(errno_code) + 
                             "): " + std::string(errno_str));
            }
        }
        
        // Передаем следующему обработчику
        if (next_)
        {
            return next_->handle(result);
        }
        
        return true; // Ошибка обработана
    }

    // Не наша ошибка, передаем дальше
    if (next_)
    {
        return next_->handle(result);
    }
    
    return false;
}

bool DefaultErrorHandler::handle(const FilterResult& result)
{
    if (result.isSuccess())
    {
        return false;
    }

    // Обрабатываем все остальные ошибки
    Logger::error("Необработанная ошибка: " + result.getFullMessage());
    
    return true; // Ошибка обработана
}

ErrorHandlerChain::ErrorHandlerChain()
{
    handlers_.push_back(std::make_shared<LoggingErrorHandler>());
    handlers_.push_back(std::make_shared<ValidationErrorHandler>());
    handlers_.push_back(std::make_shared<FileOperationErrorHandler>());
    handlers_.push_back(std::make_shared<MemoryErrorHandler>());
    handlers_.push_back(std::make_shared<SystemErrorHandler>());
    handlers_.push_back(std::make_shared<DefaultErrorHandler>());

    // Связываем обработчики в цепочку
    for (size_t i = 0; i < handlers_.size() - 1; ++i)
    {
        handlers_[i]->setNext(handlers_[i + 1]);
    }

    first_handler_ = handlers_[0];
}

ErrorHandlerChain::ErrorHandlerChain(std::vector<std::shared_ptr<IErrorHandler>> handlers)
    : handlers_(std::move(handlers))
{
    if (handlers_.empty())
    {
        first_handler_ = nullptr;
        return;
    }

    // Связываем обработчики в цепочку
    for (size_t i = 0; i < handlers_.size() - 1; ++i)
    {
        handlers_[i]->setNext(handlers_[i + 1]);
    }

    first_handler_ = handlers_[0];
}

bool ErrorHandlerChain::process(const FilterResult& result)
{
    if (!first_handler_)
    {
        return false;
    }

    return first_handler_->handle(result);
}

void ErrorHandlerChain::addHandler(std::shared_ptr<IErrorHandler> handler)
{
    if (!handler)
    {
        return;
    }

    if (handlers_.empty())
    {
        handlers_.push_back(handler);
        first_handler_ = handler;
        return;
    }

    // Добавляем в конец цепочки
    handlers_.back()->setNext(handler);
    handlers_.push_back(handler);
}

ErrorHandlerChain ErrorHandlerChain::createDefault()
{
    return ErrorHandlerChain();
}

