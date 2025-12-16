#include <gui/FilterSelectionDialog.h>
#include <gui/FilterProxyModel.h>
#include <gui/FilterListModel.h>
#include "ui_FilterSelectionDialog.h"
#include <QTreeView>
#include <QStandardItem>
#include <QModelIndex>

FilterSelectionDialog::FilterSelectionDialog(QWidget* parent)
    : QDialog(parent)
    , ui_(new Ui::FilterSelectionDialog())
    , model_(new FilterListModel(this))
    , proxyModel_(new FilterProxyModel(this))
{
    ui_->setupUi(this);
    
    // Настраиваем proxy модель
    proxyModel_->setSourceModel(model_);
    
    // Устанавливаем proxy модель в представление
    ui_->treeView->setModel(proxyModel_);
    
    // Отключаем редактирование элементов
    ui_->treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Подключаем сигнал поиска
    connect(ui_->searchLineEdit, &QLineEdit::textChanged,
            this, &FilterSelectionDialog::onSearchTextChanged);

    // Выбираем первый фильтр по умолчанию
    QModelIndex firstFilterIndex;
    for (int i = 0; i < proxyModel_->rowCount(); ++i)
    {
        const QModelIndex categoryIndex = proxyModel_->index(i, 0);
        if (categoryIndex.isValid() && proxyModel_->rowCount(categoryIndex) > 0)
        {
            firstFilterIndex = proxyModel_->index(0, 0, categoryIndex);
            break;
        }
    }
    
    if (firstFilterIndex.isValid())
    {
        ui_->treeView->setCurrentIndex(firstFilterIndex);
        ui_->treeView->expandAll();
    }
}

FilterSelectionDialog::~FilterSelectionDialog()
{
    delete ui_;
}

QString FilterSelectionDialog::getSelectedFilter() const
{
    const QModelIndex currentIndex = ui_->treeView->currentIndex();
    if (currentIndex.isValid())
    {
        // Преобразуем индекс из proxy модели в индекс исходной модели
        const QModelIndex sourceIndex = proxyModel_->mapToSource(currentIndex);
        QStandardItem* item = model_->itemFromIndex(sourceIndex);
        if (item != nullptr && item->parent() != nullptr)
        {
            return item->data(Qt::UserRole).toString();
        }
    }
    return QString();
}

void FilterSelectionDialog::onSearchTextChanged(const QString& text)
{
    proxyModel_->setFilterText(text);
}


