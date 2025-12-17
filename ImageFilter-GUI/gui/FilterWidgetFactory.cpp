#include <gui/FilterWidgetFactory.h>

#include <gui/BaseFilterConfigWidget.h>
#include <gui/filters/BrightnessFilterWidget.h>
#include <gui/filters/BlurFilterWidget.h>
#include <gui/filters/BoxBlurFilterWidget.h>
#include <gui/filters/ContrastFilterWidget.h>
#include <gui/filters/EdgeDetectionFilterWidget.h>
#include <gui/filters/EmbossFilterWidget.h>
#include <gui/filters/MedianFilterWidget.h>
#include <gui/filters/MotionBlurFilterWidget.h>
#include <gui/filters/NoiseFilterWidget.h>
#include <gui/filters/PosterizeFilterWidget.h>
#include <gui/filters/Rotate90FilterWidget.h>
#include <gui/filters/SaturationFilterWidget.h>
#include <gui/filters/SharpenFilterWidget.h>
#include <gui/filters/ThresholdFilterWidget.h>
#include <gui/filters/VignetteFilterWidget.h>

BaseFilterConfigWidget* FilterWidgetFactory::create(const std::string& filterName, QWidget* parent) const
{
    if (filterName == "brightness")
    {
        return new BrightnessFilterWidget(parent);
    }

    if (filterName == "contrast")
    {
        return new ContrastFilterWidget(parent);
    }

    if (filterName == "saturation")
    {
        return new SaturationFilterWidget(parent);
    }

    if (filterName == "blur")
    {
        return new BlurFilterWidget(parent);
    }

    if (filterName == "box_blur")
    {
        return new BoxBlurFilterWidget(parent);
    }

    if (filterName == "motion_blur")
    {
        return new MotionBlurFilterWidget(parent);
    }

    if (filterName == "rotate90")
    {
        return new Rotate90FilterWidget(parent);
    }

    if (filterName == "sharpen")
    {
        return new SharpenFilterWidget(parent);
    }

    if (filterName == "edges")
    {
        return new EdgeDetectionFilterWidget(parent);
    }

    if (filterName == "emboss")
    {
        return new EmbossFilterWidget(parent);
    }

    if (filterName == "median")
    {
        return new MedianFilterWidget(parent);
    }

    if (filterName == "noise")
    {
        return new NoiseFilterWidget(parent);
    }

    if (filterName == "posterize")
    {
        return new PosterizeFilterWidget(parent);
    }

    if (filterName == "threshold")
    {
        return new ThresholdFilterWidget(parent);
    }

    if (filterName == "vignette")
    {
        return new VignetteFilterWidget(parent);
    }

    return nullptr;
}

