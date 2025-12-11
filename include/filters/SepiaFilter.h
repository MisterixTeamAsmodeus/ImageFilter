#pragma once

#include <filters/IFilter.h>

/**
 * @brief Фильтр сепии (винтажный эффект)
 * 
 * Применяет классический эффект сепии, который придает изображению
 * коричневатый оттенок, имитирующий старые фотографии.
 * 
 * Формула преобразования:
 * R' = (R * 0.393) + (G * 0.769) + (B * 0.189)
 * G' = (R * 0.349) + (G * 0.686) + (B * 0.168)
 * B' = (R * 0.272) + (G * 0.534) + (B * 0.131)
 */
class SepiaFilter : public IFilter {
public:
    /**
     * @brief Применяет фильтр сепии к изображению
     * @param image Обрабатываемое изображение
     * @return FilterResult с кодом ошибки
     */
    FilterResult apply(ImageProcessor& image) override;

    std::string getName() const override;
    std::string getDescription() const override;
    std::string getCategory() const override;
};



