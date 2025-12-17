#include "ui_FilterChainPanel.h"
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <gui/FilterChainPanel.h>
#include <gui/FilterItemWidget.h>
#include <gui/FilterSelectionDialog.h>
#include <model/FilterChainModel.h>

FilterChainPanel::FilterChainPanel(FilterChainModel* filterChainModel, QWidget* parent)
    : QWidget(parent), filterChainModel_(filterChainModel), ui_(new Ui::FilterChainPanel()) {
    if (filterChainModel_ == nullptr) {
        qWarning() << "FilterChainPanel: filterChainModel равен nullptr";
        return;
    }

    ui_->setupUi(this);

    // Получаем указатель на layout из UI
    qobject_cast<QVBoxLayout*>(ui_->filterListWidget->layout())
        ->addStretch(); // Добавляем растягивающий элемент в конец

    // Подключаем кнопку добавления фильтра
    connect(ui_->addFilterButton, &QPushButton::clicked, this, &FilterChainPanel::onAddFilter);

    // Подключаемся к сигналам модели
    connect(filterChainModel_, &FilterChainModel::chainChanged, this, &FilterChainPanel::onChainChanged);
    connect(filterChainModel_, &FilterChainModel::filterParameterChanged, this, &FilterChainPanel::onChainChanged);

    // Обновляем список фильтров
    updateFilterList();
}

FilterChainPanel::~FilterChainPanel() {
    delete ui_;
}

void FilterChainPanel::updateFilterList() {
    if (filterChainModel_ == nullptr || ui_->filterListWidget == nullptr) {
        return;
    }

    // Удаляем все существующие виджеты фильтров
    for (auto* widget : filterItemWidgets_) {
        if (widget != nullptr) {
            widget->deleteLater();
        }
    }
    filterItemWidgets_.clear();

    // Удаляем все элементы из layout (кроме растягивающего)
    QLayoutItem* item;
    while ((item = ui_->filterListWidget->layout()->takeAt(0)) != nullptr) {
        if (item->widget() != nullptr) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    // Добавляем растягивающий элемент обратно
    ui_->filterListLayout->addStretch(); // Добавляем растягивающий элемент в конец

    // Добавляем виджеты для каждого фильтра
    const size_t filterCount = filterChainModel_->getFilterCount();
    for (size_t i = 0; i < filterCount; ++i) {
        const auto& filter = filterChainModel_->getFilter(i);
        const bool canRemove = filterChainModel_->canRemoveFilter();
        const bool canMoveUp = (i > 0);
        const bool canMoveDown = (i < filterCount - 1);

        auto* filterWidget = new FilterItemWidget(i, filter.filterName, filter.parameters, canRemove, canMoveUp,
                                                  canMoveDown, ui_->filterListWidget);

        // Подключаем сигналы
        connect(filterWidget, &FilterItemWidget::parameterChanged, this,
                [this](size_t index, const std::string& paramName, const QVariant& value) {
                    if (filterChainModel_ != nullptr) {
                        filterChainModel_->updateFilterParameter(index, paramName, value);
                    }
                });

        connect(filterWidget, &FilterItemWidget::removeRequested, this, [this](size_t index) {
            if (filterChainModel_ != nullptr) {
                // Валидация: проверяем, можно ли удалить фильтр
                if (!filterChainModel_->canRemoveFilter()) {
                    QMessageBox::warning(this, "Ошибка",
                                         "Невозможно удалить фильтр: в цепочке должен остаться хотя бы один фильтр.");
                    return;
                }

                const bool success = filterChainModel_->removeFilter(index);
                if (!success) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось удалить фильтр. Возможно, индекс невалиден.");
                }
            }
        });

        connect(filterWidget, &FilterItemWidget::moveUpRequested, this, [this](size_t index) {
            if (filterChainModel_ != nullptr) {
                const bool success = filterChainModel_->moveFilterUp(index);
                if (!success) {
                    QMessageBox::warning(
                        this, "Ошибка",
                        "Не удалось переместить фильтр вверх. Возможно, фильтр уже находится в начале цепочки.");
                }
            }
        });

        connect(filterWidget, &FilterItemWidget::moveDownRequested, this, [this](size_t index) {
            if (filterChainModel_ != nullptr) {
                const bool success = filterChainModel_->moveFilterDown(index);
                if (!success) {
                    QMessageBox::warning(
                        this, "Ошибка",
                        "Не удалось переместить фильтр вниз. Возможно, фильтр уже находится в конце цепочки.");
                }
            }
        });

        connect(filterWidget, &FilterItemWidget::filterTypeChangeRequested, this, [this](size_t index) {
            if (filterChainModel_ != nullptr) {
                const QString selectedFilter = showFilterSelectionDialog();
                if (!selectedFilter.isEmpty()) {
                    const bool success = filterChainModel_->changeFilterType(index, selectedFilter.toStdString());
                    if (!success) {
                        QMessageBox::warning(this, "Ошибка",
                                             "Не удалось изменить тип фильтра. Возможно, имя фильтра невалидно.");
                    }
                }
            }
        });

        filterItemWidgets_.push_back(filterWidget);
        ui_->filterListLayout->insertWidget(static_cast<int>(i), filterWidget);
    }
}

void FilterChainPanel::onAddFilter() {
    const QString selectedFilter = showFilterSelectionDialog();
    if (!selectedFilter.isEmpty() && filterChainModel_ != nullptr) {
        const bool success = filterChainModel_->addFilter(selectedFilter.toStdString());
        if (!success) {
            QMessageBox::warning(this, "Ошибка", "Не удалось добавить фильтр. Возможно, имя фильтра невалидно.");
        }
    }
}

QString FilterChainPanel::showFilterSelectionDialog() {
    FilterSelectionDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getSelectedFilter();
    }
    return QString();
}

void FilterChainPanel::onChainChanged() {
    updateFilterList();
}
