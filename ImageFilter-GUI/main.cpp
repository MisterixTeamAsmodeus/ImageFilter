/**
 * @file main.cpp
 * @brief Точка входа для Qt5 GUI-приложения ImageFilter.
 */

#include <QGuiApplication>

/**
 * @brief Точка входа в GUI-приложение.
 *
 * На данном этапе приложение инициализирует Qt и завершает работу.
 * Загрузка QML-интерфейса и интеграция с ViewModel будут добавлены
 * на следующих этапах.
 *
 * @param argc Количество аргументов командной строки.
 * @param argv Массив аргументов командной строки.
 *
 * @return Код возврата приложения.
 */
int main(int argc, char* argv[])
{
    QGuiApplication application(argc, argv);
    return 0;
}


