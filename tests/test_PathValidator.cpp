#include <gtest/gtest.h>
#include <utils/PathValidator.h>
#include <filesystem>
#include <fstream>
#include <cstring>

namespace fs = std::filesystem;

/**
 * @brief Тесты для PathValidator
 */
class PathValidatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестов
        test_dir_ = fs::temp_directory_path() / "PathValidatorTests";
        fs::create_directories(test_dir_);

        // Создаем тестовый файл
        test_file_ = test_dir_ / "test_file.txt";
        std::ofstream file(test_file_);
        file << "test content";
        file.close();
    }

    void TearDown() override
    {
        // Очищаем временные файлы
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }

    fs::path test_dir_;
    fs::path test_file_;
};

// Тест проверки безопасности пути - успешный случай
TEST_F(PathValidatorTest, IsPathSafe_Success)
{
    const std::string safe_path = test_file_.string();
    EXPECT_TRUE(PathValidator::isPathSafe(safe_path));
}

// Тест проверки безопасности пути - пустой путь
TEST_F(PathValidatorTest, IsPathSafe_EmptyPath)
{
    EXPECT_FALSE(PathValidator::isPathSafe(""));
}

// Тест проверки безопасности пути - path traversal (..)
TEST_F(PathValidatorTest, IsPathSafe_PathTraversal)
{
    const std::string unsafe_path = test_dir_.string() + "/../etc/passwd";
    EXPECT_FALSE(PathValidator::isPathSafe(unsafe_path, test_dir_.string()));
}

// Тест проверки безопасности пути - с базовой директорией
TEST_F(PathValidatorTest, IsPathSafe_WithBaseDir)
{
    const std::string safe_path = test_file_.string();
    EXPECT_TRUE(PathValidator::isPathSafe(safe_path, test_dir_.string()));
}

// Тест проверки безопасности пути - путь вне базовой директории
TEST_F(PathValidatorTest, IsPathSafe_OutsideBaseDir)
{
    const auto outside_path = fs::temp_directory_path() / "outside_file.txt";
    EXPECT_FALSE(PathValidator::isPathSafe(outside_path.string(), test_dir_.string()));
}

// Тест нормализации и валидации пути - успешный случай
TEST_F(PathValidatorTest, NormalizeAndValidate_Success)
{
    const std::string path = test_file_.string();
    const auto normalized = PathValidator::normalizeAndValidate(path);
    EXPECT_FALSE(normalized.empty());
}

// Тест нормализации и валидации пути - пустой путь
TEST_F(PathValidatorTest, NormalizeAndValidate_EmptyPath)
{
    const auto normalized = PathValidator::normalizeAndValidate("");
    EXPECT_TRUE(normalized.empty());
}

// Тест нормализации и валидации пути - небезопасный путь
TEST_F(PathValidatorTest, NormalizeAndValidate_UnsafePath)
{
    const std::string unsafe_path = test_dir_.string() + "/../etc/passwd";
    const auto normalized = PathValidator::normalizeAndValidate(unsafe_path, test_dir_.string());
    EXPECT_TRUE(normalized.empty());
}

// Тест проверки размера файла - успешный случай
TEST_F(PathValidatorTest, ValidateFileSize_Success)
{
    const uint64_t max_size = 1024 * 1024; // 1 MB
    EXPECT_TRUE(PathValidator::validateFileSize(test_file_.string(), max_size));
}

// Тест проверки размера файла - файл слишком большой
TEST_F(PathValidatorTest, ValidateFileSize_TooLarge)
{
    // Создаем большой файл
    const auto large_file = test_dir_ / "large_file.txt";
    std::ofstream file(large_file);
    const std::string content(1024 * 1024 + 1, 'x'); // 1 MB + 1 байт
    file << content;
    file.close();

    const uint64_t max_size = 1024 * 1024; // 1 MB
    EXPECT_FALSE(PathValidator::validateFileSize(large_file.string(), max_size));
}

// Тест проверки размера файла - несуществующий файл
TEST_F(PathValidatorTest, ValidateFileSize_NonExistent)
{
    const uint64_t max_size = 1024 * 1024;
    EXPECT_FALSE(PathValidator::validateFileSize("nonexistent_file.txt", max_size));
}

// Тест проверки размера файла - директория вместо файла
TEST_F(PathValidatorTest, ValidateFileSize_Directory)
{
    const uint64_t max_size = 1024 * 1024;
    EXPECT_FALSE(PathValidator::validateFileSize(test_dir_.string(), max_size));
}

