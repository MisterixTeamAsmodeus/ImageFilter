#include <gtest/gtest.h>
#include <cli/PresetManager.h>
#include <utils/Config.h>
#include <utils/FilterFactory.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Тесты для PresetManager
 */
class PresetManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        test_dir_ = fs::temp_directory_path() / "ImageFilterPresetTests";
        preset_dir_ = test_dir_ / "presets";
        
        fs::create_directories(preset_dir_);
        
        // Регистрируем тестовые фильтры
        auto& factory = FilterFactory::getInstance();
        factory.registerAll();
    }
    
    void TearDown() override
    {
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    fs::path test_dir_;
    fs::path preset_dir_;
};

// Тест loadPreset с абсолютным путем
TEST_F(PresetManagerTest, LoadPresetAbsolutePath)
{
    // Создаем тестовый пресет
    Config config;
    FilterChainConfig preset_config;
    preset_config.name = "test_preset";
    preset_config.description = "Тестовый пресет";
    
    FilterConfig filter1;
    filter1.name = "grayscale";
    preset_config.filters.push_back(filter1);
    
    FilterConfig filter2;
    filter2.name = "sepia";
    preset_config.filters.push_back(filter2);
    
    auto preset_file = preset_dir_ / "test_preset.json";
    config.savePreset(preset_file.string(), preset_config);
    
    // Загружаем пресет
    std::string filter_name;
    EXPECT_TRUE(PresetManager::loadPreset(preset_file.string(), preset_dir_.string(), filter_name));
    EXPECT_EQ(filter_name, "grayscale,sepia");
}

// Тест loadPreset с относительным путем
TEST_F(PresetManagerTest, LoadPresetRelativePath)
{
    // Создаем тестовый пресет
    Config config;
    FilterChainConfig preset_config;
    preset_config.name = "test_preset";
    preset_config.description = "Тестовый пресет";
    
    FilterConfig filter1;
    filter1.name = "grayscale";
    preset_config.filters.push_back(filter1);
    
    auto preset_file = preset_dir_ / "test_preset.json";
    config.savePreset(preset_file.string(), preset_config);
    
    // Загружаем пресет с относительным путем
    std::string filter_name;
    EXPECT_TRUE(PresetManager::loadPreset("test_preset", preset_dir_.string(), filter_name));
    EXPECT_EQ(filter_name, "grayscale");
}

// Тест loadPreset с относительным путем без расширения
TEST_F(PresetManagerTest, LoadPresetRelativePathNoExtension)
{
    // Создаем тестовый пресет
    Config config;
    FilterChainConfig preset_config;
    preset_config.name = "test_preset";
    preset_config.description = "Тестовый пресет";
    
    FilterConfig filter1;
    filter1.name = "grayscale";
    preset_config.filters.push_back(filter1);
    
    auto preset_file = preset_dir_ / "test_preset.json";
    config.savePreset(preset_file.string(), preset_config);
    
    // Загружаем пресет с относительным путем без .json
    std::string filter_name;
    EXPECT_TRUE(PresetManager::loadPreset("test_preset", preset_dir_.string(), filter_name));
    EXPECT_EQ(filter_name, "grayscale");
}

// Тест loadPreset с несуществующим файлом
TEST_F(PresetManagerTest, LoadPresetNonExistent)
{
    std::string filter_name;
    EXPECT_FALSE(PresetManager::loadPreset("nonexistent.json", preset_dir_.string(), filter_name));
}

// Тест loadPreset с невалидным пресетом
TEST_F(PresetManagerTest, LoadPresetInvalid)
{
    // Создаем невалидный JSON файл
    auto preset_file = preset_dir_ / "invalid.json";
    std::ofstream file(preset_file);
    file << "invalid json content";
    file.close();
    
    std::string filter_name;
    EXPECT_FALSE(PresetManager::loadPreset(preset_file.string(), preset_dir_.string(), filter_name));
}

