#include <filters/ContrastFilter.h>
#include <ImageProcessor.h>
#include <utils/ParallelImageProcessor.h>
#include <algorithm>

bool ContrastFilter::apply(ImageProcessor& image) {
    if (!image.isValid()) {
        return false;
    }

    const auto width = image.getWidth();
    const auto height = image.getHeight();
    const auto channels = image.getChannels();

    if (channels != 3) {
        return false;
    }

    auto* data = image.getData();
    const auto factor = static_cast<int>(factor_ * 65536);  // Масштабируем для целочисленной арифметики
    constexpr int MIDDLE = 128;

    ParallelImageProcessor::processRowsParallel(
        height,
        [width, channels, data, factor](int start_row, int end_row) {
            for (int y = start_row; y < end_row; ++y) {
                const auto row_offset = static_cast<size_t>(y) * width * channels;
                
                for (int x = 0; x < width; ++x) {
                    const auto pixel_offset = row_offset + static_cast<size_t>(x) * channels;
                    
                    for (int c = 0; c < channels; ++c) {
                        const auto old_value = static_cast<int>(data[pixel_offset + c]);
                        const auto diff = old_value - MIDDLE;
                        const auto new_value = ((diff * factor) >> 16) + MIDDLE;
                        data[pixel_offset + c] = static_cast<uint8_t>(std::max(0, std::min(255, new_value)));
                    }
                }
            }
        }
    );

    return true;
}



