#include <gtest/gtest.h>
#include <utils/ResumeStateManager.h>
#include <filesystem>
#include <fstream>
#include <set>

namespace fs = std::filesystem;

/**
 * @brief Тесты для ResumeStateManager
 */
class ResumeStateManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterResumeTests";
        state_file_ = test_dir_ / "resume_state.txt";
        
        fs::create_directories(test_dir_);
    }
    
    void TearDown() override
    {
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    fs::path test_dir_;
    fs::path state_file_;
};

// Тест loadResumeState с несуществующим файлом
TEST_F(ResumeStateManagerTest, LoadResumeStateNonExistent)
{
    auto processed = ResumeStateManager::loadResumeState(state_file_.string());
    EXPECT_TRUE(processed.empty());
}

// Тест loadResumeState с пустым файлом
TEST_F(ResumeStateManagerTest, LoadResumeStateEmpty)
{
    std::ofstream file(state_file_);
    file.close();
    
    auto processed = ResumeStateManager::loadResumeState(state_file_.string());
    EXPECT_TRUE(processed.empty());
}

// Тест loadResumeState с валидным файлом
TEST_F(ResumeStateManagerTest, LoadResumeStateValid)
{
    std::ofstream file(state_file_);
    file << "file1.jpg\n";
    file << "file2.png\n";
    file << "file3.jpg\n";
    file.close();
    
    auto processed = ResumeStateManager::loadResumeState(state_file_.string());
    
    EXPECT_EQ(processed.size(), 3);
    EXPECT_TRUE(processed.find("file1.jpg") != processed.end());
    EXPECT_TRUE(processed.find("file2.png") != processed.end());
    EXPECT_TRUE(processed.find("file3.jpg") != processed.end());
}

// Тест loadResumeState с комментариями и пустыми строками
TEST_F(ResumeStateManagerTest, LoadResumeStateWithComments)
{
    std::ofstream file(state_file_);
    file << "# Это комментарий\n";
    file << "file1.jpg\n";
    file << "\n";  // Пустая строка
    file << "file2.png\n";
    file << "# Еще комментарий\n";
    file << "file3.jpg\n";
    file.close();
    
    auto processed = ResumeStateManager::loadResumeState(state_file_.string());
    
    // Комментарии и пустые строки должны быть проигнорированы
    EXPECT_EQ(processed.size(), 3);
    EXPECT_TRUE(processed.find("file1.jpg") != processed.end());
    EXPECT_TRUE(processed.find("file2.png") != processed.end());
    EXPECT_TRUE(processed.find("file3.jpg") != processed.end());
}

// Тест loadResumeState с пробелами
TEST_F(ResumeStateManagerTest, LoadResumeStateWithWhitespace)
{
    std::ofstream file(state_file_);
    file << "  file1.jpg  \n";
    file << "file2.png\n";
    file << "  file3.jpg  \n";
    file.close();
    
    auto processed = ResumeStateManager::loadResumeState(state_file_.string());
    
    // Пробелы должны быть удалены
    EXPECT_EQ(processed.size(), 3);
    EXPECT_TRUE(processed.find("file1.jpg") != processed.end());
    EXPECT_TRUE(processed.find("file2.png") != processed.end());
    EXPECT_TRUE(processed.find("file3.jpg") != processed.end());
}

// Тест loadResumeState с пустым путем
TEST_F(ResumeStateManagerTest, LoadResumeStateEmptyPath)
{
    auto processed = ResumeStateManager::loadResumeState("");
    EXPECT_TRUE(processed.empty());
}

// Тест saveResumeState
TEST_F(ResumeStateManagerTest, SaveResumeState)
{
    std::set<std::string> processed_files;
    processed_files.insert("file1.jpg");
    processed_files.insert("file2.png");
    processed_files.insert("file3.jpg");
    
    EXPECT_TRUE(ResumeStateManager::saveResumeState(state_file_.string(), processed_files));
    EXPECT_TRUE(fs::exists(state_file_));
    
    // Проверяем содержимое файла
    std::ifstream file(state_file_);
    std::string line;
    std::set<std::string> loaded_files;
    
    while (std::getline(file, line))
    {
        if (!line.empty() && line[0] != '#')
        {
            // Убираем пробелы
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            if (!line.empty())
            {
                loaded_files.insert(line);
            }
        }
    }
    
    EXPECT_EQ(loaded_files.size(), 3);
    EXPECT_TRUE(loaded_files.find("file1.jpg") != loaded_files.end());
    EXPECT_TRUE(loaded_files.find("file2.png") != loaded_files.end());
    EXPECT_TRUE(loaded_files.find("file3.jpg") != loaded_files.end());
}

