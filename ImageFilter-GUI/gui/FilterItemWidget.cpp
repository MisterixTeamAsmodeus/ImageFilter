#include <gui/FilterItemWidget.h>
#include <model/FilterChainModel.h>
#include <QLabel>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

FilterItemWidget::FilterItemWidget(
    size_t filterIndex,
    const std::string& filterName,
    const std::map<std::string, QVariant>& parameters,
    bool canRemove,
    bool canMoveUp,
    bool canMoveDown,
    FilterChainModel* filterChainModel,
    QWidget* parent)
    : QWidget(parent)
    , filterIndex_(filterIndex)
    , filterName_(filterName)
    , filterChainModel_(filterChainModel)
    , mainLayout_(nullptr)
    , headerLayout_(nullptr)
    , filterNameLabel_(nullptr)
    , changeTypeButton_(nullptr)
    , removeButton_(nullptr)
    , moveUpButton_(nullptr)
    , moveDownButton_(nullptr)
    , parametersLayout_(nullptr)
    , updatingParameters_(false)
{
    setupUI();
    createParameterWidgets(filterName, parameters);
    updateButtonsState(canRemove, canMoveUp, canMoveDown);
}

void FilterItemWidget::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(5, 5, 5, 5);
    mainLayout_->setSpacing(5);

    // Заголовок с именем фильтра и кнопками управления
    headerLayout_ = new QHBoxLayout();
    headerLayout_->setContentsMargins(0, 0, 0, 0);
    headerLayout_->setSpacing(5);

    filterNameLabel_ = new QLabel(getFilterDisplayName(filterName_), this);
    filterNameLabel_->setStyleSheet("QLabel { font-weight: bold; }");
    headerLayout_->addWidget(filterNameLabel_);

    // Кнопка изменения типа фильтра
    changeTypeButton_ = new QPushButton("✎", this);
    changeTypeButton_->setMaximumWidth(30);
    changeTypeButton_->setToolTip("Изменить тип фильтра");
    connect(changeTypeButton_, &QPushButton::clicked, this, &FilterItemWidget::onChangeFilterTypeClicked);
    headerLayout_->addWidget(changeTypeButton_);

    headerLayout_->addStretch();

    // Кнопки управления
    moveUpButton_ = new QPushButton("↑", this);
    moveUpButton_->setMaximumWidth(30);
    moveUpButton_->setToolTip("Переместить вверх");
    connect(moveUpButton_, &QPushButton::clicked, this, &FilterItemWidget::onMoveUpClicked);
    headerLayout_->addWidget(moveUpButton_);

    moveDownButton_ = new QPushButton("↓", this);
    moveDownButton_->setMaximumWidth(30);
    moveDownButton_->setToolTip("Переместить вниз");
    connect(moveDownButton_, &QPushButton::clicked, this, &FilterItemWidget::onMoveDownClicked);
    headerLayout_->addWidget(moveDownButton_);

    removeButton_ = new QPushButton("×", this);
    removeButton_->setMaximumWidth(30);
    removeButton_->setToolTip("Удалить фильтр");
    connect(removeButton_, &QPushButton::clicked, this, &FilterItemWidget::onRemoveClicked);
    headerLayout_->addWidget(removeButton_);

    mainLayout_->addLayout(headerLayout_);

    // Layout для параметров
    parametersLayout_ = new QVBoxLayout();
    parametersLayout_->setContentsMargins(10, 0, 0, 0);
    parametersLayout_->setSpacing(5);
    mainLayout_->addLayout(parametersLayout_);

    // Рамка вокруг виджета
    setStyleSheet("QWidget { border: 1px solid #ccc; border-radius: 3px; background-color: #f9f9f9; }");
    setMinimumHeight(50);
}

