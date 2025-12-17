#include <gui/FilterWidgetFactory.h>

#include <gui/BaseFilterConfigWidget.h>
#include <gui/filters/BrightnessFilterWidget.h>
#include <gui/filters/BlurFilterWidget.h>
#include <gui/filters/BoxBlurFilterWidget.h>
#include <gui/filters/ContrastFilterWidget.h>
#include <gui/filters/MotionBlurFilterWidget.h>
#include <gui/filters/SaturationFilterWidget.h>

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

    return nullptr;
}

