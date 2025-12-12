#include <gtest/gtest.h>
#include <utils/FileSystemHelper.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

/**
 * @brief Тесты для FileSystemHelper
 */
class FileSystemHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterFileSystemTests";
        input_dir_ = test_dir_ / "input";
        output_dir_ = test_dir_ / "output";
        
        fs::create_directories(input_dir_);
        fs::create_directories(output_dir_);
        
        // Создаем тестовые файлы
        createTestFiles();
    }
    
    void TearDown() override
    {
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    void createTestFiles()
    {
        // Создаем изображения
        createTestImage(input_dir_ / "test1.jpg");
        createTestImage(input_dir_ / "test2.png");
        createTestImage(input_dir_ / "test3.JPG");
        
        // Создаем не-изображения
        std::ofstream(input_dir_ / "test.txt") << "text file";
        std::ofstream(input_dir_ / "test.dat") << "data file";
        
        // Создаем поддиректорию с изображением
        auto subdir = input_dir_ / "subdir";
        fs::create_directories(subdir);
        createTestImage(subdir / "sub_test.png");
    }
    
    void createTestImage(const fs::path& path)
    {
        // Создаем минимальный валидный файл (заголовок)
        std::ofstream file(path, std::ios::binary);
        if (path.extension() == ".jpg" || path.extension() == ".JPG")
        {
            // Минимальный JPEG заголовок
            file.write("\xFF\xD8\xFF", 3);
        }
        else if (path.extension() == ".png")
        {
            // Минимальный PNG заголовок
            file.write("\x89PNG\r\n\x1a\n", 8);
        }
    }
    
    fs::path test_dir_;
    fs::path input_dir_;
    fs::path output_dir_;
};

// Тест isImageFile
TEST_F(FileSystemHelperTest, IsImageFile)
{
    EXPECT_TRUE(FileSystemHelper::isImageFile(input_dir_ / "test1.jpg"));
    EXPECT_TRUE(FileSystemHelper::isImageFile(input_dir_ / "test2.png"));
    EXPECT_TRUE(FileSystemHelper::isImageFile(input_dir_ / "test3.JPG"));
    EXPECT_FALSE(FileSystemHelper::isImageFile(input_dir_ / "test.txt"));
    EXPECT_FALSE(FileSystemHelper::isImageFile(input_dir_ / "test.dat"));
    
    // Тест без расширения
    EXPECT_FALSE(FileSystemHelper::isImageFile(input_dir_ / "test"));
}

// Тест matchesPattern
TEST_F(FileSystemHelperTest, MatchesPattern)
{
    EXPECT_TRUE(FileSystemHelper::matchesPattern("test.jpg", "*.jpg"));
    EXPECT_TRUE(FileSystemHelper::matchesPattern("test.png", "*.png"));
    EXPECT_TRUE(FileSystemHelper::matchesPattern("test.JPG", "*.jpg"));
    EXPECT_FALSE(FileSystemHelper::matchesPattern("test.jpg", "*.png"));
    EXPECT_FALSE(FileSystemHelper::matchesPattern("test.txt", "*.jpg"));
    
    // Тест пустого шаблона
    EXPECT_TRUE(FileSystemHelper::matchesPattern("test.jpg", ""));
    
    // Тест точного совпадения
    EXPECT_TRUE(FileSystemHelper::matchesPattern("test.jpg", "test.jpg"));
}

// Тест findImages (не рекурсивно)
TEST_F(FileSystemHelperTest, FindImagesNonRecursive)
{
    auto images = FileSystemHelper::findImages(input_dir_.string(), false, "");
    
    // Должно найти 3 изображения в корневой директории
    EXPECT_GE(images.size(), 3);
    
    // Проверяем, что все найденные файлы являются изображениями
    for (const auto& img : images)
    {
        EXPECT_TRUE(FileSystemHelper::isImageFile(img));
    }
}

