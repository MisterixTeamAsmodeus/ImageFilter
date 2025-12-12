#include <gtest/gtest.h>
#include <utils/Config.h>
#include <utils/FilterFactory.h>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

/**
 * @brief Тесты для класса Config
 */
class ConfigTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Создаем временную директорию для тестов
        test_dir_ = fs::temp_directory_path() / "ImageFilterConfigTests";
        fs::create_directories(test_dir_);
        
        // Регистрируем фильтры для валидации
        FilterFactory::getInstance().registerAll();
    }
    
    void TearDown() override
    {
        // Удаляем временную директорию
        if (fs::exists(test_dir_))
        {
            fs::remove_all(test_dir_);
        }
    }
    
    std::string getTestFilePath(const std::string& filename) const
    {
        return (test_dir_ / filename).string();
    }
    
    fs::path test_dir_;
};

/**
 * @brief Тест сохранения пресета
 */
TEST_F(ConfigTest, SavePreset)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "test_preset";
    chain.description = "Тестовый пресет";
    chain.filters.push_back({"grayscale", {}});
    chain.filters.push_back({"sepia", {}});
    
    const std::string filepath = getTestFilePath("test_preset.json");
    const bool result = config.savePreset(filepath, chain);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(filepath));
    
    // Проверяем содержимое файла
    std::ifstream file(filepath);
    ASSERT_TRUE(file.is_open());
    
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    
    EXPECT_NE(content.find("test_preset"), std::string::npos);
    EXPECT_NE(content.find("Тестовый пресет"), std::string::npos);
    EXPECT_NE(content.find("grayscale"), std::string::npos);
    EXPECT_NE(content.find("sepia"), std::string::npos);
}

/**
 * @brief Тест сохранения пресета с параметрами фильтров
 */
TEST_F(ConfigTest, SavePresetWithParams)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "preset_with_params";
    chain.description = "Пресет с параметрами";
    
    FilterConfig filter1;
    filter1.name = "brightness";
    filter1.params["brightness_factor"] = "1.5";
    chain.filters.push_back(filter1);
    
    FilterConfig filter2;
    filter2.name = "vignette";
    filter2.params["vignette_strength"] = "0.7";
    chain.filters.push_back(filter2);
    
    const std::string filepath = getTestFilePath("preset_with_params.json");
    const bool result = config.savePreset(filepath, chain);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(filepath));
}

/**
 * @brief Тест сохранения пресета с созданием директории
 */
TEST_F(ConfigTest, SavePresetCreatesDirectory)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "test";
    chain.description = "Тест";
    chain.filters.push_back({"grayscale", {}});
    
    const auto subdir = test_dir_ / "subdir";
    const std::string filepath = (subdir / "test.json").string();
    
    EXPECT_FALSE(fs::exists(subdir));
    
    const bool result = config.savePreset(filepath, chain);
    
    EXPECT_TRUE(result);
    EXPECT_TRUE(fs::exists(subdir));
    EXPECT_TRUE(fs::exists(filepath));
}

/**
 * @brief Тест загрузки пресета
 */
TEST_F(ConfigTest, LoadPreset)
{
    Config config;
    
    // Сначала сохраняем пресет
    FilterChainConfig original;
    original.name = "load_test";
    original.description = "Тест загрузки";
    original.filters.push_back({"grayscale", {}});
    original.filters.push_back({"invert", {}});
    
    const std::string filepath = getTestFilePath("load_test.json");
    ASSERT_TRUE(config.savePreset(filepath, original));
    
    // Загружаем пресет
    auto loaded = config.loadPreset(filepath);
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->name, "load_test");
    EXPECT_EQ(loaded->description, "Тест загрузки");
    EXPECT_EQ(loaded->filters.size(), 2);
    EXPECT_EQ(loaded->filters[0].name, "grayscale");
    EXPECT_EQ(loaded->filters[1].name, "invert");
}

/**
 * @brief Тест загрузки пресета с параметрами
 */
TEST_F(ConfigTest, LoadPresetWithParams)
{
    Config config;
    
    FilterChainConfig original;
    original.name = "params_test";
    original.description = "Тест параметров";
    
    FilterConfig filter;
    filter.name = "brightness";
    filter.params["brightness_factor"] = "2.0";
    original.filters.push_back(filter);
    
    const std::string filepath = getTestFilePath("params_test.json");
    ASSERT_TRUE(config.savePreset(filepath, original));
    
    auto loaded = config.loadPreset(filepath);
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->filters.size(), 1);
    EXPECT_EQ(loaded->filters[0].name, "brightness");
    EXPECT_EQ(loaded->filters[0].params.at("brightness_factor"), "2.0");
}

