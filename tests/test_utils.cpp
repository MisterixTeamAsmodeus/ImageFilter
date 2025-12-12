#include "test_utils.h"
#include <ImageProcessor.h>
#include <utils/BorderHandler.h>
#include <utils/BatchProcessor.h>
#include <utils/FilterResult.h>
#include <filters/GrayscaleFilter.h>
#include <filters/BrightnessFilter.h>
#include <filters/InvertFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/FlipHorizontalFilter.h>
#include <filters/SepiaFilter.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

namespace TestUtils
{
    std::vector<uint8_t> createTestImage(int width, int height)
    {
        std::vector<uint8_t> image(static_cast<size_t>(width) * height * 3);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const auto index = static_cast<size_t>(y * width + x) * 3;
                image[index + 0] = static_cast<uint8_t>(x * 85); // R
                image[index + 1] = static_cast<uint8_t>(y * 85); // G
                image[index + 2] = static_cast<uint8_t>((x + y) * 42); // B
            }
        }
        return image;
    }

    std::vector<uint8_t> createTestImageRGBA(int width, int height)
    {
        std::vector<uint8_t> image(static_cast<size_t>(width) * height * 4);
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                const auto index = static_cast<size_t>(y * width + x) * 4;
                image[index + 0] = static_cast<uint8_t>(x * 85); // R
                image[index + 1] = static_cast<uint8_t>(y * 85); // G
                image[index + 2] = static_cast<uint8_t>((x + y) * 42); // B
                // Альфа-канал: полная непрозрачность в центре, прозрачность по краям
                const auto center_x = width / 2.0;
                const auto center_y = height / 2.0;
                const auto dx = x - center_x;
                const auto dy = y - center_y;
                const auto distance = std::sqrt(dx * dx + dy * dy);
                const auto max_distance = std::sqrt(center_x * center_x + center_y * center_y);
                const auto alpha = static_cast<uint8_t>(255 * (1.0 - distance / max_distance * 0.5));
                image[index + 3] = alpha; // A
            }
        }
        return image;
    }

    std::string saveTestImagePNG(
        const std::vector<uint8_t>& data,
        int width,
        int height,
        const std::string& filepath
    )
    {
        // Используем ImageProcessor для сохранения, чтобы избежать проблем с STB_IMAGE_WRITE_IMPLEMENTATION
        ImageProcessor processor;
        
        // Определяем количество каналов на основе размера данных
        const auto expected_size_rgb = static_cast<size_t>(width) * height * 3;
        const auto expected_size_rgba = static_cast<size_t>(width) * height * 4;
        const bool has_alpha = (data.size() == expected_size_rgba);
        const int channels = has_alpha ? 4 : 3;
        
        // Выделяем память и копируем данные
        auto* allocated_data = static_cast<uint8_t*>(std::malloc(data.size()));
        if (allocated_data == nullptr)
        {
            return "";
        }
        std::memcpy(allocated_data, data.data(), data.size());
        
        // Устанавливаем данные в процессор с указанием количества каналов
        const auto resize_result = processor.resize(width, height, channels, allocated_data);
        if (!resize_result.isSuccess())
        {
            std::free(allocated_data);
            return "";
        }
        
        // Сохраняем через ImageProcessor с сохранением альфа-канала, если он есть
        const auto save_result = processor.saveToFile(filepath, has_alpha);
        if (!save_result.isSuccess())
        {
            return "";
        }
        
        return filepath;
    }
}

/**
 * @brief Тесты для BorderHandler
 */
class BorderHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }
};

// Тест стратегии Mirror
TEST_F(BorderHandlerTest, MirrorStrategy)
{
    BorderHandler handler(BorderHandler::Strategy::Mirror);
    constexpr int width = 10;
    
    // Координаты внутри диапазона
    EXPECT_EQ(handler.getX(5, width), 5);
    EXPECT_EQ(handler.getX(0, width), 0);
    EXPECT_EQ(handler.getX(9, width), 9);
    
    // Координаты за границами - отражение
    EXPECT_EQ(handler.getX(-1, width), 1);  // -1 отражается в 1
    EXPECT_EQ(handler.getX(-2, width), 2);  // -2 отражается в 2
    EXPECT_EQ(handler.getX(10, width), 9);    // 10 отражается в 9
    EXPECT_EQ(handler.getX(11, width), 8);   // 11 отражается в 8
    EXPECT_EQ(handler.getX(19, width), 0);  // 19 отражается в 0
}

