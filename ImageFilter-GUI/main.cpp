#include <gui/MainWindow.h>
#include <utils/LoggerConfigurator.h>
#include <ImageProcessor.h>
#include <QApplication>
#include <QStyleFactory>
#include <QMetaType>
#include <memory>

// Объявление мета-типа для указателя на ImageProcessor
Q_DECLARE_METATYPE(ImageProcessor*)
// Объявление мета-типа для shared_ptr на ImageProcessor
Q_DECLARE_METATYPE(std::shared_ptr<ImageProcessor>)

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

    // Настройка информации о приложении
    app.setApplicationName("ImageFilter");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("ImageFilter");

    // Создание и отображение главного окна
    MainWindow window;
    window.show();

    return app.exec();
}