/**
 * @brief Тест загрузки несуществующего пресета
 */
TEST_F(ConfigTest, LoadNonExistentPreset)
{
    Config config;
    
    const std::string filepath = getTestFilePath("nonexistent.json");
    auto loaded = config.loadPreset(filepath);
    
    EXPECT_EQ(loaded, nullptr);
}

/**
 * @brief Тест загрузки поврежденного JSON файла
 */
TEST_F(ConfigTest, LoadInvalidJSON)
{
    Config config;
    
    const std::string filepath = getTestFilePath("invalid.json");
    
    // Создаем файл с невалидным JSON
    std::ofstream file(filepath);
    file << "{ invalid json }";
    file.close();
    
    auto loaded = config.loadPreset(filepath);
    
    EXPECT_EQ(loaded, nullptr);
}

/**
 * @brief Тест загрузки JSON файла с неполными данными
 */
TEST_F(ConfigTest, LoadIncompleteJSON)
{
    Config config;
    
    const std::string filepath = getTestFilePath("incomplete.json");
    
    // Создаем файл с неполным JSON
    std::ofstream file(filepath);
    file << "{}";
    file.close();
    
    auto loaded = config.loadPreset(filepath);
    
    // Должен загрузиться, но с пустыми полями
    ASSERT_NE(loaded, nullptr);
    EXPECT_TRUE(loaded->name.empty());
    EXPECT_TRUE(loaded->description.empty());
    EXPECT_TRUE(loaded->filters.empty());
}

/**
 * @brief Тест валидации корректной конфигурации
 */
TEST_F(ConfigTest, ValidateValidConfig)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "valid_preset";
    chain.description = "Валидный пресет";
    chain.filters.push_back({"grayscale", {}});
    chain.filters.push_back({"sepia", {}});
    chain.filters.push_back({"invert", {}});
    
    const bool result = config.validateConfig(chain);
    
    EXPECT_TRUE(result);
}

/**
 * @brief Тест валидации конфигурации с пустым именем
 */
TEST_F(ConfigTest, ValidateConfigWithEmptyName)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "";
    chain.description = "Пресет без имени";
    chain.filters.push_back({"grayscale", {}});
    
    const bool result = config.validateConfig(chain);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест валидации конфигурации с незарегистрированным фильтром
 */
TEST_F(ConfigTest, ValidateConfigWithUnregisteredFilter)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "invalid_preset";
    chain.description = "Пресет с несуществующим фильтром";
    chain.filters.push_back({"grayscale", {}});
    chain.filters.push_back({"nonexistent_filter", {}});
    
    const bool result = config.validateConfig(chain);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест валидации конфигурации с пустым именем фильтра
 */
TEST_F(ConfigTest, ValidateConfigWithEmptyFilterName)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "test";
    chain.description = "Пресет с пустым именем фильтра";
    chain.filters.push_back({"grayscale", {}});
    chain.filters.push_back({"", {}});
    
    const bool result = config.validateConfig(chain);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест валидации конфигурации с пустой цепочкой фильтров
 */
TEST_F(ConfigTest, ValidateConfigWithEmptyFilters)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "empty_chain";
    chain.description = "Пустая цепочка";
    // filters пустой
    
    const bool result = config.validateConfig(chain);
    
    // Пустая цепочка валидна, если имя не пустое
    EXPECT_TRUE(result);
}

/**
 * @brief Тест получения списка пресетов
 */
TEST_F(ConfigTest, ListPresets)
{
    Config config;
    
    // Создаем несколько пресетов
    FilterChainConfig chain1;
    chain1.name = "preset1";
    chain1.description = "Пресет 1";
    chain1.filters.push_back({"grayscale", {}});
    config.savePreset(getTestFilePath("preset1.json"), chain1);
    
    FilterChainConfig chain2;
    chain2.name = "preset2";
    chain2.description = "Пресет 2";
    chain2.filters.push_back({"sepia", {}});
    config.savePreset(getTestFilePath("preset2.json"), chain2);
    
    FilterChainConfig chain3;
    chain3.name = "preset3";
    chain3.description = "Пресет 3";
    chain3.filters.push_back({"invert", {}});
    config.savePreset(getTestFilePath("preset3.json"), chain3);
    
    // Создаем файл не-JSON, который должен быть проигнорирован
    std::ofstream non_json(getTestFilePath("not_a_preset.txt"));
    non_json << "not json";
    non_json.close();
    
    const auto presets = config.listPresets(test_dir_.string());
    
    EXPECT_EQ(presets.size(), 3);
    EXPECT_NE(std::find(presets.begin(), presets.end(), "preset1"), presets.end());
    EXPECT_NE(std::find(presets.begin(), presets.end(), "preset2"), presets.end());
    EXPECT_NE(std::find(presets.begin(), presets.end(), "preset3"), presets.end());
}