// Тест стратегии Clamp
TEST_F(BorderHandlerTest, ClampStrategy)
{
    BorderHandler handler(BorderHandler::Strategy::Clamp);
    constexpr int width = 10;
    
    // Координаты внутри диапазона
    EXPECT_EQ(handler.getX(5, width), 5);
    EXPECT_EQ(handler.getX(0, width), 0);
    EXPECT_EQ(handler.getX(9, width), 9);
    
    // Координаты за границами - зажим
    EXPECT_EQ(handler.getX(-1, width), 0);   // Зажимается к 0
    EXPECT_EQ(handler.getX(-10, width), 0); // Зажимается к 0
    EXPECT_EQ(handler.getX(10, width), 9); // Зажимается к 9
    EXPECT_EQ(handler.getX(100, width), 9); // Зажимается к 9
}

// Тест стратегии Wrap
TEST_F(BorderHandlerTest, WrapStrategy)
{
    BorderHandler handler(BorderHandler::Strategy::Wrap);
    constexpr int width = 10;
    
    // Координаты внутри диапазона
    EXPECT_EQ(handler.getX(5, width), 5);
    EXPECT_EQ(handler.getX(0, width), 0);
    EXPECT_EQ(handler.getX(9, width), 9);
    
    // Координаты за границами - циклическое повторение
    EXPECT_EQ(handler.getX(10, width), 0);  // 10 -> 0
    EXPECT_EQ(handler.getX(11, width), 1);  // 11 -> 1
    EXPECT_EQ(handler.getX(20, width), 0);  // 20 -> 0
    EXPECT_EQ(handler.getX(-1, width), 9);  // -1 -> 9
    EXPECT_EQ(handler.getX(-2, width), 8);  // -2 -> 8
    EXPECT_EQ(handler.getX(-10, width), 0); // -10 -> 0
    EXPECT_EQ(handler.getX(-11, width), 9); // -11 -> 9
}

// Тест стратегии Extend
TEST_F(BorderHandlerTest, ExtendStrategy)
{
    BorderHandler handler(BorderHandler::Strategy::Extend);
    constexpr int width = 10;
    
    // Координаты внутри диапазона
    EXPECT_EQ(handler.getX(5, width), 5);
    EXPECT_EQ(handler.getX(0, width), 0);
    EXPECT_EQ(handler.getX(9, width), 9);
    
    // Координаты за границами - расширение граничных значений
    EXPECT_EQ(handler.getX(-1, width), 0);   // Расширяется к 0
    EXPECT_EQ(handler.getX(-10, width), 0);  // Расширяется к 0
    EXPECT_EQ(handler.getX(10, width), 9);   // Расширяется к 9
    EXPECT_EQ(handler.getX(100, width), 9);  // Расширяется к 9
}

// Тест getY с различными стратегиями
TEST_F(BorderHandlerTest, GetYWithStrategies)
{
    constexpr int height = 10;
    
    // Mirror
    BorderHandler mirror_handler(BorderHandler::Strategy::Mirror);
    EXPECT_EQ(mirror_handler.getY(-1, height), 1);
    EXPECT_EQ(mirror_handler.getY(10, height), 9);
    
    // Clamp
    BorderHandler clamp_handler(BorderHandler::Strategy::Clamp);
    EXPECT_EQ(clamp_handler.getY(-1, height), 0);
    EXPECT_EQ(clamp_handler.getY(10, height), 9);
    
    // Wrap
    BorderHandler wrap_handler(BorderHandler::Strategy::Wrap);
    EXPECT_EQ(wrap_handler.getY(-1, height), 9);
    EXPECT_EQ(wrap_handler.getY(10, height), 0);
    
    // Extend
    BorderHandler extend_handler(BorderHandler::Strategy::Extend);
    EXPECT_EQ(extend_handler.getY(-1, height), 0);
    EXPECT_EQ(extend_handler.getY(10, height), 9);
}

// Тест изменения стратегии
TEST_F(BorderHandlerTest, ChangeStrategy)
{
    BorderHandler handler(BorderHandler::Strategy::Mirror);
    EXPECT_EQ(handler.getStrategy(), BorderHandler::Strategy::Mirror);
    
    handler.setStrategy(BorderHandler::Strategy::Clamp);
    EXPECT_EQ(handler.getStrategy(), BorderHandler::Strategy::Clamp);
    
    constexpr int width = 10;
    EXPECT_EQ(handler.getX(-1, width), 0); // Теперь использует Clamp
}