// Тест findImages (рекурсивно)
TEST_F(FileSystemHelperTest, FindImagesRecursive)
{
    auto images = FileSystemHelper::findImages(input_dir_.string(), true, "");
    
    // Должно найти изображения в корневой директории и поддиректории
    EXPECT_GE(images.size(), 4);
    
    // Проверяем наличие файла из поддиректории
    bool found_sub = false;
    for (const auto& img : images)
    {
        if (img.filename() == "sub_test.png")
        {
            found_sub = true;
            break;
        }
    }
    EXPECT_TRUE(found_sub);
}

// Тест findImages с шаблоном
TEST_F(FileSystemHelperTest, FindImagesWithPattern)
{
    auto images = FileSystemHelper::findImages(input_dir_.string(), false, "*.jpg");
    
    // Должно найти только JPG файлы
    for (const auto& img : images)
    {
        const auto ext = img.extension().string();
        const std::string ext_lower = [&ext]() {
            std::string result = ext;
            std::transform(result.begin(), result.end(), result.begin(), ::tolower);
            return result;
        }();
        EXPECT_TRUE(ext_lower == ".jpg" || ext_lower == ".jpeg");
    }
}

// Тест findImages с несуществующей директорией
TEST_F(FileSystemHelperTest, FindImagesNonExistentDir)
{
    auto images = FileSystemHelper::findImages("/nonexistent/directory", false, "");
    EXPECT_TRUE(images.empty());
}

// Тест findImages с файлом вместо директории
TEST_F(FileSystemHelperTest, FindImagesWithFile)
{
    auto images = FileSystemHelper::findImages((input_dir_ / "test.txt").string(), false, "");
    EXPECT_TRUE(images.empty());
}

// Тест getRelativePath
TEST_F(FileSystemHelperTest, GetRelativePath)
{
    fs::path base = "/base";
    fs::path full = "/base/sub/file.png";
    
    auto relative = FileSystemHelper::getRelativePath(full, base);
    EXPECT_EQ(relative.string(), "sub/file.png");
    
    // Тест с реальными путями
    auto test_file = input_dir_ / "test1.jpg";
    auto relative_test = FileSystemHelper::getRelativePath(test_file, input_dir_);
    EXPECT_EQ(relative_test.string(), "test1.jpg");
}

// Тест getRelativePath с несовместимыми путями
TEST_F(FileSystemHelperTest, GetRelativePathIncompatible)
{
    fs::path base = "/base1";
    fs::path full = "/base2/file.png";
    
    // Может вернуть относительный путь с ".." или только имя файла
    // В зависимости от реализации std::filesystem::relative
    auto relative = FileSystemHelper::getRelativePath(full, base);
    // Проверяем, что результат содержит имя файла
    EXPECT_EQ(relative.filename().string(), "file.png");
}

// Тест ensureOutputDirectory
TEST_F(FileSystemHelperTest, EnsureOutputDirectory)
{
    auto output_file = output_dir_ / "subdir" / "output.png";
    
    EXPECT_TRUE(FileSystemHelper::ensureOutputDirectory(output_file));
    EXPECT_TRUE(fs::exists(output_file.parent_path()));
}

// Тест ensureOutputDirectory для существующей директории
TEST_F(FileSystemHelperTest, EnsureOutputDirectoryExists)
{
    fs::create_directories(output_dir_ / "existing");
    auto output_file = output_dir_ / "existing" / "output.png";
    
    EXPECT_TRUE(FileSystemHelper::ensureOutputDirectory(output_file));
    EXPECT_TRUE(fs::exists(output_file.parent_path()));
}

// Тест ensureOutputDirectory для файла без родительской директории
TEST_F(FileSystemHelperTest, EnsureOutputDirectoryNoParent)
{
    // Файл в корневой директории (без поддиректорий)
    auto output_file = output_dir_ / "output.png";
    
    EXPECT_TRUE(FileSystemHelper::ensureOutputDirectory(output_file));
}

