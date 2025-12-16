#include <cmath>
#include <mutex>
#include <numbers>
#include <utils/LookupTables.h>
#include <utils/CacheManager.h>
#include <algorithm>

namespace {
// Константы размеров таблиц (соответствуют константам в LookupTables)
constexpr int SIN_COS_TABLE_SIZE = 360;
constexpr int EXP_TABLE_SIZE = 2001;   // [0, 20] с шагом 0.01
constexpr int SQRT_TABLE_SIZE = 10001; // [0, 10000]
constexpr double EXP_TABLE_STEP = 0.01;
constexpr double EXP_TABLE_MAX = 20.0;

/**
 * @brief Нормализует угол в диапазон [0, 360)
 * @param angle_degrees Угол в градусах
 * @return Нормализованный угол
 */
int normalizeAngle(int angle_degrees) noexcept {
    // Нормализуем угол в диапазон [0, 360)
    angle_degrees %= 360;
    if (angle_degrees < 0) {
        angle_degrees += 360;
    }
    return angle_degrees;
}

/**
 * @brief Получает ссылку на таблицу sin (масштабированная)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу sin (масштабированная)
 */
std::array<int32_t, SIN_COS_TABLE_SIZE>& getSinTableScaled() noexcept {
    static std::array<int32_t, SIN_COS_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу cos (масштабированная)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу cos (масштабированная)
 */
std::array<int32_t, SIN_COS_TABLE_SIZE>& getCosTableScaled() noexcept {
    static std::array<int32_t, SIN_COS_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу sin (double)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу sin (double)
 */
std::array<double, SIN_COS_TABLE_SIZE>& getSinTable() noexcept {
    static std::array<double, SIN_COS_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу cos (double)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу cos (double)
 */
std::array<double, SIN_COS_TABLE_SIZE>& getCosTable() noexcept {
    static std::array<double, SIN_COS_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу exp(-x)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу exp(-x)
 */
std::array<double, EXP_TABLE_SIZE>& getExpNegativeTable() noexcept {
    static std::array<double, EXP_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу sqrt (double)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу sqrt (double)
 */
std::array<double, SQRT_TABLE_SIZE>& getSqrtTable() noexcept {
    static std::array<double, SQRT_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на таблицу sqrt (масштабированная)
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на таблицу sqrt (масштабированная)
 */
std::array<int32_t, SQRT_TABLE_SIZE>& getSqrtTableScaled() noexcept {
    static std::array<int32_t, SQRT_TABLE_SIZE> table{};
    return table;
}

/**
 * @brief Получает ссылку на флаг инициализации
 * Использует локальную статическую переменную для избежания exit-time destructors
 * @return Ссылка на флаг инициализации
 */
bool& getInitialized() noexcept {
    static bool initialized = false;
    return initialized;
}
} // namespace

void LookupTables::initialize() {
    static std::once_flag flag;
    std::call_once(flag, []() {
        auto& sin_table = getSinTable();
        auto& cos_table = getCosTable();
        auto& sin_table_scaled = getSinTableScaled();
        auto& cos_table_scaled = getCosTableScaled();
        auto& exp_negative_table = getExpNegativeTable();
        auto& sqrt_table = getSqrtTable();
        auto& sqrt_table_scaled = getSqrtTableScaled();

        // Инициализация таблиц sin/cos
        for (int i = 0; i < SIN_COS_TABLE_SIZE; ++i) {
            const auto angle_rad = static_cast<double>(i) * std::numbers::pi / 180.0;
            sin_table[static_cast<size_t>(i)] = std::sin(angle_rad);
            cos_table[static_cast<size_t>(i)] = std::cos(angle_rad);
            sin_table_scaled[static_cast<size_t>(i)] =
                static_cast<int32_t>(sin_table[static_cast<size_t>(i)] * 65536.0);
            cos_table_scaled[static_cast<size_t>(i)] =
                static_cast<int32_t>(cos_table[static_cast<size_t>(i)] * 65536.0);
        }

        // Инициализация таблицы exp(-x)
        for (int i = 0; i < EXP_TABLE_SIZE; ++i) {
            const auto x = static_cast<double>(i) * EXP_TABLE_STEP;
            exp_negative_table[static_cast<size_t>(i)] = std::exp(-x);
        }

        // Инициализация таблицы sqrt
        for (int i = 0; i < SQRT_TABLE_SIZE; ++i) {
            sqrt_table[static_cast<size_t>(i)] = std::sqrt(static_cast<double>(i));
            sqrt_table_scaled[static_cast<size_t>(i)] =
                static_cast<int32_t>(sqrt_table[static_cast<size_t>(i)] * 65536.0);
        }

        getInitialized() = true;
    });
}

int32_t LookupTables::sinScaled(int angle_degrees) noexcept {
    if (!getInitialized()) {
        initialize();
    }
    return getSinTableScaled()[static_cast<size_t>(normalizeAngle(angle_degrees))];
}

int32_t LookupTables::cosScaled(int angle_degrees) noexcept {
    if (!getInitialized()) {
        initialize();
    }
    return getCosTableScaled()[static_cast<size_t>(normalizeAngle(angle_degrees))];
}

double LookupTables::sin(int angle_degrees) noexcept {
    if (!getInitialized()) {
        initialize();
    }
    return getSinTable()[static_cast<size_t>(normalizeAngle(angle_degrees))];
}

double LookupTables::cos(int angle_degrees) noexcept {
    if (!getInitialized()) {
        initialize();
    }
    return getCosTable()[static_cast<size_t>(normalizeAngle(angle_degrees))];
}

double LookupTables::expNegative(double x) noexcept {
    if (!getInitialized()) {
        initialize();
    }

    if (x < 0.0) {
        // Для отрицательных значений возвращаем exp(x) = 1/exp(-x)
        return 1.0 / expNegative(-x);
    }

    if (x > EXP_TABLE_MAX) {
        // Для больших значений exp(-x) очень близко к 0
        return 0.0;
    }

    auto& exp_negative_table = getExpNegativeTable();

    // Линейная интерполяция между ближайшими значениями в таблице
    const auto index = x / EXP_TABLE_STEP;
    const auto index_low = static_cast<int>(index);
    const auto index_high = index_low + 1;

    if (index_high >= EXP_TABLE_SIZE) {
        return exp_negative_table[EXP_TABLE_SIZE - 1];
    }

    const auto t = index - static_cast<double>(index_low);
    return exp_negative_table[static_cast<size_t>(index_low)] * (1.0 - t) +
           exp_negative_table[static_cast<size_t>(index_high)] * t;
}

double LookupTables::sqrtInt(int x) noexcept {
    if (!getInitialized()) {
        initialize();
    }

    if (x < 0) {
        return 0.0;
    }

    if (x < SQRT_TABLE_SIZE) {
        return getSqrtTable()[static_cast<size_t>(x)];
    }

    // Для значений вне таблицы используем std::sqrt
    return std::sqrt(static_cast<double>(x));
}

int32_t LookupTables::sqrtIntScaled(int x) noexcept {
    if (!getInitialized()) {
        initialize();
    }

    if (x < 0) {
        return 0;
    }

    if (x < SQRT_TABLE_SIZE) {
        return getSqrtTableScaled()[static_cast<size_t>(x)];
    }

    // Для значений вне таблицы вычисляем и масштабируем
    return static_cast<int32_t>(std::sqrt(static_cast<double>(x)) * 65536.0);
}

std::vector<uint8_t> LookupTables::getGammaLUT(double gamma) noexcept
{
    if (gamma <= 0.0)
    {
        gamma = 1.0;  // Нормализуем некорректные значения
    }
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Gamma;
    key.parameter = gamma;
    
    auto& cache_manager = CacheManager::getInstance();
    return cache_manager.getOrGenerateLUT(key, [gamma]() {
        std::vector<uint8_t> lut(256);
        const double inv_gamma = 1.0 / gamma;
        for (int i = 0; i < 256; ++i)
        {
            const double normalized = static_cast<double>(i) / 255.0;
            const double corrected = std::pow(normalized, inv_gamma);
            lut[static_cast<size_t>(i)] = static_cast<uint8_t>(
                std::max(0.0, std::min(255.0, corrected * 255.0)));
        }
        return lut;
    });
}

std::vector<uint8_t> LookupTables::getBrightnessLUT(double brightness) noexcept
{
    // Ограничиваем диапазон [-1.0, 1.0]
    brightness = std::max(-1.0, std::min(1.0, brightness));
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Brightness;
    key.parameter = brightness;
    
    auto& cache_manager = CacheManager::getInstance();
    return cache_manager.getOrGenerateLUT(key, [brightness]() {
        std::vector<uint8_t> lut(256);
        const double factor = 1.0 + brightness;
        for (int i = 0; i < 256; ++i)
        {
            const double value = static_cast<double>(i) * factor;
            lut[static_cast<size_t>(i)] = static_cast<uint8_t>(
                std::max(0.0, std::min(255.0, value)));
        }
        return lut;
    });
}

std::vector<uint8_t> LookupTables::getContrastLUT(double contrast) noexcept
{
    // Ограничиваем диапазон [-1.0, 1.0]
    contrast = std::max(-1.0, std::min(1.0, contrast));
    
    LUTCacheKey key;
    key.type = LUTCacheKey::Type::Contrast;
    key.parameter = contrast;
    
    auto& cache_manager = CacheManager::getInstance();
    return cache_manager.getOrGenerateLUT(key, [contrast]() {
        std::vector<uint8_t> lut(256);
        // Преобразуем контраст из [-1, 1] в множитель
        const double factor = (1.0 + contrast) / (1.0 - contrast);
        const double offset = 128.0 * (1.0 - factor);
        
        for (int i = 0; i < 256; ++i)
        {
            const double value = static_cast<double>(i) * factor + offset;
            lut[static_cast<size_t>(i)] = static_cast<uint8_t>(
                std::max(0.0, std::min(255.0, value)));
        }
        return lut;
    });
}
