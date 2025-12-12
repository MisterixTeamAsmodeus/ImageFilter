#include <cli/CommandParser.h>
#include <cli/CommandExecutor.h>
#include <utils/FilterFactory.h>

/**
 * @brief Точка входа приложения ImageFilter
 * 
 * Регистрирует фильтры, парсит аргументы командной строки и выполняет команды.
 */
int main(int argc, char* argv[])
{
    // Регистрируем все фильтры в фабрике
    FilterFactory::getInstance().registerAll();
    
    // Парсим аргументы командной строки
    CommandParser parser;
    CommandOptions options;
    
    int parse_result = parser.parse(argc, argv, options);
    if (parse_result != 0)
    {
        return parse_result;
    }
    
    // Выполняем команду
    CommandExecutor executor;
    return executor.execute(options, parser.getApp());
}
