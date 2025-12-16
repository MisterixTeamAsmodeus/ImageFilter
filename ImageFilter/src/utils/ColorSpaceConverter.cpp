#include <utils/ColorSpaceConverter.h>
#include <utils/SafeMath.h>

FilterResult ColorSpaceConverter::convertRGBAToRGB(const uint8_t* rgba_data, 
                                                   int width, 
                                                   int height, 
                                                   uint8_t* rgb_data)
{
    size_t width_height_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 4);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    size_t rgb_size = 0;
    if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(3), rgb_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 4);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    size_t pixel_count = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), pixel_count))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 4);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    for (size_t i = 0; i < pixel_count; ++i)
    {
        size_t rgba_offset = 0;
        size_t rgb_offset = 0;
        if (!SafeMath::safeMultiply(i, static_cast<size_t>(4), rgba_offset) ||
            !SafeMath::safeMultiply(i, static_cast<size_t>(3), rgb_offset))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, 4);
            return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                       "Размер изображения слишком большой", ctx);
        }
        
        rgb_data[rgb_offset + 0] = rgba_data[rgba_offset + 0]; // R
        rgb_data[rgb_offset + 1] = rgba_data[rgba_offset + 1]; // G
        rgb_data[rgb_offset + 2] = rgba_data[rgba_offset + 2]; // B
    }
    
    return FilterResult::success();
}

FilterResult ColorSpaceConverter::convertRGBToRGBA(const uint8_t* rgb_data, 
                                                  int width, 
                                                  int height, 
                                                  uint8_t* rgba_data)
{
    size_t width_height_product = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), width_height_product))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 3);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    size_t rgba_size = 0;
    if (!SafeMath::safeMultiply(width_height_product, static_cast<size_t>(4), rgba_size))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 3);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    size_t pixel_count = 0;
    if (!SafeMath::safeMultiply(static_cast<size_t>(width), static_cast<size_t>(height), pixel_count))
    {
        ErrorContext ctx = ErrorContext::withImage(width, height, 3);
        return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                   "Размер изображения слишком большой", ctx);
    }
    
    for (size_t i = 0; i < pixel_count; ++i)
    {
        size_t rgba_offset = 0;
        size_t rgb_offset = 0;
        if (!SafeMath::safeMultiply(i, static_cast<size_t>(4), rgba_offset) ||
            !SafeMath::safeMultiply(i, static_cast<size_t>(3), rgb_offset))
        {
            ErrorContext ctx = ErrorContext::withImage(width, height, 3);
            return FilterResult::failure(FilterError::ArithmeticOverflow, 
                                       "Размер изображения слишком большой", ctx);
        }
        
        rgba_data[rgba_offset + 0] = rgb_data[rgb_offset + 0]; // R
        rgba_data[rgba_offset + 1] = rgb_data[rgb_offset + 1]; // G
        rgba_data[rgba_offset + 2] = rgb_data[rgb_offset + 2]; // B
        rgba_data[rgba_offset + 3] = 255; // A (полная непрозрачность)
    }
    
    return FilterResult::success();
}