void FilterItemWidget::createParameterWidgets(const std::string& filterName, const std::map<std::string, QVariant>& parameters)
{
    // Очищаем существующие виджеты параметров из layout
    if (parametersLayout_ != nullptr)
    {
        QLayoutItem* item;
        while ((item = parametersLayout_->takeAt(0)) != nullptr)
        {
            if (item->widget() != nullptr)
            {
                item->widget()->deleteLater();
            }
            delete item;
        }
    }

    // Очищаем map виджетов параметров
    parameterWidgets_.clear();

    // Создаем виджеты параметров в зависимости от типа фильтра
    if (filterName == "brightness")
    {
        createBrightnessWidget(parameters.count("brightness_factor") > 0 ? parameters.at("brightness_factor") : QVariant(1.2));
    }
    else if (filterName == "contrast")
    {
        createContrastWidget(parameters.count("contrast_factor") > 0 ? parameters.at("contrast_factor") : QVariant(1.5));
    }
    else if (filterName == "saturation")
    {
        createSaturationWidget(parameters.count("saturation_factor") > 0 ? parameters.at("saturation_factor") : QVariant(1.5));
    }
    else if (filterName == "blur")
    {
        createBlurRadiusWidget(parameters.count("blur_radius") > 0 ? parameters.at("blur_radius") : QVariant(5.0));
    }
    else if (filterName == "box_blur")
    {
        createBoxBlurRadiusWidget(parameters.count("box_blur_radius") > 0 ? parameters.at("box_blur_radius") : QVariant(5));
    }
    else if (filterName == "motion_blur")
    {
        createMotionBlurLengthWidget(parameters.count("motion_blur_length") > 0 ? parameters.at("motion_blur_length") : QVariant(10));
        createMotionBlurAngleWidget(parameters.count("motion_blur_angle") > 0 ? parameters.at("motion_blur_angle") : QVariant(0.0));
    }
    else if (filterName == "median")
    {
        createMedianRadiusWidget(parameters.count("median_radius") > 0 ? parameters.at("median_radius") : QVariant(2));
    }
    else if (filterName == "noise")
    {
        createNoiseIntensityWidget(parameters.count("noise_intensity") > 0 ? parameters.at("noise_intensity") : QVariant(0.1));
    }
    else if (filterName == "posterize")
    {
        createPosterizeLevelsWidget(parameters.count("posterize_levels") > 0 ? parameters.at("posterize_levels") : QVariant(4));
    }
    else if (filterName == "threshold")
    {
        createThresholdValueWidget(parameters.count("threshold_value") > 0 ? parameters.at("threshold_value") : QVariant(128));
    }
    else if (filterName == "vignette")
    {
        createVignetteStrengthWidget(parameters.count("vignette_strength") > 0 ? parameters.at("vignette_strength") : QVariant(0.5));
    }
    else if (filterName == "sharpen")
    {
        createSharpenStrengthWidget(parameters.count("sharpen_strength") > 0 ? parameters.at("sharpen_strength") : QVariant(1.0));
    }
    else if (filterName == "edges")
    {
        createEdgeSensitivityWidget(parameters.count("edge_sensitivity") > 0 ? parameters.at("edge_sensitivity") : QVariant(0.5));
        createEdgeOperatorWidget(parameters.count("edge_operator") > 0 ? parameters.at("edge_operator") : QVariant("sobel"));
    }
    else if (filterName == "emboss")
    {
        createEmbossStrengthWidget(parameters.count("emboss_strength") > 0 ? parameters.at("emboss_strength") : QVariant(1.0));
    }
    else if (filterName == "rotate90")
    {
        createCounterClockwiseWidget(parameters.count("counter_clockwise") > 0 ? parameters.at("counter_clockwise") : QVariant(false));
    }
}

