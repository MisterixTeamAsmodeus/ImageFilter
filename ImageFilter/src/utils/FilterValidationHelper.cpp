#include <utils/FilterValidationHelper.h>
#include <utils/ImageValidator.h>
#include <ImageProcessor.h>
#include <utils/FilterResult.h>

namespace FilterValidationHelper
{
    FilterResult validateImageOnly(const ImageProcessor& image)
    {
        return ImageValidator::validateBasic(image);
    }
}

