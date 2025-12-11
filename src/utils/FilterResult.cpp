#include <utils/FilterResult.h>

std::string filterErrorToString(FilterError error)
{
    switch (error)
    {
        // Успешное выполнение
        case FilterError::Success:
            return "Успешно";

        // Ошибки валидации (1-99)
        case FilterError::InvalidImage:
            return "Невалидное изображение";
        case FilterError::InvalidChannels:
            return "Невалидное количество каналов";
        case FilterError::InvalidParameter:
            return "Невалидный параметр";
        case FilterError::InvalidWidth:
            return "Невалидная ширина изображения";
        case FilterError::InvalidHeight:
            return "Невалидная высота изображения";
        case FilterError::InvalidSize:
            return "Невалидный размер изображения";
        case FilterError::InvalidRadius:
            return "Невалидный радиус (должен быть > 0)";
        case FilterError::InvalidFactor:
            return "Невалидный коэффициент (должен быть > 0)";
        case FilterError::InvalidThreshold:
            return "Невалидное пороговое значение";
        case FilterError::InvalidQuality:
            return "Невалидное качество (должно быть в диапазоне 0-100)";
        case FilterError::InvalidAngle:
            return "Невалидный угол поворота";
        case FilterError::InvalidKernelSize:
            return "Невалидный размер ядра свертки";
        case FilterError::ParameterOutOfRange:
            return "Параметр вне допустимого диапазона";
        case FilterError::EmptyImage:
            return "Изображение пустое (нулевой размер)";

        // Ошибки памяти (100-199)
        case FilterError::OutOfMemory:
            return "Недостаточно памяти";
        case FilterError::MemoryAllocationFailed:
            return "Ошибка выделения памяти";
        case FilterError::MemoryOverflow:
            return "Переполнение памяти при вычислениях";

        // Ошибки формата (200-299)
        case FilterError::UnsupportedFormat:
            return "Неподдерживаемый формат изображения";
        case FilterError::UnsupportedChannels:
            return "Неподдерживаемое количество каналов";
        case FilterError::CorruptedImage:
            return "Поврежденное изображение";
        case FilterError::InvalidImageData:
            return "Некорректные данные изображения";
        case FilterError::FormatMismatch:
            return "Несоответствие формата ожидаемому";

        // Ошибки вычислений (300-399)
        case FilterError::DivisionByZero:
            return "Деление на ноль";
        case FilterError::ArithmeticOverflow:
            return "Арифметическое переполнение";
        case FilterError::ArithmeticUnderflow:
            return "Арифметическое исчерпание";
        case FilterError::InvalidKernel:
            return "Некорректное ядро свертки";
        case FilterError::KernelNormalizationFailed:
            return "Ошибка нормализации ядра";

        // Ошибки файловых операций (400-499)
        case FilterError::FileOperationFailed:
            return "Ошибка операции с файлом";
        case FilterError::FileNotFound:
            return "Файл не найден";
        case FilterError::FileReadError:
            return "Ошибка чтения файла";
        case FilterError::FileWriteError:
            return "Ошибка записи файла";
        case FilterError::FilePermissionDenied:
            return "Отказано в доступе к файлу";
        case FilterError::FileTooLarge:
            return "Файл слишком большой";
        case FilterError::InvalidFilePath:
            return "Некорректный путь к файлу";

        // Системные ошибки (500-599)
        case FilterError::SystemError:
            return "Системная ошибка";
        case FilterError::InvalidSystemCall:
            return "Некорректный системный вызов";
    }
    __builtin_unreachable();
}
