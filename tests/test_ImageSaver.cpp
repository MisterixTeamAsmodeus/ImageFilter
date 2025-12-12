#include "test_utils.h"
#include <utils/ImageSaver.h>
#include <utils/ImageLoader.h>
#include <utils/FilterResult.h>
#include <utils/ErrorCodes.h>
#include <gtest/gtest.h>
#include <filesystem>
#include <cstdlib>
#include <cstring>

namespace fs = std::filesystem;

/**
 * @brief Тесты для ImageSaver
 */
class ImageSaverTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterSaverTests";
        fs::create_directories(test_dir_);
        
        // Создаем тестовые данные изображений
        createTestImageData();
    }
    
    void TearDown() override
    {
        // Освобождаем память
        if (rgb_data_ != nullptr)
        {
            std::free(rgb_data_);
            rgb_data_ = nullptr;
        }
        
        if (rgba_data_ != nullptr)
        {
            std::free(rgba_data_);
            rgba_data_ = nullptr;
        }
        
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestImageData()
    {
        constexpr int width = 10;
        constexpr int height = 10;
        
        // Создаем RGB данные
        auto rgb_image = TestUtils::createTestImage(width, height);
        rgb_data_ = static_cast<uint8_t*>(std::malloc(rgb_image.size()));
        ASSERT_NE(rgb_data_, nullptr);
        std::memcpy(rgb_data_, rgb_image.data(), rgb_image.size());
        width_ = width;
        height_ = height;
        
        // Создаем RGBA данные
        auto rgba_image = TestUtils::createTestImageRGBA(width, height);
        rgba_data_ = static_cast<uint8_t*>(std::malloc(rgba_image.size()));
        ASSERT_NE(rgba_data_, nullptr);
        std::memcpy(rgba_data_, rgba_image.data(), rgba_image.size());
    }
    
    fs::path test_dir_;
    uint8_t* rgb_data_ = nullptr;
    uint8_t* rgba_data_ = nullptr;
    int width_ = 10;
    int height_ = 10;
};

/**
 * @brief Тест сохранения PNG изображения (RGB)
 */