// Тест savePreset
TEST_F(PresetManagerTest, SavePreset)
{
    std::string filter_chain = "grayscale,sepia";
    std::string preset_name = "test_preset";
    
    EXPECT_TRUE(PresetManager::savePreset(filter_chain, preset_name, preset_dir_.string()));
    
    // Проверяем, что файл создан
    auto preset_file = preset_dir_ / (preset_name + ".json");
    EXPECT_TRUE(fs::exists(preset_file));
    
    // Проверяем содержимое
    Config config;
    auto loaded = config.loadPreset(preset_file.string());
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->name, preset_name);
    EXPECT_EQ(loaded->filters.size(), 2);
    EXPECT_EQ(loaded->filters[0].name, "grayscale");
    EXPECT_EQ(loaded->filters[1].name, "sepia");
}

// Тест savePreset с пустой цепочкой фильтров
TEST_F(PresetManagerTest, SavePresetEmptyChain)
{
    std::string filter_chain = "";
    std::string preset_name = "empty_preset";
    
    EXPECT_FALSE(PresetManager::savePreset(filter_chain, preset_name, preset_dir_.string()));
}

// Тест savePreset с одним фильтром
TEST_F(PresetManagerTest, SavePresetSingleFilter)
{
    std::string filter_chain = "grayscale";
    std::string preset_name = "single_filter";
    
    EXPECT_TRUE(PresetManager::savePreset(filter_chain, preset_name, preset_dir_.string()));
    
    auto preset_file = preset_dir_ / (preset_name + ".json");
    EXPECT_TRUE(fs::exists(preset_file));
    
    Config config;
    auto loaded = config.loadPreset(preset_file.string());
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->filters.size(), 1);
    EXPECT_EQ(loaded->filters[0].name, "grayscale");
}

// Тест savePreset с пробелами в цепочке
TEST_F(PresetManagerTest, SavePresetWithSpaces)
{
    std::string filter_chain = "grayscale , sepia , invert";
    std::string preset_name = "spaced_preset";
    
    EXPECT_TRUE(PresetManager::savePreset(filter_chain, preset_name, preset_dir_.string()));
    
    auto preset_file = preset_dir_ / (preset_name + ".json");
    EXPECT_TRUE(fs::exists(preset_file));
    
    Config config;
    auto loaded = config.loadPreset(preset_file.string());
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->filters.size(), 3);
}

// Тест полного цикла: сохранение и загрузка
TEST_F(PresetManagerTest, SaveAndLoadCycle)
{
    std::string original_chain = "grayscale,sepia,invert";
    std::string preset_name = "cycle_test";
    
    // Сохраняем
    EXPECT_TRUE(PresetManager::savePreset(original_chain, preset_name, preset_dir_.string()));
    
    // Загружаем
    std::string loaded_chain;
    EXPECT_TRUE(PresetManager::loadPreset(preset_name, preset_dir_.string(), loaded_chain));
    
    // Проверяем, что цепочки совпадают
    EXPECT_EQ(loaded_chain, original_chain);
}

// Тест loadPreset с несколькими фильтрами
TEST_F(PresetManagerTest, LoadPresetMultipleFilters)
{
    Config config;
    FilterChainConfig preset_config;
    preset_config.name = "multi_filter";
    preset_config.description = "Пресет с несколькими фильтрами";
    
    FilterConfig filter1;
    filter1.name = "grayscale";
    preset_config.filters.push_back(filter1);
    
    FilterConfig filter2;
    filter2.name = "sepia";
    preset_config.filters.push_back(filter2);
    
    FilterConfig filter3;
    filter3.name = "invert";
    preset_config.filters.push_back(filter3);
    
    auto preset_file = preset_dir_ / "multi_filter.json";
    config.savePreset(preset_file.string(), preset_config);
    
    std::string filter_name;
    EXPECT_TRUE(PresetManager::loadPreset(preset_file.string(), preset_dir_.string(), filter_name));
    EXPECT_EQ(filter_name, "grayscale,sepia,invert");
}

