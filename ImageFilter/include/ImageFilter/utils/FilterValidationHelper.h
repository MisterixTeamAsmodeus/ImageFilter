#pragma once

#include <utils/FilterResult.h>
#include <ImageProcessor.h>
#include <string>

namespace ImageValidator
{
    FilterResult validateBasic(const ImageProcessor& image);
}

/**
 * @brief Вспомогательные функции для валидации фильтров
 * 
 * Предоставляет общие функции для валидации изображения и параметров фильтров,
 * устраняя дублирование кода в различных фильтрах.
 */
namespace FilterValidationHelper
{
    /**
     * @brief Валидирует изображение и параметр фильтра с автоматическим добавлением контекста
     * 
     * Выполняет базовую валидацию изображения, затем валидацию параметра фильтра.
     * Автоматически добавляет параметр фильтра в контекст ошибки.
     * 
     * @param image Изображение для валидации
     * @param param_validation_result Результат валидации параметра фильтра
     * @param param_name Имя параметра фильтра (для контекста ошибки)
     * @param param_value Значение параметра фильтра (для контекста ошибки)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    template<typename T>
    FilterResult validateImageAndParam(const ImageProcessor& image,
                                      const FilterResult& param_validation_result,
                                      const std::string& param_name,
                                      T param_value)
    {
        // Базовая валидация изображения
        auto basic_result = ImageValidator::validateBasic(image);
        if (basic_result.hasError())
        {
            ErrorContext ctx = basic_result.context.value_or(ErrorContext());
            ctx.withFilterParam(param_name, param_value);
            return FilterResult::failure(basic_result.error, basic_result.message, ctx);
        }

        const auto width = image.getWidth();
        const auto height = image.getHeight();
        const auto channels = image.getChannels();

        // Валидация параметра фильтра
        if (param_validation_result.hasError())
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, channels);
            ctx.withFilterParam(param_name, param_value);
            return FilterResult::failure(param_validation_result.error, 
                                       param_validation_result.message, ctx);
        }

        return FilterResult::success();
    }

    /**
     * @brief Валидирует только изображение с добавлением параметра фильтра в контекст
     * 
     * Используется для фильтров, которые не требуют валидации параметров
     * или валидация параметров выполняется отдельно.
     * 
     * @param image Изображение для валидации
     * @param param_name Имя параметра фильтра (для контекста ошибки, опционально)
     * @param param_value Значение параметра фильтра (для контекста ошибки, опционально)
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    template<typename T>
    FilterResult validateImageWithParam(const ImageProcessor& image,
                                       const std::string& param_name,
                                       T param_value)
    {
        auto basic_result = ImageValidator::validateBasic(image);
        if (basic_result.hasError())
        {
            ErrorContext ctx = basic_result.context.value_or(ErrorContext());
            ctx.withFilterParam(param_name, param_value);
            return FilterResult::failure(basic_result.error, basic_result.message, ctx);
        }
        return FilterResult::success();
    }

    /**
     * @brief Валидирует только изображение без параметров фильтра
     * 
     * Используется для фильтров без параметров.
     * 
     * @param image Изображение для валидации
     * @return FilterResult с ошибкой, если валидация не прошла, иначе Success
     */
    FilterResult validateImageOnly(const ImageProcessor& image);
}

