#include <cli/CommandExecutor.h>
#include <cli/ImageProcessingHelper.h>
#include <cli/FilterInfoDisplay.h>
#include <cli/ProgressDisplay.h>
#include <preset/PresetManager.h>
#include <utils/ErrorHandlerChain.h>
#include <utils/FilterResult.h>
#include <CLI/CLI.hpp>
#include <utils/Logger.h>
#include <utils/LoggerConfigurator.h>
#include <cli/FilterFactory.h>
#include "BatchProcessor.h"
#include <utils/BufferPool.h>
#include <utils/ThreadPool.h>
#include <memory>

CommandExecutor::CommandExecutor() {
}

int CommandExecutor::execute(const CommandOptions &options, CLI::App &app) {
    // Настраиваем логирование на основе разобранных опций
    LoggerConfigurator::configure(options.quiet, options.log_level_str);

    // Обработка специальных команд
    if (options.list_filters) {
        return executeListFilters(app);
    }

    if (!options.filter_info.empty()) {
        return executeFilterInfo(options.filter_info, app);
    }

    // Обработка пресетов
    std::string preset_directory = options.preset_dir.empty() ? "./presets" : options.preset_dir;
    std::string filter_name = options.filter_name;

    // Если указан пресет, загружаем его
    if (!options.preset_file.empty()) {
        if (!PresetManager::loadPreset(options.preset_file, preset_directory, filter_name)) {
            return 1;
        }
    }

    // Парсим цепочку фильтров (нужно для обоих режимов)
    auto filters = ImageProcessingHelper::parseFilterChain(filter_name);
    if (filters.empty() && !options.batch_mode && options.save_preset.empty()) {
        Logger::error("Ошибка: не указаны фильтры для применения");
        return 1;
    }

    // Сохранение пресета
    if (!options.save_preset.empty()) {
        int result = executeSavePreset(options);
        if (result != 0 || (options.input_file.empty() && options.output_file.empty())) {
            return result;
        }
        // Продолжаем с обработкой изображения после сохранения пресета
    }

    // Пакетный режим обработки
    if (options.batch_mode) {
        return executeBatchProcessing(options, app);
    }

    // Обычный режим обработки одного файла
    return executeSingleImage(options, app);
}

int CommandExecutor::executeListFilters(CLI::App &app) {
    FilterInfoDisplay::printFilterList(app);
    return 0;
}

int CommandExecutor::executeFilterInfo(const std::string &filter_name, CLI::App &app) {
    FilterInfoDisplay::printFilterInfo(filter_name, app);
    return 0;
}

int CommandExecutor::executeSavePreset(const CommandOptions &options) {
    std::string preset_directory = options.preset_dir.empty() ? "./presets" : options.preset_dir;
    if (PresetManager::savePreset(options.filter_name, options.save_preset, preset_directory)) {
        return 0;
    } else {
        return 1;
    }
}

int CommandExecutor::executeSingleImage(const CommandOptions &options, CLI::App &app) {
    if (options.input_file.empty() || options.filter_name.empty() || options.output_file.empty()) {
        Logger::error("Ошибка: необходимо указать input, filter и output");
        Logger::error("Используйте --help для справки");
        Logger::error("Или --list-filters для списка доступных фильтров");
        return 1;
    }

    // Парсим цепочку фильтров
    auto filters = ImageProcessingHelper::parseFilterChain(options.filter_name);
    if (filters.empty()) {
        Logger::error("Ошибка: не указаны фильтры для применения");
        return 1;
    }

    Logger::info("Загрузка изображения: " + options.input_file);

    // Используем вспомогательную функцию для обработки изображения
    if (!ImageProcessingHelper::processSingleImage(
        options.input_file,
        options.output_file,
        filters,
        app,
        options.preserve_alpha,
        options.force_rgb,
        options.jpeg_quality)) {
        return 1;
    }

    Logger::info("Готово! Результат сохранен в " + options.output_file);

    return 0;
}

