#pragma once

#include <vector>
#include <string>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;

/**
 * @brief Утилиты для тестирования
 */
namespace TestUtils
{
    /**
     * @brief Создает простое тестовое изображение в памяти (RGB)
     */
    std::vector<uint8_t> createTestImage(int width, int height);

    /**
     * @brief Создает простое тестовое изображение в памяти (RGBA)
     */
    std::vector<uint8_t> createTestImageRGBA(int width, int height);

    /**
     * @brief Сохраняет тестовое изображение как PNG
     */
    std::string saveTestImagePNG(
        const std::vector<uint8_t>& data,
        int width,
        int height,
        const std::string& filepath
    );
}
