#include <filters/BoxBlurFilter.h>
#include <filters/BrightnessFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/EdgeDetectionFilter.h>
#include <filters/EmbossFilter.h>
#include <filters/FlipHorizontalFilter.h>
#include <filters/FlipVerticalFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/GrayscaleFilter.h>
#include <filters/InvertFilter.h>
#include <filters/MedianFilter.h>
#include <filters/MotionBlurFilter.h>
#include <filters/NoiseFilter.h>
#include <filters/OutlineFilter.h>
#include <filters/PosterizeFilter.h>
#include <filters/Rotate90Filter.h>
#include <filters/SaturationFilter.h>
#include <filters/SepiaFilter.h>
#include <filters/SharpenFilter.h>
#include <filters/ThresholdFilter.h>
#include <filters/VignetteFilter.h>
#include <worker/FilterAdapter.h>

namespace FilterAdapter {
double getDoubleParameter(const std::map<std::string, QVariant>& parameters, const std::string& paramName,
                          double defaultValue) {
    const auto it = parameters.find(paramName);
    if (it != parameters.end() && it->second.canConvert<double>()) {
        return it->second.toDouble();
    }
    return defaultValue;
}

int getIntParameter(const std::map<std::string, QVariant>& parameters, const std::string& paramName, int defaultValue) {
    const auto it = parameters.find(paramName);
    if (it != parameters.end() && it->second.canConvert<int>()) {
        return it->second.toInt();
    }
    return defaultValue;
}

bool getBoolParameter(const std::map<std::string, QVariant>& parameters, const std::string& paramName,
                      bool defaultValue) {
    const auto it = parameters.find(paramName);
    if (it != parameters.end() && it->second.canConvert<bool>()) {
        return it->second.toBool();
    }
    return defaultValue;
}

std::string getStringParameter(const std::map<std::string, QVariant>& parameters, const std::string& paramName,
                               const std::string& defaultValue) {
    const auto it = parameters.find(paramName);
    if (it != parameters.end() && it->second.canConvert<QString>()) {
        return it->second.toString().toStdString();
    }
    return defaultValue;
}

std::unique_ptr<IFilter> createFilter(const std::string& filterName, const std::map<std::string, QVariant>& parameters,
                                      IBufferPool* bufferPool) {
    // Цветовые фильтры
    if (filterName == "grayscale") {
        return std::make_unique<GrayscaleFilter>();
    }
    if (filterName == "sepia") {
        return std::make_unique<SepiaFilter>();
    }
    if (filterName == "invert") {
        return std::make_unique<InvertFilter>();
    }
    if (filterName == "brightness") {
        const double factor = getDoubleParameter(parameters, "brightness_factor", 1.2);
        return std::make_unique<BrightnessFilter>(factor);
    }
    if (filterName == "contrast") {
        const double factor = getDoubleParameter(parameters, "contrast_factor", 1.5);
        return std::make_unique<ContrastFilter>(factor);
    }
    if (filterName == "saturation") {
        const double factor = getDoubleParameter(parameters, "saturation_factor", 1.5);
        return std::make_unique<SaturationFilter>(factor);
    }

    // Геометрические фильтры
    if (filterName == "flip_h") {
        return std::make_unique<FlipHorizontalFilter>();
    }
    if (filterName == "flip_v") {
        return std::make_unique<FlipVerticalFilter>();
    }
    if (filterName == "rotate90") {
        const bool counterClockwise = getBoolParameter(parameters, "counter_clockwise", false);
        return std::make_unique<Rotate90Filter>(!counterClockwise, bufferPool);
    }

    // Фильтры краёв и деталей
    if (filterName == "sharpen") {
        const double strength = getDoubleParameter(parameters, "sharpen_strength", 1.0);
        return std::make_unique<SharpenFilter>(strength, BorderHandler::Strategy::Mirror, bufferPool);
    }
    if (filterName == "edges") {
        const double sensitivity = getDoubleParameter(parameters, "edge_sensitivity", 0.5);
        const std::string operatorStr = getStringParameter(parameters, "edge_operator", "sobel");

        EdgeDetectionFilter::Operator op = EdgeDetectionFilter::Operator::Sobel;
        if (operatorStr == "prewitt") {
            op = EdgeDetectionFilter::Operator::Prewitt;
        } else if (operatorStr == "scharr") {
            op = EdgeDetectionFilter::Operator::Scharr;
        }

        return std::make_unique<EdgeDetectionFilter>(sensitivity, op, BorderHandler::Strategy::Mirror);
    }
    if (filterName == "emboss") {
        const double strength = getDoubleParameter(parameters, "emboss_strength", 1.0);
        return std::make_unique<EmbossFilter>(strength, BorderHandler::Strategy::Mirror);
    }
    if (filterName == "outline") {
        return std::make_unique<OutlineFilter>(BorderHandler::Strategy::Mirror);
    }

    // Фильтры размытия и шума
    if (filterName == "blur") {
        const double radius = getDoubleParameter(parameters, "blur_radius", 5.0);
        return std::make_unique<GaussianBlurFilter>(radius, BorderHandler::Strategy::Mirror, bufferPool);
    }
    if (filterName == "box_blur") {
        const int radius = getIntParameter(parameters, "box_blur_radius", 5);
        return std::make_unique<BoxBlurFilter>(radius, BorderHandler::Strategy::Mirror, bufferPool);
    }
    if (filterName == "motion_blur") {
        const int length = getIntParameter(parameters, "motion_blur_length", 10);
        const double angle = getDoubleParameter(parameters, "motion_blur_angle", 0.0);
        return std::make_unique<MotionBlurFilter>(length, angle, BorderHandler::Strategy::Mirror, bufferPool);
    }
    if (filterName == "median") {
        const int radius = getIntParameter(parameters, "median_radius", 2);
        return std::make_unique<MedianFilter>(radius, BorderHandler::Strategy::Mirror, bufferPool);
    }
    if (filterName == "noise") {
        const double intensity = getDoubleParameter(parameters, "noise_intensity", 0.1);
        return std::make_unique<NoiseFilter>(intensity);
    }

    // Стилистические фильтры
    if (filterName == "posterize") {
        const int levels = getIntParameter(parameters, "posterize_levels", 4);
        return std::make_unique<PosterizeFilter>(levels);
    }
    if (filterName == "threshold") {
        const int threshold = getIntParameter(parameters, "threshold_value", 128);
        return std::make_unique<ThresholdFilter>(threshold);
    }
    if (filterName == "vignette") {
        const double strength = getDoubleParameter(parameters, "vignette_strength", 0.5);
        return std::make_unique<VignetteFilter>(strength);
    }

    // Фильтр не найден
    return nullptr;
}
} // namespace FilterAdapter
