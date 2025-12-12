#include <gtest/gtest.h>
#include <utils/FilterFactory.h>
#include <utils/BufferPool.h>
#include <filters/IFilter.h>
#include <filters/GrayscaleFilter.h>
#include <filters/BrightnessFilter.h>
#include <ImageProcessor.h>
#include <CLI/CLI.hpp>
#include <memory>
#include <string>
#include <vector>
#include <cstring>
#include <algorithm>

/**
 * @brief Мок-фильтр для тестирования FilterFactory
 */
class MockFilter : public IFilter
{
public:
    explicit MockFilter(const std::string& name) : name_(name) {}
    
    FilterResult apply(ImageProcessor&) override
    {
        return FilterResult::success();
    }
    
    [[nodiscard]] std::string getName() const override
    {
        return name_;
    }
    
    [[nodiscard]] std::string getDescription() const override
    {
        return "Mock filter for testing";
    }
    
    [[nodiscard]] std::string getCategory() const override
    {
        return "Test";
    }

private:
    std::string name_;
};

/**
 * @brief Тесты для класса FilterFactory
 */
class FilterFactoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        factory_ = &FilterFactory::getInstance();
        app_ = std::make_unique<CLI::App>("test");
    }
    
    void TearDown() override
    {
        // Очищаем состояние между тестами не требуется,
        // так как FilterFactory - Singleton
    }
    
    FilterFactory* factory_;
    std::unique_ptr<CLI::App> app_;
};

/**
 * @brief Тест Singleton паттерна
 * 
 * Проверяет, что getInstance() всегда возвращает один и тот же экземпляр
 */