/**
 * @brief Тест получения списка пресетов из пустой директории
 */
TEST_F(ConfigTest, ListPresetsEmptyDirectory)
{
    Config config;
    
    const auto empty_dir = test_dir_ / "empty";
    fs::create_directories(empty_dir);
    
    const auto presets = config.listPresets(empty_dir.string());
    
    EXPECT_TRUE(presets.empty());
}

/**
 * @brief Тест получения списка пресетов из несуществующей директории
 */
TEST_F(ConfigTest, ListPresetsNonExistentDirectory)
{
    Config config;
    
    const auto nonexistent_dir = test_dir_ / "nonexistent";
    
    const auto presets = config.listPresets(nonexistent_dir.string());
    
    EXPECT_TRUE(presets.empty());
}

/**
 * @brief Тест удаления пресета
 */
TEST_F(ConfigTest, DeletePreset)
{
    Config config;
    
    // Создаем пресет
    FilterChainConfig chain;
    chain.name = "to_delete";
    chain.description = "Пресет для удаления";
    chain.filters.push_back({"grayscale", {}});
    
    const std::string filepath = getTestFilePath("to_delete.json");
    ASSERT_TRUE(config.savePreset(filepath, chain));
    ASSERT_TRUE(fs::exists(filepath));
    
    // Удаляем пресет
    const bool result = config.deletePreset(filepath);
    
    EXPECT_TRUE(result);
    EXPECT_FALSE(fs::exists(filepath));
}

/**
 * @brief Тест удаления несуществующего пресета
 */
TEST_F(ConfigTest, DeleteNonExistentPreset)
{
    Config config;
    
    const std::string filepath = getTestFilePath("nonexistent.json");
    
    const bool result = config.deletePreset(filepath);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест сохранения и загрузки с числовыми параметрами
 */
TEST_F(ConfigTest, SaveLoadNumericParams)
{
    Config config;
    
    FilterChainConfig original;
    original.name = "numeric_test";
    original.description = "Тест числовых параметров";
    
    FilterConfig filter;
    filter.name = "threshold";
    filter.params["threshold_value"] = "128";
    original.filters.push_back(filter);
    
    const std::string filepath = getTestFilePath("numeric_test.json");
    ASSERT_TRUE(config.savePreset(filepath, original));
    
    auto loaded = config.loadPreset(filepath);
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->filters[0].params.at("threshold_value"), "128");
}

/**
 * @brief Тест сохранения пресета с опасными символами в пути
 */
TEST_F(ConfigTest, SavePresetWithDangerousPath)
{
    Config config;
    
    FilterChainConfig chain;
    chain.name = "test";
    chain.description = "Тест";
    chain.filters.push_back({"grayscale", {}});
    
    // Попытка использовать опасные символы в пути
    const std::string dangerous_path = "../../etc/passwd";
    
    const bool result = config.savePreset(dangerous_path, chain);
    
    EXPECT_FALSE(result);
}

/**
 * @brief Тест загрузки пресета с опасными символами в пути
 */
TEST_F(ConfigTest, LoadPresetWithDangerousPath)
{
    Config config;
    
    const std::string dangerous_path = "../../etc/passwd";
    
    auto loaded = config.loadPreset(dangerous_path);
    
    EXPECT_EQ(loaded, nullptr);
}

/**
 * @brief Тест сохранения и загрузки пресета с несколькими параметрами
 */
TEST_F(ConfigTest, SaveLoadMultipleParams)
{
    Config config;
    
    FilterChainConfig original;
    original.name = "multi_params";
    original.description = "Множественные параметры";
    
    FilterConfig filter;
    filter.name = "motion_blur";
    filter.params["motion_blur_length"] = "15";
    filter.params["motion_blur_angle"] = "45.0";
    original.filters.push_back(filter);
    
    const std::string filepath = getTestFilePath("multi_params.json");
    ASSERT_TRUE(config.savePreset(filepath, original));
    
    auto loaded = config.loadPreset(filepath);
    
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->filters[0].params.size(), 2);
    EXPECT_EQ(loaded->filters[0].params.at("motion_blur_length"), "15");
    EXPECT_EQ(loaded->filters[0].params.at("motion_blur_angle"), "45.0");
}