// Тест с нулевой шириной/высотой
TEST_F(BorderHandlerTest, ZeroDimensions)
{
    BorderHandler handler;
    
    EXPECT_EQ(handler.getX(5, 0), 0);
    EXPECT_EQ(handler.getY(5, 0), 0);
    EXPECT_EQ(handler.getX(-1, 0), 0);
    EXPECT_EQ(handler.getY(-1, 0), 0);
}

// Тест с отрицательной шириной/высотой
TEST_F(BorderHandlerTest, NegativeDimensions)
{
    BorderHandler handler;
    
    EXPECT_EQ(handler.getX(5, -10), 0);
    EXPECT_EQ(handler.getY(5, -10), 0);
}

/**
 * @brief Тесты для BatchProcessor
 */
class BatchProcessorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = std::filesystem::temp_directory_path() / "ImageFilterBatchTests";
        input_dir_ = test_dir_ / "input";
        output_dir_ = test_dir_ / "output";
        
        std::filesystem::create_directories(input_dir_);
        std::filesystem::create_directories(output_dir_);
        
        // Создаем тестовые изображения
        createTestImages();
    }
    
    void TearDown() override
    {
        if (std::filesystem::exists(test_dir_))
        {
            std::filesystem::remove_all(test_dir_);
        }
    }
    
    void createTestImages()
    {
        // Создаем несколько тестовых изображений
        for (int i = 1; i <= 3; ++i)
        {
            constexpr int width = 10;
            constexpr int height = 10;
            auto test_image = TestUtils::createTestImage(width, height);
            const auto filepath = input_dir_ / ("test" + std::to_string(i) + ".png");
            TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
        }
    }
    
    std::filesystem::path test_dir_;
    std::filesystem::path input_dir_;
    std::filesystem::path output_dir_;
};

// Тест поиска изображений
TEST_F(BatchProcessorTest, FindImages)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    auto images = processor.findImages();
    
    EXPECT_GE(images.size(), 3); // Должно найти минимум 3 изображения
}

// Тест обработки всех изображений
TEST_F(BatchProcessorTest, ProcessAll)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    size_t processed_count = 0;
    auto process_func = [&processed_count](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        processed_count++;
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_EQ(stats.processed_files, processed_count);
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест обработки с callback прогресса
TEST_F(BatchProcessorTest, ProcessAllWithProgress)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    size_t callback_count = 0;
    auto progress_callback = [&callback_count](const ProgressInfo& info)
    {
        callback_count++;
        EXPECT_GT(info.current, 0);
        EXPECT_LE(info.current, info.total);
        EXPECT_GE(info.percentage, 0.0);
        EXPECT_LE(info.percentage, 100.0);
    };
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func, progress_callback);
    
    EXPECT_EQ(callback_count, stats.total_files);
}

// Тест обработки с ошибкой
TEST_F(BatchProcessorTest, ProcessAllWithError)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string&, const std::string&) -> FilterResult
    {
        return FilterResult::failure(FilterError::InvalidParameter, "Тестовая ошибка");
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_EQ(stats.failed_files, stats.total_files);
    EXPECT_EQ(stats.processed_files, 0);
}

// Тест рекурсивного поиска
TEST_F(BatchProcessorTest, RecursiveSearch)
{
    // Создаем поддиректорию с изображением
    auto subdir = input_dir_ / "subdir";
    std::filesystem::create_directories(subdir);
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = subdir / "sub_test.png";
    TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string(), true); // Рекурсивно
    auto images = processor.findImages();
    
    EXPECT_GE(images.size(), 4); // Должно найти изображения в поддиректории тоже
}

// Тест фильтрации по шаблону
TEST_F(BatchProcessorTest, PatternFiltering)
{
    // Создаем файл с другим расширением
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto jpg_path = input_dir_ / "test.jpg";
    TestUtils::saveTestImagePNG(test_image, width, height, jpg_path.string());
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string(), false, "*.png");
    auto images = processor.findImages();
    
    // Должны найти только PNG файлы
    for (const auto& img : images)
    {
        EXPECT_EQ(img.extension(), ".png");
    }
}

// Тест isImageFile
TEST_F(BatchProcessorTest, IsImageFile)
{
    EXPECT_TRUE(BatchProcessor::isImageFile("test.jpg"));
    EXPECT_TRUE(BatchProcessor::isImageFile("test.png"));
    EXPECT_TRUE(BatchProcessor::isImageFile("test.JPG"));
    EXPECT_TRUE(BatchProcessor::isImageFile("test.PNG"));
    EXPECT_FALSE(BatchProcessor::isImageFile("test.txt"));
    EXPECT_FALSE(BatchProcessor::isImageFile("test"));
}