TEST_F(FilterFactoryTest, SingletonPattern)
{
    FilterFactory& instance1 = FilterFactory::getInstance();
    FilterFactory& instance2 = FilterFactory::getInstance();
    
    // Должны быть одним и тем же объектом
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @brief Тест регистрации фильтра
 * 
 * Проверяет, что фильтр успешно регистрируется и может быть найден
 */
TEST_F(FilterFactoryTest, RegisterFilter)
{
    const std::string filter_name = "test_filter";
    
    // Регистрируем тестовый фильтр
    factory_->registerFilter(filter_name, [](const CLI::App&) {
        return std::make_unique<MockFilter>("test_filter");
    });
    
    // Проверяем, что фильтр зарегистрирован
    EXPECT_TRUE(factory_->isRegistered(filter_name));
}

/**
 * @brief Тест создания зарегистрированного фильтра
 * 
 * Проверяет, что зарегистрированный фильтр может быть создан
 */
TEST_F(FilterFactoryTest, CreateRegisteredFilter)
{
    const std::string filter_name = "test_filter_create";
    
    // Регистрируем фильтр
    factory_->registerFilter(filter_name, [](const CLI::App&) {
        return std::make_unique<MockFilter>("test_filter_create");
    });
    
    // Создаем фильтр
    auto filter = factory_->create(filter_name, *app_);
    
    // Проверяем, что фильтр создан
    ASSERT_NE(filter, nullptr);
    EXPECT_EQ(filter->getName(), filter_name);
}

/**
 * @brief Тест создания несуществующего фильтра
 * 
 * Проверяет, что попытка создать несуществующий фильтр возвращает nullptr
 */
TEST_F(FilterFactoryTest, CreateNonExistentFilter)
{
    const std::string filter_name = "non_existent_filter";
    
    // Пытаемся создать несуществующий фильтр
    auto filter = factory_->create(filter_name, *app_);
    
    // Должен вернуться nullptr
    EXPECT_EQ(filter, nullptr);
}

/**
 * @brief Тест регистрации всех фильтров
 * 
 * Проверяет, что registerAll() регистрирует все доступные фильтры
 */
TEST_F(FilterFactoryTest, RegisterAll)
{
    // Регистрируем все фильтры
    factory_->registerAll();
    
    // Проверяем, что некоторые известные фильтры зарегистрированы
    EXPECT_TRUE(factory_->isRegistered("grayscale"));
    EXPECT_TRUE(factory_->isRegistered("sepia"));
    EXPECT_TRUE(factory_->isRegistered("invert"));
    EXPECT_TRUE(factory_->isRegistered("brightness"));
    EXPECT_TRUE(factory_->isRegistered("contrast"));
    EXPECT_TRUE(factory_->isRegistered("blur"));
    EXPECT_TRUE(factory_->isRegistered("sharpen"));
}

/**
 * @brief Тест получения списка зарегистрированных фильтров
 * 
 * Проверяет, что getRegisteredFilters() возвращает все зарегистрированные фильтры
 */
TEST_F(FilterFactoryTest, GetRegisteredFilters)
{
    // Регистрируем несколько фильтров
    factory_->registerFilter("filter1", [](const CLI::App&) {
        return std::make_unique<MockFilter>("filter1");
    });
    factory_->registerFilter("filter2", [](const CLI::App&) {
        return std::make_unique<MockFilter>("filter2");
    });
    factory_->registerFilter("filter3", [](const CLI::App&) {
        return std::make_unique<MockFilter>("filter3");
    });
    
    // Получаем список фильтров
    auto filters = factory_->getRegisteredFilters();
    
    // Проверяем, что все три фильтра в списке
    EXPECT_GE(filters.size(), 3);
    
    // Проверяем, что список отсортирован
    bool is_sorted = true;
    for (size_t i = 1; i < filters.size(); ++i)
    {
        if (filters[i] < filters[i - 1])
        {
            is_sorted = false;
            break;
        }
    }
    EXPECT_TRUE(is_sorted);
    
    // Проверяем наличие наших фильтров
    EXPECT_NE(std::find(filters.begin(), filters.end(), "filter1"), filters.end());
    EXPECT_NE(std::find(filters.begin(), filters.end(), "filter2"), filters.end());
    EXPECT_NE(std::find(filters.begin(), filters.end(), "filter3"), filters.end());
}

/**
 * @brief Тест проверки регистрации фильтра
 * 
 * Проверяет, что isRegistered() корректно определяет наличие фильтра
 */
TEST_F(FilterFactoryTest, IsRegistered)
{
    const std::string registered_name = "registered_filter";
    const std::string unregistered_name = "unregistered_filter";
    
    // Регистрируем один фильтр
    factory_->registerFilter(registered_name, [registered_name](const CLI::App&) {
        return std::make_unique<MockFilter>(registered_name);
    });
    
    // Проверяем зарегистрированный фильтр
    EXPECT_TRUE(factory_->isRegistered(registered_name));
    
    // Проверяем незарегистрированный фильтр
    EXPECT_FALSE(factory_->isRegistered(unregistered_name));
}

/**
 * @brief Тест работы с BufferPool
 * 
 * Проверяет, что FilterFactory корректно работает с пулом буферов
 */
TEST_F(FilterFactoryTest, BufferPool)
{
    // Создаем пул буферов
    BufferPool buffer_pool;
    
    // Устанавливаем пул буферов
    factory_->setBufferPool(&buffer_pool);
    
    // Проверяем, что пул установлен
    EXPECT_EQ(factory_->getBufferPool(), &buffer_pool);
    
    // Устанавливаем nullptr
    factory_->setBufferPool(nullptr);
    
    // Проверяем, что пул сброшен
    EXPECT_EQ(factory_->getBufferPool(), nullptr);
}

/**
 * @brief Тест создания фильтра с параметрами из CLI::App
 * 
 * Проверяет, что фильтры могут получать параметры из CLI::App
 */
TEST_F(FilterFactoryTest, CreateFilterWithParameters)
{
    // Регистрируем все фильтры
    factory_->registerAll();
    
    // Создаем CLI::App с параметрами
    CLI::App app("test");
    double brightness_factor = 2.0;
    app.add_option("--brightness-factor", brightness_factor);
    
    // Создаем фильтр brightness с параметрами
    auto filter = factory_->create("brightness", app);
    
    // Проверяем, что фильтр создан
    ASSERT_NE(filter, nullptr);
    
    // Проверяем, что фильтр может быть применен
    ImageProcessor processor;
    auto test_image = std::vector<uint8_t>(100 * 100 * 3, 128);
    auto* data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    std::memcpy(data, test_image.data(), test_image.size());
    processor.resize(100, 100, data);
    
    auto result = filter->apply(processor);
    EXPECT_TRUE(result.isSuccess());
}

/**
 * @brief Тест перезаписи фильтра
 * 
 * Проверяет, что повторная регистрация фильтра с тем же именем перезаписывает старый
 */
TEST_F(FilterFactoryTest, OverwriteFilter)
{
    const std::string filter_name = "overwrite_test";
    
    // Регистрируем первый фильтр
    factory_->registerFilter(filter_name, [](const CLI::App&) {
        return std::make_unique<MockFilter>("first");
    });
    
    auto filter1 = factory_->create(filter_name, *app_);
    ASSERT_NE(filter1, nullptr);
    EXPECT_EQ(filter1->getName(), "first");
    
    // Регистрируем второй фильтр с тем же именем
    factory_->registerFilter(filter_name, [](const CLI::App&) {
        return std::make_unique<MockFilter>("second");
    });
    
    auto filter2 = factory_->create(filter_name, *app_);
    ASSERT_NE(filter2, nullptr);
    EXPECT_EQ(filter2->getName(), "second");
}

/**
 * @brief Тест создания нескольких фильтров подряд
 * 
 * Проверяет, что можно создать несколько разных фильтров
 */
TEST_F(FilterFactoryTest, CreateMultipleFilters)
{
    // Регистрируем все фильтры
    factory_->registerAll();
    
    // Создаем несколько разных фильтров
    auto filter1 = factory_->create("grayscale", *app_);
    auto filter2 = factory_->create("invert", *app_);
    auto filter3 = factory_->create("sepia", *app_);
    
    // Проверяем, что все фильтры созданы
    ASSERT_NE(filter1, nullptr);
    ASSERT_NE(filter2, nullptr);
    ASSERT_NE(filter3, nullptr);
    
    // Проверяем, что это разные фильтры
    EXPECT_NE(filter1->getName(), filter2->getName());
    EXPECT_NE(filter2->getName(), filter3->getName());
}

/**
 * @brief Тест создания фильтров, использующих BufferPool
 * 
 * Проверяет, что фильтры, которые поддерживают BufferPool, получают его при создании
 */
TEST_F(FilterFactoryTest, FilterWithBufferPool)
{
    // Регистрируем все фильтры
    factory_->registerAll();
    
    // Создаем пул буферов
    BufferPool buffer_pool;
    factory_->setBufferPool(&buffer_pool);
    
    // Создаем фильтр, который использует BufferPool (например, blur)
    CLI::App app("test");
    double blur_radius = 5.0;
    app.add_option("--blur-radius", blur_radius);
    
    auto filter = factory_->create("blur", app);
    
    // Проверяем, что фильтр создан
    ASSERT_NE(filter, nullptr);
    
    // Применяем фильтр для проверки работоспособности
    ImageProcessor processor;
    auto test_image = std::vector<uint8_t>(50 * 50 * 3, 128);
    auto* data = static_cast<uint8_t*>(std::malloc(test_image.size()));
    std::memcpy(data, test_image.data(), test_image.size());
    processor.resize(50, 50, data);
    
    auto result = filter->apply(processor);
    EXPECT_TRUE(result.isSuccess());
}

