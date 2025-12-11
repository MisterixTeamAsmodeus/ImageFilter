#include <utils/PathValidator.h>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace PathValidator
{
    bool isPathSafe(const std::string& filepath, const std::string& base_dir)
    {
        if (filepath.empty())
        {
            return false;
        }

        try
        {
            fs::path path(filepath);
            
            // Нормализуем путь (убираем "..", "." и т.д.)
            fs::path canonical_path;
            if (fs::exists(path))
            {
                canonical_path = fs::canonical(path);
            }
            else
            {
                // Если файл не существует, используем absolute для нормализации
                canonical_path = fs::absolute(path);
                canonical_path = canonical_path.lexically_normal();
            }

            // Если указана базовая директория, проверяем, что путь внутри неё
            if (!base_dir.empty())
            {
                fs::path base_path(base_dir);
                if (fs::exists(base_path))
                {
                    base_path = fs::canonical(base_path);
                }
                else
                {
                    base_path = fs::absolute(base_path);
                    base_path = base_path.lexically_normal();
                }

                // Проверяем, что канонический путь начинается с базового пути
                std::string canonical_str = canonical_path.string();
                std::string base_str = base_path.string();
                
                // Нормализуем разделители для сравнения
                std::replace(canonical_str.begin(), canonical_str.end(), '\\', '/');
                std::replace(base_str.begin(), base_str.end(), '\\', '/');
                
                // Убираем завершающий слеш из базового пути для корректного сравнения
                if (!base_str.empty() && base_str.back() == '/')
                {
                    base_str.pop_back();
                }
                
                if (canonical_str.length() < base_str.length())
                {
                    return false;
                }
                
                // Проверяем, что путь начинается с базового пути
                if (canonical_str.substr(0, base_str.length()) != base_str)
                {
                    return false;
                }
                
                // Проверяем, что следующий символ - разделитель (чтобы избежать частичных совпадений)
                if (canonical_str.length() > base_str.length())
                {
                    if (canonical_str[base_str.length()] != '/')
                    {
                        return false;
                    }
                }
            }

            // Проверяем на наличие ".." в нормализованном пути
            std::string path_str = canonical_path.string();
            if (path_str.find("..") != std::string::npos)
            {
                return false;
            }

            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::string normalizeAndValidate(const std::string& filepath, const std::string& base_dir)
    {
        if (filepath.empty())
        {
            return "";
        }

        try
        {
            fs::path path(filepath);
            fs::path normalized;

            if (fs::exists(path))
            {
                normalized = fs::canonical(path);
            }
            else
            {
                normalized = fs::absolute(path);
                normalized = normalized.lexically_normal();
            }

            // Проверяем безопасность пути
            if (!isPathSafe(normalized.string(), base_dir))
            {
                return "";
            }

            return normalized.string();
        }
        catch (...)
        {
            return "";
        }
    }

    bool validateFileSize(const std::string& filepath, uint64_t max_size)
    {
        try
        {
            if (!fs::exists(filepath))
            {
                return false;
            }

            if (!fs::is_regular_file(filepath))
            {
                return false;
            }

            const auto file_size = fs::file_size(filepath);
            return file_size <= max_size;
        }
        catch (...)
        {
            return false;
        }
    }

    bool containsDangerousCharacters(const std::string& filepath)
    {
        // Проверяем на наличие опасных последовательностей
        if (filepath.find("..") != std::string::npos)
        {
            return true;
        }

        // Проверяем на null-байты
        if (filepath.find('\0') != std::string::npos)
        {
            return true;
        }

        // Проверяем на управляющие символы (кроме разрешенных)
        for (char c : filepath)
        {
            if (std::iscntrl(static_cast<unsigned char>(c)) && c != '\0')
            {
                return true;
            }
        }

        return false;
    }

    uint64_t getFileSize(const std::string& filepath)
    {
        try
        {
            if (!fs::exists(filepath) || !fs::is_regular_file(filepath))
            {
                return 0;
            }

            return static_cast<uint64_t>(fs::file_size(filepath));
        }
        catch (...)
        {
            return 0;
        }
    }
}