// Тест matchesPattern
TEST_F(BatchProcessorTest, MatchesPattern)
{
    EXPECT_TRUE(BatchProcessor::matchesPattern("test.jpg", "*.jpg"));
    EXPECT_TRUE(BatchProcessor::matchesPattern("test.png", "*.png"));
    EXPECT_FALSE(BatchProcessor::matchesPattern("test.jpg", "*.png"));
    EXPECT_FALSE(BatchProcessor::matchesPattern("test.txt", "*.jpg"));
}

// Тест getRelativePath
TEST_F(BatchProcessorTest, GetRelativePath)
{
    std::filesystem::path base = "/base";
    std::filesystem::path full = "/base/sub/file.png";
    
    auto relative = BatchProcessor::getRelativePath(full, base);
    EXPECT_EQ(relative.string(), "sub/file.png");
}

// Тест обработки с пустым списком файлов
TEST_F(BatchProcessorTest, ProcessAllEmptyList)
{
    // Создаем пустую директорию
    auto empty_input_dir = test_dir_ / "empty_input";
    std::filesystem::create_directories(empty_input_dir);
    
    BatchProcessor processor(empty_input_dir.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        return img.loadFromFile(input);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_EQ(stats.total_files, 0);
    EXPECT_EQ(stats.processed_files, 0);
    EXPECT_EQ(stats.failed_files, 0);
    EXPECT_EQ(stats.skipped_files, 0);
}

// Тест обработки с несуществующими файлами (симуляция)
TEST_F(BatchProcessorTest, ProcessAllWithNonExistentFiles)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        // Пытаемся загрузить несуществующий файл
        ImageProcessor img;
        return img.loadFromFile("/nonexistent/file.png");
    };
    
    auto stats = processor.processAll(process_func);
    
    // Все файлы должны завершиться ошибкой
    EXPECT_GT(stats.total_files, 0);
    EXPECT_EQ(stats.failed_files, stats.total_files);
    EXPECT_EQ(stats.processed_files, 0);
}

// Тест обработки с частичными ошибками
TEST_F(BatchProcessorTest, ProcessAllWithPartialErrors)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    size_t call_count = 0;
    auto process_func = [&call_count](const std::string& input, const std::string& output) -> FilterResult
    {
        call_count++;
        // Первый файл - успех, остальные - ошибка
        if (call_count == 1)
        {
            ImageProcessor img;
            const auto load_result = img.loadFromFile(input);
            if (!load_result.isSuccess())
            {
                return load_result;
            }
            return img.saveToFile(output);
        }
        return FilterResult::failure(FilterError::InvalidParameter, "Тестовая ошибка");
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GT(stats.total_files, 0);
    EXPECT_EQ(stats.processed_files, 1);
    EXPECT_EQ(stats.failed_files, stats.total_files - 1);
}

// Тест обработки с различными комбинациями фильтров
TEST_F(BatchProcessorTest, ProcessAllWithFilters)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        
        // Применяем несколько фильтров
        GrayscaleFilter grayscale;
        auto result = grayscale.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        BrightnessFilter brightness(1.2);
        result = brightness.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест обработки с различными комбинациями фильтров (расширенный набор)
TEST_F(BatchProcessorTest, ProcessAllWithMultipleFilters)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        
        // Применяем различные типы фильтров
        GrayscaleFilter grayscale;
        auto result = grayscale.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        ContrastFilter contrast(1.5);
        result = contrast.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        FlipHorizontalFilter flip_h;
        result = flip_h.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест обработки с обработкой исключений
TEST_F(BatchProcessorTest, ProcessAllWithException)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        // Бросаем исключение для тестирования обработки
        throw std::runtime_error("Тестовое исключение");
    };
    
    auto stats = processor.processAll(process_func);
    
    // Все файлы должны завершиться ошибкой из-за исключений
    EXPECT_GT(stats.total_files, 0);
    EXPECT_EQ(stats.failed_files, stats.total_files);
    EXPECT_EQ(stats.processed_files, 0);
}

// Тест processAllWithResume без файла состояния
TEST_F(BatchProcessorTest, ProcessAllWithResumeNoStateFile)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    // Вызываем без файла состояния (пустая строка)
    auto stats = processor.processAllWithResume(process_func, nullptr, "");
    
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест processAllWithResume с файлом состояния
TEST_F(BatchProcessorTest, ProcessAllWithResumeWithStateFile)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    const auto state_file = (test_dir_ / "resume_state.json").string();
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAllWithResume(process_func, nullptr, state_file);
    
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
    
    // Проверяем, что файл состояния создан
    EXPECT_TRUE(std::filesystem::exists(state_file));
}