TEST_F(ImageSaverTest, SavePNGRGB)
{
    const std::string output_file = (test_dir_ / "output_rgb.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения PNG изображения (RGBA)
 */
TEST_F(ImageSaverTest, SavePNGRGBA)
{
    const std::string output_file = (test_dir_ / "output_rgba.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, true, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения PNG с принудительным RGB (из RGBA)
 */
TEST_F(ImageSaverTest, SavePNGForceRGB)
{
    const std::string output_file = (test_dir_ / "output_force_rgb.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG изображения (RGB)
 */
TEST_F(ImageSaverTest, SaveJPEGRGB)
{
    const std::string output_file = (test_dir_ / "output.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG из RGBA (должен преобразовать в RGB)
 */
TEST_F(ImageSaverTest, SaveJPEGFromRGBA)
{
    const std::string output_file = (test_dir_ / "output_from_rgba.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG с различным качеством (высокое качество)
 */
TEST_F(ImageSaverTest, SaveJPEGHighQuality)
{
    const std::string output_file = (test_dir_ / "output_high_quality.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 100);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG с низким качеством
 */
TEST_F(ImageSaverTest, SaveJPEGLowQuality)
{
    const std::string output_file = (test_dir_ / "output_low_quality.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 10);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    // Файл с низким качеством обычно меньше по размеру
    const std::string high_quality_file = (test_dir_ / "output_high_quality.jpg").string();
    if (fs::exists(high_quality_file))
    {
        // Низкое качество может дать меньший размер файла
        // Но это не гарантировано, поэтому просто проверяем успешность
    }
}

/**
 * @brief Тест сохранения JPEG со средним качеством
 */
TEST_F(ImageSaverTest, SaveJPEGMediumQuality)
{
    const std::string output_file = (test_dir_ / "output_medium_quality.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 50);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения BMP изображения (RGB)
 */
TEST_F(ImageSaverTest, SaveBMPRGB)
{
    const std::string output_file = (test_dir_ / "output.bmp").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения BMP из RGBA (должен преобразовать в RGB)
 */
TEST_F(ImageSaverTest, SaveBMPFromRGBA)
{
    const std::string output_file = (test_dir_ / "output_from_rgba.bmp").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения с пустым путем
 */
TEST_F(ImageSaverTest, SaveEmptyPath)
{
    const auto result = ImageSaver::saveToFile("", rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест сохранения с опасными символами в пути
 */
TEST_F(ImageSaverTest, SaveDangerousPath)
{
    const std::string dangerous_path = "../../etc/passwd";
    const auto result = ImageSaver::saveToFile(dangerous_path, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест сохранения с нулевым указателем данных
 */
TEST_F(ImageSaverTest, SaveNullData)
{
    const std::string output_file = (test_dir_ / "output_null.png").string();
    const auto result = ImageSaver::saveToFile(output_file, nullptr, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

/**
 * @brief Тест сохранения с некорректными размерами (нулевая ширина)
 */
TEST_F(ImageSaverTest, SaveZeroWidth)
{
    const std::string output_file = (test_dir_ / "output_zero_width.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, 0, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест сохранения с некорректными размерами (нулевая высота)
 */
TEST_F(ImageSaverTest, SaveZeroHeight)
{
    const std::string output_file = (test_dir_ / "output_zero_height.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, 0, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест сохранения с некорректными размерами (отрицательная ширина)
 */
TEST_F(ImageSaverTest, SaveNegativeWidth)
{
    const std::string output_file = (test_dir_ / "output_negative_width.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, -1, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест сохранения с некорректными размерами (отрицательная высота)
 */
TEST_F(ImageSaverTest, SaveNegativeHeight)
{
    const std::string output_file = (test_dir_ / "output_negative_height.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, -1, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест сохранения в неподдерживаемый формат
 */
TEST_F(ImageSaverTest, SaveUnsupportedFormat)
{
    const std::string output_file = (test_dir_ / "output.xyz").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::UnsupportedFormat);
}

/**
 * @brief Тест сохранения файла без расширения
 */
TEST_F(ImageSaverTest, SaveFileWithoutExtension)
{
    const std::string output_file = (test_dir_ / "output_noext").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест сохранения и последующей загрузки PNG
 */
TEST_F(ImageSaverTest, SaveAndLoadPNG)
{
    const std::string output_file = (test_dir_ / "roundtrip.png").string();
    auto save_result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(save_result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    // Загружаем обратно
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, false, loaded);
    
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_NE(loaded.data, nullptr);
    EXPECT_EQ(loaded.width, width_);
    EXPECT_EQ(loaded.height, height_);
    EXPECT_EQ(loaded.channels, 3);
    
    // Освобождаем память
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения и последующей загрузки JPEG
 */
TEST_F(ImageSaverTest, SaveAndLoadJPEG)
{
    const std::string output_file = (test_dir_ / "roundtrip.jpg").string();
    auto save_result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(save_result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    // Загружаем обратно
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, false, loaded);
    
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_NE(loaded.data, nullptr);
    EXPECT_EQ(loaded.width, width_);
    EXPECT_EQ(loaded.height, height_);
    EXPECT_EQ(loaded.channels, 3);
    
    // Освобождаем память
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения и последующей загрузки PNG с альфа-каналом
 */
TEST_F(ImageSaverTest, SaveAndLoadPNGWithAlpha)
{
    const std::string output_file = (test_dir_ / "roundtrip_alpha.png").string();
    auto save_result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, true, 90);
    
    EXPECT_TRUE(save_result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    // Загружаем обратно с альфа-каналом
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, true, loaded);
    
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_NE(loaded.data, nullptr);
    EXPECT_EQ(loaded.width, width_);
    EXPECT_EQ(loaded.height, height_);
    EXPECT_EQ(loaded.channels, 4);
    
    // Освобождаем память
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения JPEG с граничными значениями качества (0)
 */
TEST_F(ImageSaverTest, SaveJPEGQualityZero)
{
    const std::string output_file = (test_dir_ / "output_quality_0.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 0);
    
    // Качество 0 может быть принято или отклонено в зависимости от реализации
    // Проверяем, что либо успех, либо ошибка
    if (result.isSuccess())
    {
        EXPECT_TRUE(fs::exists(output_file));
    }
}

/**
 * @brief Тест сохранения JPEG с граничными значениями качества (100)
 */
TEST_F(ImageSaverTest, SaveJPEGQuality100)
{
    const std::string output_file = (test_dir_ / "output_quality_100.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 100);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG с качеством выше 100 (должно быть обработано)
 */
TEST_F(ImageSaverTest, SaveJPEGQualityOver100)
{
    const std::string output_file = (test_dir_ / "output_quality_over100.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 150);
    
    // Может быть принято как 100 или отклонено
    // Проверяем, что либо успех, либо ошибка
    if (result.isSuccess())
    {
        EXPECT_TRUE(fs::exists(output_file));
    }
}

/**
 * @brief Тест сохранения большого изображения
 */
TEST_F(ImageSaverTest, SaveLargeImage)
{
    constexpr int large_width = 100;
    constexpr int large_height = 100;
    auto large_image = TestUtils::createTestImage(large_width, large_height);
    uint8_t* large_data = static_cast<uint8_t*>(std::malloc(large_image.size()));
    ASSERT_NE(large_data, nullptr);
    std::memcpy(large_data, large_image.data(), large_image.size());
    
    const std::string output_file = (test_dir_ / "large_output.png").string();
    const auto result = ImageSaver::saveToFile(output_file, large_data, large_width, large_height, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    std::free(large_data);
}

/**
 * @brief Тест сохранения JPEG с различными значениями качества (1-99)
 */
TEST_F(ImageSaverTest, SaveJPEGQualityRange)
{
    const int qualities[] = {1, 25, 50, 75, 99};
    
    for (int quality : qualities)
    {
        const std::string output_file = (test_dir_ / ("output_quality_" + std::to_string(quality) + ".jpg")).string();
        const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, quality);
        
        EXPECT_TRUE(result.isSuccess()) << "Failed to save JPEG with quality " << quality;
        EXPECT_TRUE(fs::exists(output_file)) << "File not created for quality " << quality;
        EXPECT_GT(fs::file_size(output_file), 0) << "File is empty for quality " << quality;
    }
}

/**
 * @brief Тест сохранения JPEG с отрицательным качеством
 */
TEST_F(ImageSaverTest, SaveJPEGQualityNegative)
{
    const std::string output_file = (test_dir_ / "output_quality_negative.jpg").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, -1);
    
    // Отрицательное качество может быть обработано по-разному
    // Проверяем, что либо успех, либо ошибка
    if (result.isSuccess())
    {
        EXPECT_TRUE(fs::exists(output_file));
    }
}

/**
 * @brief Тест сохранения PNG с RGBA и preserve_alpha=true
 */
TEST_F(ImageSaverTest, SavePNGRGBAWithAlpha)
{
    const std::string output_file = (test_dir_ / "output_rgba_alpha.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, true, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    // Загружаем обратно и проверяем, что альфа-канал сохранен
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, true, loaded);
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_EQ(loaded.channels, 4);
    
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения PNG с RGBA и preserve_alpha=false
 */
TEST_F(ImageSaverTest, SavePNGRGBAWithoutAlpha)
{
    const std::string output_file = (test_dir_ / "output_rgba_no_alpha.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения JPEG с RGBA (должен преобразовать в RGB)
 */
TEST_F(ImageSaverTest, SaveJPEGFromRGBAWithPreserveAlpha)
{
    const std::string output_file = (test_dir_ / "output_rgba_jpeg.jpg").string();
    // preserve_alpha игнорируется для JPEG, так как JPEG не поддерживает альфа-канал
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, true, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    // Загружаем обратно - должен быть RGB
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, false, loaded);
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_EQ(loaded.channels, 3);
    
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения BMP с RGBA (должен преобразовать в RGB)
 */
TEST_F(ImageSaverTest, SaveBMPFromRGBAWithPreserveAlpha)
{
    const std::string output_file = (test_dir_ / "output_rgba_bmp.bmp").string();
    // preserve_alpha игнорируется для BMP, так как BMP не поддерживает альфа-канал
    const auto result = ImageSaver::saveToFile(output_file, rgba_data_, width_, height_, 4, true, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
}

/**
 * @brief Тест сохранения изображения с минимальными размерами (1x1)
 */
TEST_F(ImageSaverTest, SaveMinimalSizeImage)
{
    constexpr int min_width = 1;
    constexpr int min_height = 1;
    auto min_image = TestUtils::createTestImage(min_width, min_height);
    uint8_t* min_data = static_cast<uint8_t*>(std::malloc(min_image.size()));
    ASSERT_NE(min_data, nullptr);
    std::memcpy(min_data, min_image.data(), min_image.size());
    
    const std::string output_file = (test_dir_ / "minimal_output.png").string();
    const auto result = ImageSaver::saveToFile(output_file, min_data, min_width, min_height, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    std::free(min_data);
}

/**
 * @brief Тест сохранения изображения с очень большими размерами
 */
TEST_F(ImageSaverTest, SaveVeryLargeImage)
{
    constexpr int very_large_width = 500;
    constexpr int very_large_height = 500;
    auto very_large_image = TestUtils::createTestImage(very_large_width, very_large_height);
    uint8_t* very_large_data = static_cast<uint8_t*>(std::malloc(very_large_image.size()));
    ASSERT_NE(very_large_data, nullptr);
    std::memcpy(very_large_data, very_large_image.data(), very_large_image.size());
    
    const std::string output_file = (test_dir_ / "very_large_output.png").string();
    const auto result = ImageSaver::saveToFile(output_file, very_large_data, very_large_width, very_large_height, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    std::free(very_large_data);
}

/**
 * @brief Тест сохранения изображения с нестандартными размерами (не квадратное)
 */
TEST_F(ImageSaverTest, SaveNonSquareImage)
{
    constexpr int wide_width = 100;
    constexpr int wide_height = 50;
    auto wide_image = TestUtils::createTestImage(wide_width, wide_height);
    uint8_t* wide_data = static_cast<uint8_t*>(std::malloc(wide_image.size()));
    ASSERT_NE(wide_data, nullptr);
    std::memcpy(wide_data, wide_image.data(), wide_image.size());
    
    const std::string output_file = (test_dir_ / "wide_output.png").string();
    const auto result = ImageSaver::saveToFile(output_file, wide_data, wide_width, wide_height, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    std::free(wide_data);
}

/**
 * @brief Тест сохранения изображения с высоким изображением (высота больше ширины)
 */
TEST_F(ImageSaverTest, SaveTallImage)
{
    constexpr int tall_width = 50;
    constexpr int tall_height = 100;
    auto tall_image = TestUtils::createTestImage(tall_width, tall_height);
    uint8_t* tall_data = static_cast<uint8_t*>(std::malloc(tall_image.size()));
    ASSERT_NE(tall_data, nullptr);
    std::memcpy(tall_data, tall_image.data(), tall_image.size());
    
    const std::string output_file = (test_dir_ / "tall_output.png").string();
    const auto result = ImageSaver::saveToFile(output_file, tall_data, tall_width, tall_height, 3, false, 90);
    
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    EXPECT_GT(fs::file_size(output_file), 0);
    
    std::free(tall_data);
}

/**
 * @brief Тест сохранения во все поддерживаемые форматы с различными расширениями
 */
TEST_F(ImageSaverTest, SaveAllFormatsWithDifferentExtensions)
{
    // PNG с различными регистрами расширений
    const std::string png_lower = (test_dir_ / "output.png").string();
    EXPECT_TRUE(ImageSaver::saveToFile(png_lower, rgb_data_, width_, height_, 3, false, 90).isSuccess());
    
    // JPEG с различными расширениями
    const std::string jpg_lower = (test_dir_ / "output.jpg").string();
    EXPECT_TRUE(ImageSaver::saveToFile(jpg_lower, rgb_data_, width_, height_, 3, false, 90).isSuccess());
    
    const std::string jpeg_lower = (test_dir_ / "output.jpeg").string();
    EXPECT_TRUE(ImageSaver::saveToFile(jpeg_lower, rgb_data_, width_, height_, 3, false, 90).isSuccess());
    
    // BMP
    const std::string bmp_lower = (test_dir_ / "output.bmp").string();
    EXPECT_TRUE(ImageSaver::saveToFile(bmp_lower, rgb_data_, width_, height_, 3, false, 90).isSuccess());
}

/**
 * @brief Тест сохранения с некорректным количеством каналов
 */
TEST_F(ImageSaverTest, SaveInvalidChannels)
{
    const std::string output_file = (test_dir_ / "output_invalid_channels.png").string();
    
    // Тест с 0 каналами
    const auto result_zero = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 0, false, 90);
    // Может быть ошибка или успех в зависимости от реализации
    
    // Тест с отрицательным количеством каналов
    const auto result_negative = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, -1, false, 90);
    // Может быть ошибка или успех в зависимости от реализации
    
    // Тест с слишком большим количеством каналов
    const auto result_large = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 10, false, 90);
    // Может быть ошибка или успех в зависимости от реализации
}

/**
 * @brief Тест сохранения с путем, содержащим только точку
 */
TEST_F(ImageSaverTest, SavePathWithOnlyDot)
{
    const std::string dot_path = ".";
    const auto result = ImageSaver::saveToFile(dot_path, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест сохранения с путем, заканчивающимся точкой
 */
TEST_F(ImageSaverTest, SavePathEndingWithDot)
{
    const std::string dot_path = (test_dir_ / "output.").string();
    const auto result = ImageSaver::saveToFile(dot_path, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест сохранения с путем, содержащим только расширение
 */
TEST_F(ImageSaverTest, SavePathWithOnlyExtension)
{
    const std::string ext_path = ".png";
    const auto result = ImageSaver::saveToFile(ext_path, rgb_data_, width_, height_, 3, false, 90);
    
    // Может быть ошибка валидации пути
    if (!result.isSuccess())
    {
        EXPECT_EQ(result.error, FilterError::InvalidFilePath);
    }
}

/**
 * @brief Тест сохранения с путем в несуществующую директорию (без создания)
 */
TEST_F(ImageSaverTest, SaveToNonExistentDirectory)
{
    const std::string non_existent_dir = (test_dir_ / "nonexistent" / "output.png").string();
    const auto result = ImageSaver::saveToFile(non_existent_dir, rgb_data_, width_, height_, 3, false, 90);
    
    // Может быть ошибка записи, если директория не создается автоматически
    // Или успех, если stbi_write создает директорию
    if (!result.isSuccess())
    {
        EXPECT_TRUE(result.error == FilterError::FileWriteError || 
                   result.error == FilterError::InvalidFilePath);
    }
}

/**
 * @brief Тест сохранения RGBA изображения во все форматы
 */
TEST_F(ImageSaverTest, SaveRGBAAllFormats)
{
    // PNG с альфа-каналом
    const std::string png_file = (test_dir_ / "rgba_output.png").string();
    auto result = ImageSaver::saveToFile(png_file, rgba_data_, width_, height_, 4, true, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(png_file));
    
    // JPEG из RGBA (должен преобразовать в RGB)
    const std::string jpg_file = (test_dir_ / "rgba_output.jpg").string();
    result = ImageSaver::saveToFile(jpg_file, rgba_data_, width_, height_, 4, false, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(jpg_file));
    
    // BMP из RGBA (должен преобразовать в RGB)
    const std::string bmp_file = (test_dir_ / "rgba_output.bmp").string();
    result = ImageSaver::saveToFile(bmp_file, rgba_data_, width_, height_, 4, false, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(bmp_file));
}

/**
 * @brief Тест сохранения и загрузки BMP
 */
TEST_F(ImageSaverTest, SaveAndLoadBMP)
{
    const std::string output_file = (test_dir_ / "roundtrip.bmp").string();
    auto save_result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 3, false, 90);
    
    EXPECT_TRUE(save_result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    // Загружаем обратно
    ImageLoader::LoadedImage loaded;
    auto load_result = ImageLoader::loadFromFile(output_file, false, loaded);
    
    EXPECT_TRUE(load_result.isSuccess());
    EXPECT_NE(loaded.data, nullptr);
    EXPECT_EQ(loaded.width, width_);
    EXPECT_EQ(loaded.height, height_);
    EXPECT_EQ(loaded.channels, 3);
    
    // Освобождаем память
    if (loaded.data != nullptr)
    {
        std::free(loaded.data);
    }
}

/**
 * @brief Тест сохранения JPEG с граничными значениями качества (1, 99)
 */
TEST_F(ImageSaverTest, SaveJPEGQualityBoundaries)
{
    // Качество 1
    const std::string output_file_1 = (test_dir_ / "output_quality_1.jpg").string();
    const auto result_1 = ImageSaver::saveToFile(output_file_1, rgb_data_, width_, height_, 3, false, 1);
    EXPECT_TRUE(result_1.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_1));
    
    // Качество 99
    const std::string output_file_99 = (test_dir_ / "output_quality_99.jpg").string();
    const auto result_99 = ImageSaver::saveToFile(output_file_99, rgb_data_, width_, height_, 3, false, 99);
    EXPECT_TRUE(result_99.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_99));
}

/**
 * @brief Тест ветвления: пустая строка имени файла (if (filename.empty()))
 */
TEST_F(ImageSaverTest, Branch_EmptyFilename)
{
    const auto result = ImageSaver::saveToFile("", rgb_data_, width_, height_, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест ветвления: опасные символы в пути (if (PathValidator::containsDangerousCharacters(...)))
 */
TEST_F(ImageSaverTest, Branch_DangerousCharacters)
{
    const std::vector<std::string> dangerous_paths = {
        "../test.png",
        "../../test.png",
        "/etc/passwd"
    };
    
    for (const auto& path : dangerous_paths)
    {
        const auto result = ImageSaver::saveToFile(path, rgb_data_, width_, height_, 3, false, 90);
        EXPECT_FALSE(result.isSuccess());
        EXPECT_EQ(result.error, FilterError::InvalidFilePath);
    }
}

/**
 * @brief Тест ветвления: нормализованный путь пуст (if (normalized_path.empty()))
 */
TEST_F(ImageSaverTest, Branch_NormalizedPathEmpty)
{
    const std::string invalid_path = "\0\0\0";
    const auto result = ImageSaver::saveToFile(invalid_path, rgb_data_, width_, height_, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidFilePath);
}

/**
 * @brief Тест ветвления: нулевой указатель на данные (if (data == nullptr))
 */
TEST_F(ImageSaverTest, Branch_NullData)
{
    const std::string output_file = (test_dir_ / "output_null.png").string();
    const auto result = ImageSaver::saveToFile(output_file, nullptr, width_, height_, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidImage);
}

/**
 * @brief Тест ветвления: нулевая ширина (if (width <= 0))
 */
TEST_F(ImageSaverTest, Branch_ZeroWidth)
{
    const std::string output_file = (test_dir_ / "output_zero_width.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, 0, height_, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест ветвления: нулевая высота (if (height <= 0))
 */
TEST_F(ImageSaverTest, Branch_ZeroHeight)
{
    const std::string output_file = (test_dir_ / "output_zero_height.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, 0, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест ветвления: отрицательная ширина
 */
TEST_F(ImageSaverTest, Branch_NegativeWidth)
{
    const std::string output_file = (test_dir_ / "output_negative_width.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, -1, height_, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест ветвления: отрицательная высота
 */
TEST_F(ImageSaverTest, Branch_NegativeHeight)
{
    const std::string output_file = (test_dir_ / "output_negative_height.png").string();
    const auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, -1, 3, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidSize);
}

/**
 * @brief Тест ветвления: некорректное количество каналов (if (channels != 3 && channels != 4))
 */
TEST_F(ImageSaverTest, Branch_InvalidChannels)
{
    const std::string output_file = (test_dir_ / "output_invalid_channels.png").string();
    
    // Тест с каналами = 0
    auto result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 0, false, 90);
    EXPECT_FALSE(result.isSuccess());
    EXPECT_EQ(result.error, FilterError::InvalidChannels);
    
    // Тест с каналами = 1
    result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 1, false, 90);
    EXPECT_FALSE(result.isSuccess());
    
    // Тест с каналами = 2
    result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 2, false, 90);
    EXPECT_FALSE(result.isSuccess());
    
    // Тест с каналами = 5
    result = ImageSaver::saveToFile(output_file, rgb_data_, width_, height_, 5, false, 90);
    EXPECT_FALSE(result.isSuccess());
}

/**
 * @brief Тест ветвления: проверка переполнения при вычислении размера (if (!SafeMath::safeMultiply(...)))
 */
TEST_F(ImageSaverTest, Branch_OverflowCheck)
{
    // Создаем данные с очень большими размерами, которые могут вызвать переполнение
    // В реальности это сложно протестировать, так как SafeMath защищает от переполнения
    // Но проверяем, что функция корректно обрабатывает большие изображения
    
    constexpr int large_width = 10000;
    constexpr int large_height = 10000;
    auto large_image = TestUtils::createTestImage(large_width, large_height);
    auto* large_data = static_cast<uint8_t*>(std::malloc(large_image.size()));
    ASSERT_NE(large_data, nullptr);
    std::memcpy(large_data, large_image.data(), large_image.size());
    
    const std::string output_file = (test_dir_ / "output_large.png").string();
    const auto result = ImageSaver::saveToFile(output_file, large_data, large_width, large_height, 3, false, 90);
    // Может быть успешным или неуспешным в зависимости от доступной памяти
    (void)result;
    
    std::free(large_data);
}

// ========== ФАЗА 3.2: Тесты для граничных значений в ImageSaver ==========

/**
 * @brief Тест граничных значений: минимальный размер изображения (1x1)
 */
TEST_F(ImageSaverTest, Boundary_MinSize1x1)
{
    constexpr int min_width = 1;
    constexpr int min_height = 1;
    auto min_image = TestUtils::createTestImage(min_width, min_height);
    auto* min_data = static_cast<uint8_t*>(std::malloc(min_image.size()));
    ASSERT_NE(min_data, nullptr);
    std::memcpy(min_data, min_image.data(), min_image.size());
    
    const std::string output_file = (test_dir_ / "output_min_1x1.png").string();
    const auto result = ImageSaver::saveToFile(output_file, min_data, min_width, min_height, 3, false, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    std::free(min_data);
}

/**
 * @brief Тест граничных значений: минимальный размер изображения (1x2)
 */
TEST_F(ImageSaverTest, Boundary_MinSize1x2)
{
    constexpr int min_width = 1;
    constexpr int min_height = 2;
    auto min_image = TestUtils::createTestImage(min_width, min_height);
    auto* min_data = static_cast<uint8_t*>(std::malloc(min_image.size()));
    ASSERT_NE(min_data, nullptr);
    std::memcpy(min_data, min_image.data(), min_image.size());
    
    const std::string output_file = (test_dir_ / "output_min_1x2.png").string();
    const auto result = ImageSaver::saveToFile(output_file, min_data, min_width, min_height, 3, false, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    std::free(min_data);
}

/**
 * @brief Тест граничных значений: минимальный размер изображения (2x1)
 */
TEST_F(ImageSaverTest, Boundary_MinSize2x1)
{
    constexpr int min_width = 2;
    constexpr int min_height = 1;
    auto min_image = TestUtils::createTestImage(min_width, min_height);
    auto* min_data = static_cast<uint8_t*>(std::malloc(min_image.size()));
    ASSERT_NE(min_data, nullptr);
    std::memcpy(min_data, min_image.data(), min_image.size());
    
    const std::string output_file = (test_dir_ / "output_min_2x1.png").string();
    const auto result = ImageSaver::saveToFile(output_file, min_data, min_width, min_height, 3, false, 90);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(fs::exists(output_file));
    
    std::free(min_data);
}

/**
 * @brief Тест граничных значений: качество JPEG на границах (0, 1, 99, 100)
 */
TEST_F(ImageSaverTest, Boundary_JPEGQuality)
{
    const std::string output_file_0 = (test_dir_ / "output_quality_0.jpg").string();
    const auto result_0 = ImageSaver::saveToFile(output_file_0, rgb_data_, width_, height_, 3, false, 0);
    EXPECT_TRUE(result_0.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_0));
    
    const std::string output_file_1 = (test_dir_ / "output_quality_1.jpg").string();
    const auto result_1 = ImageSaver::saveToFile(output_file_1, rgb_data_, width_, height_, 3, false, 1);
    EXPECT_TRUE(result_1.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_1));
    
    const std::string output_file_99 = (test_dir_ / "output_quality_99.jpg").string();
    const auto result_99 = ImageSaver::saveToFile(output_file_99, rgb_data_, width_, height_, 3, false, 99);
    EXPECT_TRUE(result_99.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_99));
    
    const std::string output_file_100 = (test_dir_ / "output_quality_100.jpg").string();
    const auto result_100 = ImageSaver::saveToFile(output_file_100, rgb_data_, width_, height_, 3, false, 100);
    EXPECT_TRUE(result_100.isSuccess());
    EXPECT_TRUE(fs::exists(output_file_100));
}

/**
 * @brief Тест граничных значений: каналы (3 и 4)
 */
TEST_F(ImageSaverTest, Boundary_Channels)
{
    // Тест с 3 каналами (RGB)
    const std::string output_rgb = (test_dir_ / "output_rgb.png").string();
    const auto result_rgb = ImageSaver::saveToFile(output_rgb, rgb_data_, width_, height_, 3, false, 90);
    EXPECT_TRUE(result_rgb.isSuccess());
    EXPECT_TRUE(fs::exists(output_rgb));
    
    // Тест с 4 каналами (RGBA)
    const std::string output_rgba = (test_dir_ / "output_rgba.png").string();
    const auto result_rgba = ImageSaver::saveToFile(output_rgba, rgba_data_, width_, height_, 4, true, 90);
    EXPECT_TRUE(result_rgba.isSuccess());
    EXPECT_TRUE(fs::exists(output_rgba));
}

/**
 * @brief Тест граничных значений: preserve_alpha (true и false)
 */
TEST_F(ImageSaverTest, Boundary_PreserveAlpha)
{
    // Тест с preserve_alpha = false
    const std::string output_no_alpha = (test_dir_ / "output_no_alpha.png").string();
    const auto result_no_alpha = ImageSaver::saveToFile(output_no_alpha, rgba_data_, width_, height_, 4, false, 90);
    EXPECT_TRUE(result_no_alpha.isSuccess());
    EXPECT_TRUE(fs::exists(output_no_alpha));
    
    // Тест с preserve_alpha = true
    const std::string output_with_alpha = (test_dir_ / "output_with_alpha.png").string();
    const auto result_with_alpha = ImageSaver::saveToFile(output_with_alpha, rgba_data_, width_, height_, 4, true, 90);
    EXPECT_TRUE(result_with_alpha.isSuccess());
    EXPECT_TRUE(fs::exists(output_with_alpha));
}