void FilterItemWidget::createBrightnessWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Яркость:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 5.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    spinBox->setSuffix("x");
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["brightness_factor"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createContrastWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Контраст:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 5.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    spinBox->setSuffix("x");
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["contrast_factor"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createSaturationWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Насыщенность:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 5.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    spinBox->setSuffix("x");
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["saturation_factor"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createBlurRadiusWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Радиус размытия:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 50.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["blur_radius"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createBoxBlurRadiusWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Радиус:", this);
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(1, 50);
    spinBox->setValue(value.toInt());
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["box_blur_radius"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createMotionBlurLengthWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Длина:", this);
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(1, 100);
    spinBox->setValue(value.toInt());
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["motion_blur_length"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createMotionBlurAngleWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Угол:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.0, 360.0);
    spinBox->setSingleStep(1.0);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    spinBox->setSuffix("°");
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["motion_blur_angle"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createMedianRadiusWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Радиус:", this);
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(1, 10);
    spinBox->setValue(value.toInt());
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["median_radius"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createNoiseIntensityWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Интенсивность:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.0, 1.0);
    spinBox->setSingleStep(0.01);
    spinBox->setDecimals(2);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["noise_intensity"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createPosterizeLevelsWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Уровни:", this);
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(2, 256);
    spinBox->setValue(value.toInt());
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["posterize_levels"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createThresholdValueWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Порог:", this);
    auto* spinBox = new QSpinBox(this);
    spinBox->setRange(0, 255);
    spinBox->setValue(value.toInt());
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["threshold_value"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createVignetteStrengthWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Сила эффекта:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.0, 1.0);
    spinBox->setSingleStep(0.01);
    spinBox->setDecimals(2);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["vignette_strength"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createSharpenStrengthWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Сила:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 5.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["sharpen_strength"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createEdgeSensitivityWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Чувствительность:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 2.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["edge_sensitivity"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createEdgeOperatorWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Оператор:", this);
    auto* comboBox = new QComboBox(this);
    comboBox->addItem("Sobel", "sobel");
    comboBox->addItem("Prewitt", "prewitt");
    comboBox->addItem("Scharr", "scharr");
    const QString currentValue = value.toString();
    const int index = comboBox->findData(currentValue);
    if (index >= 0)
    {
        comboBox->setCurrentIndex(index);
    }
    connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["edge_operator"] = comboBox;
    layout->addWidget(label);
    layout->addWidget(comboBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createEmbossStrengthWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* label = new QLabel("Сила:", this);
    auto* spinBox = new QDoubleSpinBox(this);
    spinBox->setRange(0.1, 5.0);
    spinBox->setSingleStep(0.1);
    spinBox->setDecimals(1);
    spinBox->setValue(value.toDouble());
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["emboss_strength"] = spinBox;
    layout->addWidget(label);
    layout->addWidget(spinBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

void FilterItemWidget::createCounterClockwiseWidget(const QVariant& value)
{
    auto* layout = new QHBoxLayout();
    auto* checkBox = new QCheckBox("Против часовой стрелки", this);
    checkBox->setChecked(value.toBool());
    connect(checkBox, &QCheckBox::toggled,
            this, &FilterItemWidget::onParameterChanged);
    parameterWidgets_["counter_clockwise"] = checkBox;
    layout->addWidget(checkBox);
    layout->addStretch();
    parametersLayout_->addLayout(layout);
}

QString FilterItemWidget::getFilterDisplayName(const std::string& filterName)
{
    static const std::map<std::string, QString> filterNames = {
        {"grayscale", "Оттенки серого"},
        {"sepia", "Сепия"},
        {"invert", "Инверсия"},
        {"brightness", "Яркость"},
        {"contrast", "Контраст"},
        {"saturation", "Насыщенность"},
        {"flip_h", "Отразить по горизонтали"},
        {"flip_v", "Отразить по вертикали"},
        {"rotate90", "Поворот на 90°"},
        {"sharpen", "Повышение резкости"},
        {"edges", "Детекция краёв"},
        {"emboss", "Рельеф"},
        {"outline", "Контур"},
        {"blur", "Размытие по Гауссу"},
        {"box_blur", "Размытие по прямоугольнику"},
        {"motion_blur", "Размытие движения"},
        {"median", "Медианный фильтр"},
        {"noise", "Шум"},
        {"posterize", "Постеризация"},
        {"threshold", "Пороговая бинаризация"},
        {"vignette", "Виньетирование"}
    };

    const auto it = filterNames.find(filterName);
    if (it != filterNames.end())
    {
        return it->second;
    }
    return QString::fromStdString(filterName);
}

void FilterItemWidget::updateParameters(const std::map<std::string, QVariant>& parameters)
{
    updatingParameters_ = true;
    for (const auto& pair : parameters)
    {
        const auto widgetIt = parameterWidgets_.find(pair.first);
        if (widgetIt != parameterWidgets_.end() && widgetIt->second != nullptr)
        {
            if (auto* spinBox = qobject_cast<QDoubleSpinBox*>(widgetIt->second))
            {
                spinBox->setValue(pair.second.toDouble());
            }
            else if (auto* spinBox = qobject_cast<QSpinBox*>(widgetIt->second))
            {
                spinBox->setValue(pair.second.toInt());
            }
            else if (auto* comboBox = qobject_cast<QComboBox*>(widgetIt->second))
            {
                const int index = comboBox->findData(pair.second.toString());
                if (index >= 0)
                {
                    comboBox->setCurrentIndex(index);
                }
            }
            else if (auto* checkBox = qobject_cast<QCheckBox*>(widgetIt->second))
            {
                checkBox->setChecked(pair.second.toBool());
            }
        }
    }
    updatingParameters_ = false;
}

void FilterItemWidget::updateFilterName(const std::string& filterName)
{
    filterName_ = filterName;
    if (filterNameLabel_ != nullptr)
    {
        filterNameLabel_->setText(getFilterDisplayName(filterName_));
    }
    // Пересоздаем виджеты параметров для нового типа фильтра
    createParameterWidgets(filterName_, {});
}

void FilterItemWidget::updateButtonsState(bool canRemove, bool canMoveUp, bool canMoveDown)
{
    if (removeButton_ != nullptr)
    {
        removeButton_->setEnabled(canRemove);
    }
    if (moveUpButton_ != nullptr)
    {
        moveUpButton_->setEnabled(canMoveUp);
    }
    if (moveDownButton_ != nullptr)
    {
        moveDownButton_->setEnabled(canMoveDown);
    }
}

void FilterItemWidget::onParameterChanged()
{
    if (updatingParameters_)
    {
        return;
    }

    // Отправляем сигналы об изменении всех параметров
    for (const auto& pair : parameterWidgets_)
    {
        if (pair.second != nullptr)
        {
            QVariant value;
            if (auto* spinBox = qobject_cast<QDoubleSpinBox*>(pair.second))
            {
                value = spinBox->value();
            }
            else if (auto* spinBox = qobject_cast<QSpinBox*>(pair.second))
            {
                value = spinBox->value();
            }
            else if (auto* comboBox = qobject_cast<QComboBox*>(pair.second))
            {
                value = comboBox->currentData().toString();
            }
            else if (auto* checkBox = qobject_cast<QCheckBox*>(pair.second))
            {
                value = checkBox->isChecked();
            }

            if (value.isValid())
            {
                emit parameterChanged(filterIndex_, pair.first, value);
            }
        }
    }
}

void FilterItemWidget::onRemoveClicked()
{
    emit removeRequested(filterIndex_);
}

void FilterItemWidget::onMoveUpClicked()
{
    emit moveUpRequested(filterIndex_);
}

void FilterItemWidget::onMoveDownClicked()
{
    emit moveDownRequested(filterIndex_);
}

void FilterItemWidget::onChangeFilterTypeClicked()
{
    emit filterTypeChangeRequested(filterIndex_);
}