// Тест проверки опасных символов - успешный случай
TEST_F(PathValidatorTest, ContainsDangerousCharacters_Success)
{
    const std::string safe_path = test_file_.string();
    EXPECT_FALSE(PathValidator::containsDangerousCharacters(safe_path));
}

// Тест проверки опасных символов - path traversal
TEST_F(PathValidatorTest, ContainsDangerousCharacters_PathTraversal)
{
    const std::string unsafe_path = "test/../file.txt";
    EXPECT_TRUE(PathValidator::containsDangerousCharacters(unsafe_path));
}

// Тест проверки опасных символов - null-байт
TEST_F(PathValidatorTest, ContainsDangerousCharacters_NullByte)
{
    // Создаем строку с null-байтом вручную
    std::string unsafe_path = "test";
    unsafe_path += '\0';
    unsafe_path += "file.txt";
    EXPECT_TRUE(PathValidator::containsDangerousCharacters(unsafe_path));
}

// Тест получения размера файла - успешный случай
TEST_F(PathValidatorTest, GetFileSize_Success)
{
    const auto size = PathValidator::getFileSize(test_file_.string());
    EXPECT_GT(size, 0);
}

// Тест получения размера файла - несуществующий файл
TEST_F(PathValidatorTest, GetFileSize_NonExistent)
{
    const auto size = PathValidator::getFileSize("nonexistent_file.txt");
    EXPECT_EQ(size, 0);
}

// Тест получения размера файла - директория вместо файла
TEST_F(PathValidatorTest, GetFileSize_Directory)
{
    const auto size = PathValidator::getFileSize(test_dir_.string());
    EXPECT_EQ(size, 0);
}

// Тест проверки безопасности пути - относительный путь
TEST_F(PathValidatorTest, IsPathSafe_RelativePath)
{
    const std::string relative_path = "test_file.txt";
    // Относительный путь без базовой директории должен быть безопасным
    EXPECT_TRUE(PathValidator::isPathSafe(relative_path));
}

// Тест проверки безопасности пути - абсолютный путь
TEST_F(PathValidatorTest, IsPathSafe_AbsolutePath)
{
    const std::string absolute_path = test_file_.string();
    EXPECT_TRUE(PathValidator::isPathSafe(absolute_path));
}

// Тест нормализации и валидации пути - с базовой директорией
TEST_F(PathValidatorTest, NormalizeAndValidate_WithBaseDir)
{
    const std::string path = test_file_.string();
    const auto normalized = PathValidator::normalizeAndValidate(path, test_dir_.string());
    EXPECT_FALSE(normalized.empty());
}

// Тест проверки размера файла - граничное значение
TEST_F(PathValidatorTest, ValidateFileSize_Boundary)
{
    // Создаем файл точно заданного размера
    const uint64_t exact_size = 1024;
    const auto exact_file = test_dir_ / "exact_size.txt";
    std::ofstream file(exact_file);
    const std::string content(exact_size, 'x');
    file << content;
    file.close();

    EXPECT_TRUE(PathValidator::validateFileSize(exact_file.string(), exact_size));
    EXPECT_FALSE(PathValidator::validateFileSize(exact_file.string(), exact_size - 1));
}

// Тест констант по умолчанию
TEST_F(PathValidatorTest, DefaultConstants)
{
    EXPECT_GT(PathValidator::DEFAULT_MAX_IMAGE_SIZE, 0);
    EXPECT_GT(PathValidator::DEFAULT_MAX_CONFIG_SIZE, 0);
    EXPECT_LT(PathValidator::DEFAULT_MAX_CONFIG_SIZE, PathValidator::DEFAULT_MAX_IMAGE_SIZE);
}

// Тест проверки опасных символов - управляющие символы
TEST_F(PathValidatorTest, ContainsDangerousCharacters_ControlChars)
{
    const std::string unsafe_path = "test\x01file.txt";
    EXPECT_TRUE(PathValidator::containsDangerousCharacters(unsafe_path));
}

// Тест проверки безопасности пути - сложный path traversal
TEST_F(PathValidatorTest, IsPathSafe_ComplexPathTraversal)
{
    const std::string unsafe_path = test_dir_.string() + "/../../etc/passwd";
    EXPECT_FALSE(PathValidator::isPathSafe(unsafe_path, test_dir_.string()));
}

