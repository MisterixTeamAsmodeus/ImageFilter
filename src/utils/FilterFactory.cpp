#include <utils/FilterFactory.h>
#include <CLI/CLI.hpp>

#include <filters/GrayscaleFilter.h>
#include <filters/SepiaFilter.h>
#include <filters/InvertFilter.h>
#include <filters/BrightnessFilter.h>
#include <filters/ContrastFilter.h>
#include <filters/SaturationFilter.h>
#include <filters/FlipHorizontalFilter.h>
#include <filters/FlipVerticalFilter.h>
#include <filters/Rotate90Filter.h>
#include <filters/SharpenFilter.h>
#include <filters/EdgeDetectionFilter.h>
#include <filters/EmbossFilter.h>
#include <filters/OutlineFilter.h>
#include <filters/GaussianBlurFilter.h>
#include <filters/BoxBlurFilter.h>
#include <filters/MotionBlurFilter.h>
#include <filters/MedianFilter.h>
#include <filters/NoiseFilter.h>
#include <filters/PosterizeFilter.h>
#include <filters/ThresholdFilter.h>
#include <filters/VignetteFilter.h>
#include <utils/IBufferPool.h>

#include <algorithm>
#include <vector>

namespace
{
    /**
     * @brief Вспомогательная функция для получения значения опции из CLI::App
     * @param app CLI::App объект
     * @param option_name Имя опции
     * @param default_value Значение по умолчанию
     * @return Значение опции или значение по умолчанию
     */
    template<typename T>
    T getOptionValue(const CLI::App& app, const std::string& option_name, const T& default_value)
    {
        try
        {
            const auto* opt = app.get_option(option_name);
            if (opt && opt->count() > 0)
            {
                T value = default_value;
                opt->results(value);
                return value;
            }
        }
        catch (const std::exception&)
        {
            // Опция не найдена, возвращаем значение по умолчанию
        }
        return default_value;
    }
}

FilterFactory& FilterFactory::getInstance()
{
    static FilterFactory instance;
    return instance;
}

