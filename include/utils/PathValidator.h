#pragma once

#include <string>
#include <filesystem>
#include <cstdint>

/**
 * @brief Утилита для валидации путей к файлам
 * 
 * Предоставляет функции для проверки безопасности путей,
 * защиты от path traversal атак и валидации файлов.
 */
namespace PathValidator
{
    /**
     * @brief Проверяет, является ли путь безопасным (защита от path traversal)
     * 
     * Проверяет, что путь не содержит попыток выхода за пределы базовой директории
     * через использование ".." или символьных ссылок.
     * 
     * @param filepath Путь к файлу для проверки
     * @param base_dir Базовая директория (путь должен быть внутри этой директории)
     * @return true если путь безопасен, false в противном случае
     */
    bool isPathSafe(const std::string& filepath, const std::string& base_dir = "");

    /**
     * @brief Нормализует путь и проверяет его безопасность
     * 
     * Преобразует путь в каноническую форму и проверяет на path traversal.
     * 
     * @param filepath Путь к файлу
     * @param base_dir Базовая директория (опционально)
     * @return Нормализованный путь или пустая строка при ошибке
     */
    std::string normalizeAndValidate(const std::string& filepath, const std::string& base_dir = "");

    /**
     * @brief Проверяет размер файла перед загрузкой
     * 
     * @param filepath Путь к файлу
     * @param max_size Максимальный допустимый размер в байтах
     * @return true если размер файла допустим, false в противном случае
     */
    bool validateFileSize(const std::string& filepath, uint64_t max_size);

    /**
     * @brief Проверяет, что путь не содержит опасных символов
     * 
     * @param filepath Путь к файлу
     * @return true если путь безопасен, false в противном случае
     */
    bool containsDangerousCharacters(const std::string& filepath);

    /**
     * @brief Получает размер файла в байтах
     * 
     * @param filepath Путь к файлу
     * @return Размер файла в байтах или 0 при ошибке
     */
    uint64_t getFileSize(const std::string& filepath);

    /**
     * @brief Максимальный размер изображения по умолчанию (1 GB)
     */
    static constexpr uint64_t DEFAULT_MAX_IMAGE_SIZE = 1024ULL * 1024 * 1024;

    /**
     * @brief Максимальный размер файла конфигурации (10 MB)
     */
    static constexpr uint64_t DEFAULT_MAX_CONFIG_SIZE = 10ULL * 1024 * 1024;
}