// Тест saveResumeState с пустым множеством
TEST_F(ResumeStateManagerTest, SaveResumeStateEmpty)
{
    std::set<std::string> processed_files;
    
    EXPECT_TRUE(ResumeStateManager::saveResumeState(state_file_.string(), processed_files));
    EXPECT_TRUE(fs::exists(state_file_));
}

// Тест saveResumeState с созданием директории
TEST_F(ResumeStateManagerTest, SaveResumeStateCreateDirectory)
{
    auto nested_file = test_dir_ / "subdir" / "resume_state.txt";
    
    std::set<std::string> processed_files;
    processed_files.insert("file1.jpg");
    
    EXPECT_TRUE(ResumeStateManager::saveResumeState(nested_file.string(), processed_files));
    EXPECT_TRUE(fs::exists(nested_file));
    EXPECT_TRUE(fs::exists(nested_file.parent_path()));
}

// Тест saveResumeState с пустым путем
TEST_F(ResumeStateManagerTest, SaveResumeStateEmptyPath)
{
    std::set<std::string> processed_files;
    processed_files.insert("file1.jpg");
    
    EXPECT_FALSE(ResumeStateManager::saveResumeState("", processed_files));
}

// Тест isFileProcessed с существующим файлом
TEST_F(ResumeStateManagerTest, IsFileProcessedExists)
{
    auto test_file = test_dir_ / "test.jpg";
    std::ofstream file(test_file);
    file << "test content";
    file.close();
    
    EXPECT_TRUE(ResumeStateManager::isFileProcessed(test_file));
}

// Тест isFileProcessed с несуществующим файлом
TEST_F(ResumeStateManagerTest, IsFileProcessedNotExists)
{
    auto test_file = test_dir_ / "nonexistent.jpg";
    
    EXPECT_FALSE(ResumeStateManager::isFileProcessed(test_file));
}

// Тест isFileProcessed с директорией
TEST_F(ResumeStateManagerTest, IsFileProcessedDirectory)
{
    auto test_dir = test_dir_ / "subdir";
    fs::create_directories(test_dir);
    
    // Директория не является файлом
    EXPECT_FALSE(ResumeStateManager::isFileProcessed(test_dir));
}

// Тест полного цикла: сохранение и загрузка
TEST_F(ResumeStateManagerTest, SaveAndLoadCycle)
{
    std::set<std::string> original_files;
    original_files.insert("file1.jpg");
    original_files.insert("file2.png");
    original_files.insert("file3.jpg");
    
    // Сохраняем
    EXPECT_TRUE(ResumeStateManager::saveResumeState(state_file_.string(), original_files));
    
    // Загружаем
    auto loaded_files = ResumeStateManager::loadResumeState(state_file_.string());
    
    // Проверяем, что все файлы совпадают
    EXPECT_EQ(loaded_files.size(), original_files.size());
    for (const auto& file : original_files)
    {
        EXPECT_TRUE(loaded_files.find(file) != loaded_files.end());
    }
}

// Тест с большим количеством файлов
TEST_F(ResumeStateManagerTest, LargeFileList)
{
    std::set<std::string> processed_files;
    for (int i = 0; i < 1000; ++i)
    {
        processed_files.insert("file" + std::to_string(i) + ".jpg");
    }
    
    EXPECT_TRUE(ResumeStateManager::saveResumeState(state_file_.string(), processed_files));
    
    auto loaded_files = ResumeStateManager::loadResumeState(state_file_.string());
    
    EXPECT_EQ(loaded_files.size(), 1000);
    for (int i = 0; i < 1000; ++i)
    {
        const std::string filename = "file" + std::to_string(i) + ".jpg";
        EXPECT_TRUE(loaded_files.find(filename) != loaded_files.end());
    }
}

