#include <gui/MainWindow.h>
#include <utils/LoggerConfigurator.h>
#include <ImageProcessor.h>

#include <QApplication>
#include <QFile>
#include <QMetaType>
#include <QPalette>
#include <QStyleFactory>

#include <memory>

// Объявление мета-типа для указателя на ImageProcessor
Q_DECLARE_METATYPE(ImageProcessor*)
// Объявление мета-типа для shared_ptr на ImageProcessor
Q_DECLARE_METATYPE(std::shared_ptr<ImageProcessor>)

namespace {

/**
 * @brief Настраивает светлую палитру приложения.
 *
 * Цвета согласованы с light_theme.qss и задают базовый вид всех окон,
 * кнопок и текстов до применения стилей.
 *
 * @param app Ссылка на приложение Qt.
 */
void configureLightPalette(QApplication& app)
{
    QPalette palette = app.palette();

    palette.setColor(QPalette::Window, QColor(0xF8, 0xF9, 0xFA));        // фон панелей
    palette.setColor(QPalette::Base, QColor(0xFF, 0xFF, 0xFF));          // фон центральных областей/полей ввода
    palette.setColor(QPalette::AlternateBase, QColor(0xF1, 0xF3, 0xF5)); // альтернативный фон

    palette.setColor(QPalette::Text, QColor(0x21, 0x25, 0x29));          // основной текст
    palette.setColor(QPalette::WindowText, QColor(0x21, 0x25, 0x29));
    palette.setColor(QPalette::ButtonText, QColor(0x21, 0x25, 0x29));

    palette.setColor(QPalette::Button, QColor(0xF8, 0xF9, 0xFA));
    palette.setColor(QPalette::Highlight, QColor(0x6C, 0x5C, 0xE7));     // акцент
    palette.setColor(QPalette::HighlightedText, QColor(0xFF, 0xFF, 0xFF));

    palette.setColor(QPalette::Mid, QColor(0xCE, 0xD4, 0xDA));           // границы
    palette.setColor(QPalette::Dark, QColor(0xE0, 0xE0, 0xE0));          // разделители

    app.setPalette(palette);
}

/**
 * @brief Загружает и применяет стиль из ресурсов light_theme.qss.
 *
 * В случае ошибки загрузки стиль приложения не изменяется.
 *
 * @param app Ссылка на приложение Qt.
 */
void applyStyleSheet(QApplication& app)
{
    QFile styleFile(QStringLiteral(":/style/style/light_theme.qss"));
    if (!styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    const QByteArray data = styleFile.readAll();
    styleFile.close();

    app.setStyleSheet(QString::fromUtf8(data));
}

} // namespace

/**
 * @brief Точка входа в приложение
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код возврата приложения
 */
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Регистрация пользовательских типов для мета-системы Qt
    qRegisterMetaType<ImageProcessor*>();
    qRegisterMetaType<std::shared_ptr<ImageProcessor>>();

    // Инициализация системы логирования
    // По умолчанию включаем логирование с уровнем INFO
    LoggerConfigurator::configure(false, "INFO");

    // Используем стиль Fusion как более современную базу
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));

    // Настраиваем глобальную светлую палитру и подключаем тему стилей
    configureLightPalette(app);
    applyStyleSheet(app);

    // Настройка информации о приложении
    app.setApplicationName(QStringLiteral("ImageFilter"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));
    app.setOrganizationName(QStringLiteral("ImageFilter"));

    // Создание и отображение главного окна
    MainWindow window;
    window.show();

    return app.exec();
}


