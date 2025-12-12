#include <gtest/gtest.h>
#include <utils/ErrorHandlerChain.h>
#include <utils/FilterResult.h>
#include <utils/ErrorCodes.h>
#include <memory>
#include <cerrno>

/**
 * @brief Тесты для ErrorHandlerChain
 */
class ErrorHandlerChainTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }
};

// Тест создания цепочки по умолчанию
TEST_F(ErrorHandlerChainTest, DefaultChain)
{
    ErrorHandlerChain chain;
    
    // Цепочка должна обрабатывать любую ошибку
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Тестовая ошибка");
    EXPECT_TRUE(chain.process(error));
}

// Тест обработки ошибок валидации
TEST_F(ErrorHandlerChainTest, ValidationError)
{
    ErrorHandlerChain chain;
    
    FilterResult validation_error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка валидации");
    EXPECT_TRUE(chain.process(validation_error));
    
    FilterResult empty_image = FilterResult::failure(FilterError::EmptyImage, "Пустое изображение");
    EXPECT_TRUE(chain.process(empty_image));
}

// Тест обработки ошибок файловых операций
TEST_F(ErrorHandlerChainTest, FileOperationError)
{
    ErrorHandlerChain chain;
    
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Файл не найден");
    EXPECT_TRUE(chain.process(file_error));
    
    FilterResult write_error = FilterResult::failure(FilterError::FileWriteError, "Ошибка записи");
    EXPECT_TRUE(chain.process(write_error));
}

// Тест обработки ошибок памяти
TEST_F(ErrorHandlerChainTest, MemoryError)
{
    ErrorHandlerChain chain;
    
    FilterResult memory_error = FilterResult::failure(FilterError::OutOfMemory, "Недостаточно памяти");
    EXPECT_TRUE(chain.process(memory_error));
    
    FilterResult alloc_error = FilterResult::failure(FilterError::MemoryAllocationFailed, "Ошибка выделения");
    EXPECT_TRUE(chain.process(alloc_error));
}

// Тест обработки системных ошибок
TEST_F(ErrorHandlerChainTest, SystemError)
{
    ErrorHandlerChain chain;
    
    FilterResult system_error = FilterResult::failureWithSystemError(
        FilterError::SystemError, "Системная ошибка", ENOENT);
    EXPECT_TRUE(chain.process(system_error));
}

// Тест обработки успешного результата
TEST_F(ErrorHandlerChainTest, SuccessResult)
{
    ErrorHandlerChain chain;
    
    FilterResult success = FilterResult::success();
    // Успешный результат не должен обрабатываться как ошибка
    EXPECT_FALSE(chain.process(success));
}

// Тест пользовательской цепочки
TEST_F(ErrorHandlerChainTest, CustomChain)
{
    auto handler1 = std::make_shared<DefaultErrorHandler>();
    auto handler2 = std::make_shared<DefaultErrorHandler>();
    
    std::vector<std::shared_ptr<IErrorHandler>> handlers;
    handlers.push_back(handler1);
    handlers.push_back(handler2);
    
    ErrorHandlerChain chain(handlers);
    
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(chain.process(error));
}

// Тест пустой цепочки
TEST_F(ErrorHandlerChainTest, EmptyChain)
{
    std::vector<std::shared_ptr<IErrorHandler>> handlers;
    ErrorHandlerChain chain(handlers);
    
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_FALSE(chain.process(error));
}

// Тест добавления обработчика
TEST_F(ErrorHandlerChainTest, AddHandler)
{
    ErrorHandlerChain chain;
    
    auto custom_handler = std::make_shared<DefaultErrorHandler>();
    chain.addHandler(custom_handler);
    
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(chain.process(error));
}

// Тест добавления nullptr обработчика
TEST_F(ErrorHandlerChainTest, AddNullHandler)
{
    ErrorHandlerChain chain;
    
    chain.addHandler(nullptr);
    
    // Цепочка должна продолжать работать
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(chain.process(error));
}

// Тест createDefault
TEST_F(ErrorHandlerChainTest, CreateDefault)
{
    auto chain = ErrorHandlerChain::createDefault();
    
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(chain.process(error));
}