void FilterFactory::registerAll()
{
    // Цветовые фильтры
    registerFilter("grayscale", [](const CLI::App&) {
        return std::make_unique<GrayscaleFilter>();
    });

    registerFilter("sepia", [](const CLI::App&) {
        return std::make_unique<SepiaFilter>();
    });

    registerFilter("invert", [](const CLI::App&) {
        return std::make_unique<InvertFilter>();
    });

    registerFilter("brightness", [](const CLI::App& app) {
        const double factor = getOptionValue(app, "--brightness-factor", 1.2);
        return std::make_unique<BrightnessFilter>(factor);
    });

    registerFilter("contrast", [](const CLI::App& app) {
        const double factor = getOptionValue(app, "--contrast-factor", 1.5);
        return std::make_unique<ContrastFilter>(factor);
    });

    registerFilter("saturation", [](const CLI::App& app) {
        const double factor = getOptionValue(app, "--saturation-factor", 1.5);
        return std::make_unique<SaturationFilter>(factor);
    });

    // Геометрические фильтры
    registerFilter("flip_h", [](const CLI::App&) {
        return std::make_unique<FlipHorizontalFilter>();
    });

    registerFilter("flip_v", [](const CLI::App&) {
        return std::make_unique<FlipVerticalFilter>();
    });

    registerFilter("rotate90", [](const CLI::App& app) {
        bool counter_clockwise = false;
        const auto* opt = app.get_option("--counter-clockwise");
        if (opt && opt->count() > 0)
        {
            counter_clockwise = true;
        }
        return std::make_unique<Rotate90Filter>(!counter_clockwise);
    });

    // Фильтры краёв и деталей
    registerFilter("sharpen", [this](const CLI::App& app) {
        const double strength = getOptionValue(app, "--sharpen-strength", 1.0);
        return std::make_unique<SharpenFilter>(strength, BorderHandler::Strategy::Mirror, buffer_pool_);
    });

    registerFilter("edges", [](const CLI::App& app) {
        const double sensitivity = getOptionValue(app, "--edge-sensitivity", 0.5);
        std::string operator_str = getOptionValue(app, "--edge-operator", std::string("sobel"));
        
        // Преобразуем строку в enum
        EdgeDetectionFilter::Operator op = EdgeDetectionFilter::Operator::Sobel;
        if (operator_str == "prewitt")
        {
            op = EdgeDetectionFilter::Operator::Prewitt;
        }
        else if (operator_str == "scharr")
        {
            op = EdgeDetectionFilter::Operator::Scharr;
        }
        // По умолчанию используется Sobel
        
        return std::make_unique<EdgeDetectionFilter>(sensitivity, op);
    });

    registerFilter("emboss", [](const CLI::App& app) {
        const double strength = getOptionValue(app, "--emboss-strength", 1.0);
        return std::make_unique<EmbossFilter>(strength);
    });

    registerFilter("outline", [](const CLI::App&) {
        return std::make_unique<OutlineFilter>();
    });

    // Фильтры размытия и шума
    registerFilter("blur", [this](const CLI::App& app) {
        const double radius = getOptionValue(app, "--blur-radius", 5.0);
        return std::make_unique<GaussianBlurFilter>(radius, BorderHandler::Strategy::Mirror, buffer_pool_);
    });

    registerFilter("box_blur", [this](const CLI::App& app) {
        const int radius = getOptionValue(app, "--box-blur-radius", 5);
        return std::make_unique<BoxBlurFilter>(radius, BorderHandler::Strategy::Mirror, buffer_pool_);
    });

    registerFilter("motion_blur", [this](const CLI::App& app) {
        const int length = getOptionValue(app, "--motion-blur-length", 10);
        const double angle = getOptionValue(app, "--motion-blur-angle", 0.0);
        return std::make_unique<MotionBlurFilter>(length, angle, BorderHandler::Strategy::Mirror, buffer_pool_);
    });

    registerFilter("median", [this](const CLI::App& app) {
        const int radius = getOptionValue(app, "--median-radius", 2);
        return std::make_unique<MedianFilter>(radius, BorderHandler::Strategy::Mirror, buffer_pool_);
    });

    registerFilter("noise", [](const CLI::App& app) {
        const double intensity = getOptionValue(app, "--noise-intensity", 0.1);
        return std::make_unique<NoiseFilter>(intensity);
    });

    // Стилистические фильтры
    registerFilter("posterize", [](const CLI::App& app) {
        const int levels = getOptionValue(app, "--posterize-levels", 4);
        return std::make_unique<PosterizeFilter>(levels);
    });

    registerFilter("threshold", [](const CLI::App& app) {
        const int threshold = getOptionValue(app, "--threshold-value", 128);
        return std::make_unique<ThresholdFilter>(threshold);
    });

    registerFilter("vignette", [](const CLI::App& app) {
        const double strength = getOptionValue(app, "--vignette-strength", 0.5);
        return std::make_unique<VignetteFilter>(strength);
    });
}

void FilterFactory::registerFilter(const std::string& name, FilterCreator creator)
{
    creators_[name] = std::move(creator);
}

std::unique_ptr<IFilter> FilterFactory::create(const std::string& name, const CLI::App& app) const
{
    const auto it = creators_.find(name);
    if (it != creators_.end())
    {
        return it->second(app);
    }
    return nullptr;
}

bool FilterFactory::isRegistered(const std::string& name) const
{
    return creators_.find(name) != creators_.end();
}

std::vector<std::string> FilterFactory::getRegisteredFilters() const
{
    std::vector<std::string> filters;
    filters.reserve(creators_.size());
    
    for (const auto& [name, _] : creators_)
    {
        filters.push_back(name);
    }
    
    std::sort(filters.begin(), filters.end());
    return filters;
}

void FilterFactory::setBufferPool(IBufferPool* buffer_pool) noexcept
{
    buffer_pool_ = buffer_pool;
}

IBufferPool* FilterFactory::getBufferPool() const noexcept
{
    return buffer_pool_;
}