int CommandExecutor::executeBatchProcessing(const CommandOptions &options, CLI::App &app) {
    if (options.input_dir.empty() || options.output_dir.empty() || options.filter_name.empty()) {
        Logger::error("Ошибка: в пакетном режиме необходимо указать --input-dir, --output-dir и filter");
        Logger::error("Используйте --help для справки");
        return 1;
    }

    auto filters = ImageProcessingHelper::parseFilterChain(options.filter_name);
    if (filters.empty()) {
        Logger::error("Ошибка: не указаны фильтры для применения");
        return 1;
    }

    Logger::info("Пакетный режим обработки");
    Logger::info("Входная директория: " + options.input_dir);
    Logger::info("Выходная директория: " + options.output_dir);
    if (options.recursive) {
        Logger::info("Рекурсивный обход: включен");
    }
    if (!options.pattern.empty()) {
        Logger::info("Шаблон фильтрации: " + options.pattern);
    }

    // Создаем пул буферов для оптимизации использования памяти
    // Используем один пул для всей пакетной обработки
    BufferPool buffer_pool;
    auto &factory = FilterFactory::getInstance();
    factory.setBufferPool(&buffer_pool);

    // Создаем процессор пакетной обработки
    BatchProcessor processor(options.input_dir, options.output_dir, options.recursive, options.pattern);

    // Создаем цепочку обработчиков ошибок
    ErrorHandlerChain error_chain = ErrorHandlerChain::createDefault();

    // Функция обработки одного файла
    auto process_function = [&](const std::string &input_path, const std::string &output_path) -> FilterResult {
        const bool success = ImageProcessingHelper::processSingleImage(
            input_path, output_path, filters, app,
            options.preserve_alpha, options.force_rgb, options.jpeg_quality);
        if (success) {
            return FilterResult::success();
        } else {
            // Создаем FilterResult с ошибкой
            ErrorContext ctx = ErrorContext::withFilename(input_path);
            FilterResult result = FilterResult::failure(FilterError::FileOperationFailed,
                                                        "Ошибка обработки изображения", ctx);
            // Обрабатываем ошибку через цепочку
            error_chain.process(result);
            return result;
        }
    };

    // Callback для отображения прогресса
    ProgressCallback progress_callback = ProgressDisplay::displayProgress;

    // Определяем параметры параллельной обработки
    std::unique_ptr<ThreadPool> thread_pool;
    IThreadPool *pool = nullptr;
    int max_parallel = 0;

    // Проверяем, нужно ли использовать параллельную обработку
    // Для простоты используем параллельную обработку по умолчанию, если не указано иное
    // В будущем можно добавить опцию командной строки для управления этим
    const bool use_parallel = true; // Можно сделать настраиваемым через опции

    if (use_parallel) {
        // Создаем пул потоков (количество потоков = количество ядер процессора)
        thread_pool = std::make_unique<ThreadPool>(0); // 0 = автоматическое определение
        pool = thread_pool.get();
        max_parallel = 0; // 0 = использовать все потоки пула
        Logger::info("Параллельная обработка: включена (" +
                     std::to_string(pool->getThreadCount()) + " потоков)");
    }

    // Определяем файл состояния для возобновления
    std::string resume_state_file;
    if (!options.resume_state_file.empty()) {
        resume_state_file = options.resume_state_file;
        Logger::info("Возобновление обработки: включено (файл состояния: " + resume_state_file + ")");
    }

    // Обрабатываем все файлы
    BatchStatistics stats;
    if (!resume_state_file.empty()) {
        stats = processor.processAllWithResume(process_function, progress_callback,
                                               resume_state_file, pool, max_parallel);
    } else {
        stats = processor.processAll(process_function, progress_callback, pool, max_parallel);
    }

    // Выводим статистику
    Logger::info("Пакетная обработка завершена:");
    Logger::info("  Всего файлов: " + std::to_string(stats.total_files));
    Logger::info("  Успешно обработано: " + std::to_string(stats.processed_files));
    Logger::info("  Ошибок: " + std::to_string(stats.failed_files));
    Logger::info("  Пропущено: " + std::to_string(stats.skipped_files));

    return (stats.failed_files > 0) ? 1 : 0;
}