// Тест обработки с поврежденными файлами (симуляция)
TEST_F(BatchProcessorTest, ProcessAllWithCorruptedFiles)
{
    // Создаем поврежденный файл
    auto corrupted_file = input_dir_ / "corrupted.png";
    std::ofstream file(corrupted_file, std::ios::binary);
    file.write("corrupted data", 14);
    file.close();
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        return img.loadFromFile(input);
    };
    
    auto stats = processor.processAll(process_func);
    
    // Должны быть как успешные, так и неуспешные файлы
    EXPECT_GT(stats.total_files, 0);
    // Поврежденный файл должен вызвать ошибку
    EXPECT_GT(stats.failed_files, 0);
}

// Тест обработки с рекурсивным поиском и фильтрами
TEST_F(BatchProcessorTest, ProcessAllRecursiveWithFilters)
{
    // Создаем поддиректорию с изображением
    auto subdir = input_dir_ / "subdir";
    std::filesystem::create_directories(subdir);
    
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    const auto filepath = subdir / "sub_test.png";
    TestUtils::saveTestImagePNG(test_image, width, height, filepath.string());
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string(), true); // Рекурсивно
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        
        InvertFilter invert;
        auto result = invert.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GE(stats.total_files, 4); // Должно найти изображения в поддиректории
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест обработки с различными форматами изображений
TEST_F(BatchProcessorTest, ProcessAllWithDifferentFormats)
{
    // Создаем изображения разных форматов
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    
    const auto jpg_path = input_dir_ / "test.jpg";
    TestUtils::saveTestImagePNG(test_image, width, height, jpg_path.string());
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GT(stats.total_files, 0);
    EXPECT_EQ(stats.processed_files, stats.total_files);
}

// Тест обработки с callback прогресса и проверкой всех полей
TEST_F(BatchProcessorTest, ProcessAllWithDetailedProgress)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    std::vector<ProgressInfo> progress_infos;
    auto progress_callback = [&progress_infos](const ProgressInfo& info)
    {
        progress_infos.push_back(info);
        EXPECT_GT(info.current, 0);
        EXPECT_LE(info.current, info.total);
        EXPECT_GE(info.percentage, 0.0);
        EXPECT_LE(info.percentage, 100.0);
        EXPECT_FALSE(info.current_file.empty());
    };
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func, progress_callback);
    
    EXPECT_EQ(progress_infos.size(), stats.total_files);
    EXPECT_GT(stats.total_files, 0);
    
    // Проверяем, что прогресс увеличивается
    for (size_t i = 1; i < progress_infos.size(); ++i)
    {
        EXPECT_GE(progress_infos[i].current, progress_infos[i-1].current);
    }
}

// Тест обработки с пустым callback прогресса
TEST_F(BatchProcessorTest, ProcessAllWithNullProgressCallback)
{
    BatchProcessor processor(input_dir_.string(), output_dir_.string());
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        return img.saveToFile(output);
    };
    
    // Передаем nullptr как callback
    auto stats = processor.processAll(process_func, nullptr);
    
    EXPECT_GE(stats.total_files, 3);
    EXPECT_EQ(stats.processed_files, stats.total_files);
    EXPECT_EQ(stats.failed_files, 0);
}

// Тест обработки с фильтрацией по шаблону и применением фильтров
TEST_F(BatchProcessorTest, ProcessAllWithPatternAndFilters)
{
    // Создаем файлы разных форматов
    constexpr int width = 10;
    constexpr int height = 10;
    auto test_image = TestUtils::createTestImage(width, height);
    
    const auto jpg_path = input_dir_ / "test.jpg";
    TestUtils::saveTestImagePNG(test_image, width, height, jpg_path.string());
    
    BatchProcessor processor(input_dir_.string(), output_dir_.string(), false, "*.png");
    
    auto process_func = [](const std::string& input, const std::string& output) -> FilterResult
    {
        ImageProcessor img;
        const auto load_result = img.loadFromFile(input);
        if (!load_result.isSuccess())
        {
            return load_result;
        }
        
        SepiaFilter sepia;
        auto result = sepia.apply(img);
        if (!result.isSuccess())
        {
            return result;
        }
        
        return img.saveToFile(output);
    };
    
    auto stats = processor.processAll(process_func);
    
    EXPECT_GT(stats.total_files, 0);
    // Должны обработать только PNG файлы
    for (const auto& img_path : processor.findImages())
    {
        EXPECT_EQ(img_path.extension(), ".png");
    }
}