// Тест отдельных обработчиков - LoggingErrorHandler
TEST_F(ErrorHandlerChainTest, LoggingErrorHandler)
{
    auto handler = std::make_shared<LoggingErrorHandler>();
    auto next_handler = std::make_shared<DefaultErrorHandler>();
    handler->setNext(next_handler);
    
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    // LoggingErrorHandler всегда передает дальше
    EXPECT_TRUE(handler->handle(error));
}

// Тест отдельных обработчиков - ValidationErrorHandler
TEST_F(ErrorHandlerChainTest, ValidationErrorHandler)
{
    auto handler = std::make_shared<ValidationErrorHandler>();
    
    // Ошибка валидации должна обрабатываться
    FilterResult validation_error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(handler->handle(validation_error));
    
    // Другая ошибка не должна обрабатываться этим обработчиком
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_FALSE(handler->handle(file_error));
}

// Тест отдельных обработчиков - FileOperationErrorHandler
TEST_F(ErrorHandlerChainTest, FileOperationErrorHandler)
{
    auto handler = std::make_shared<FileOperationErrorHandler>();
    
    // Ошибка файловой операции должна обрабатываться
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_TRUE(handler->handle(file_error));
    
    // Другая ошибка не должна обрабатываться
    FilterResult validation_error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_FALSE(handler->handle(validation_error));
}

// Тест отдельных обработчиков - MemoryErrorHandler
TEST_F(ErrorHandlerChainTest, MemoryErrorHandler)
{
    auto handler = std::make_shared<MemoryErrorHandler>();
    
    // Ошибка памяти должна обрабатываться
    FilterResult memory_error = FilterResult::failure(FilterError::OutOfMemory, "Ошибка");
    EXPECT_TRUE(handler->handle(memory_error));
    
    // Другая ошибка не должна обрабатываться
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_FALSE(handler->handle(file_error));
}

// Тест отдельных обработчиков - SystemErrorHandler
TEST_F(ErrorHandlerChainTest, SystemErrorHandler)
{
    auto handler = std::make_shared<SystemErrorHandler>();
    
    // Системная ошибка должна обрабатываться
    FilterResult system_error = FilterResult::failureWithSystemError(
        FilterError::SystemError, "Ошибка", ENOENT);
    EXPECT_TRUE(handler->handle(system_error));
    
    // Другая ошибка не должна обрабатываться
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_FALSE(handler->handle(file_error));
}

// Тест отдельных обработчиков - DefaultErrorHandler
TEST_F(ErrorHandlerChainTest, DefaultErrorHandler)
{
    auto handler = std::make_shared<DefaultErrorHandler>();
    
    // DefaultErrorHandler должен обрабатывать любую ошибку
    FilterResult error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(handler->handle(error));
    
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_TRUE(handler->handle(file_error));
}

// Тест цепочки обработчиков
TEST_F(ErrorHandlerChainTest, HandlerChain)
{
    auto handler1 = std::make_shared<LoggingErrorHandler>();
    auto handler2 = std::make_shared<ValidationErrorHandler>();
    auto handler3 = std::make_shared<DefaultErrorHandler>();
    
    handler1->setNext(handler2);
    handler2->setNext(handler3);
    
    // Ошибка валидации должна быть обработана handler2
    FilterResult validation_error = FilterResult::failure(FilterError::InvalidParameter, "Ошибка");
    EXPECT_TRUE(handler1->handle(validation_error));
    
    // Ошибка файла должна пройти через handler1 и handler2, обработана handler3
    FilterResult file_error = FilterResult::failure(FilterError::FileNotFound, "Ошибка");
    EXPECT_TRUE(handler1->handle(file_error));
}

// Тест getNext
TEST_F(ErrorHandlerChainTest, GetNext)
{
    auto handler1 = std::make_shared<DefaultErrorHandler>();
    auto handler2 = std::make_shared<DefaultErrorHandler>();
    
    handler1->setNext(handler2);
    
    EXPECT_EQ(handler1->getNext(), handler2);
    EXPECT_EQ(handler2->getNext(), nullptr);
}

