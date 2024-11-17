#include "pars.h"

// Удаление директории, если она существует
void removeDirectory(const fs::path& directoryPath) {
    if (fs::exists(directoryPath)) {
        fs::remove_all(directoryPath); // Полное удаление директории
    }
}

// Создание директорий и файлов для таблиц
void createDirectoriesAndFiles(const fs::path& schemePath, const json& structure, tableJson& tjs) {
    tNode* tableHead = nullptr; // Голова списка таблиц
    tNode* tableTail = nullptr; // Хвост списка таблиц

    for (const auto& table : structure.items()) { // Перебор таблиц из json
        fs::path tablePath = schemePath / table.key(); // Путь к директории таблицы
        if (!fs::create_directory(tablePath)) {
            cerr << "Не удалось создать директорию: " << tablePath << endl;
            return;
        }
        cout << "Создана директория: " << tablePath << endl;

        // Создаем узел таблицы и файл блокировки
        tNode* newTable = new tNode{table.key(), nullptr, nullptr}; 
        fs::current_path(tablePath);
        ofstream lockFile(table.key() + "_lock.txt"); // Файл блокировки
        if (!lockFile.is_open()) {
            cerr << "Не удалось открыть файл блокировки.\n";
        }
        lockFile << "unlocked"; // По умолчанию разблокирован
        lockFile.close();

        // Добавляем таблицу в список таблиц
        if (!tableHead) {
            tableHead = newTable;
            tableTail = newTable;
        } else {
            tableTail->next = newTable;
            tableTail = newTable;
        }

        // Создаем колонку с первичным ключом и записываем названия колонок в CSV
        string keyColumn = table.key() + "_pk";
        Node* column_pk = new Node{keyColumn, nullptr}; 
        newTable->column = column_pk; 

        fs::path csvFilePath = tablePath / "1.csv"; // Путь к CSV
        ofstream csvFile(csvFilePath);
        if (!csvFile.is_open()) {
            cerr << "Не удалось создать CSV файл: " << csvFilePath << endl;
            return;
        }
        csvFile << keyColumn << ",";

        const auto& columns = table.value(); // Список колонок
        for (size_t i = 0; i < columns.size(); ++i) { 
            csvFile << columns[i].get<string>(); // Запись колонок
            Node* newColumn = new Node{columns[i], nullptr}; // Создание колонки

            if (!newTable->column) {
                newTable->column = newColumn;
            } else {
                Node* lastColumn = newTable->column;
                while (lastColumn->next) {
                    lastColumn = lastColumn->next; // Поиск последней колонки
                }
                lastColumn->next = newColumn; // Добавление новой колонки в конец
            }
            if (i < columns.size() - 1) { 
                csvFile << ",";
            }
        }
        csvFile << endl;
        csvFile.close();
        cout << "Создан файл: " << csvFilePath << endl;

        // Создание файла для хранения уникального первичного ключа
        ofstream pkFile(table.key() + "_pk_sequence.txt");
        if (!pkFile.is_open()) {
            cerr << "Не удалось открыть файл.\n";
        }
        pkFile << "0";
        pkFile.close();
    }
    tjs.tablehead = tableHead;
}

// Парсинг схемы базы данных
void parsing(tableJson& tjs) {
    string filename = "schema.json"; // Имя JSON файла схемы
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл: " << filename << endl;
        return;
    }

    string json_content, line;
    while(getline(file, line)) { // Считывание файла
        json_content += line;
    }
    file.close();

    json jparsed = json::parse(json_content); // Парсинг JSON

    // Настройка пути и создание директории схемы
    tjs.schemeName = jparsed["name"];
    fs::path schemePath = fs::current_path() / tjs.schemeName;
    removeDirectory(schemePath); // Удаление старой директории
    if (!fs::create_directory(schemePath)) {
        cerr << "Не удалось создать директорию: " << schemePath << endl;
        return;
    }
    cout << "Создана директория: " << schemePath << endl;

    // Создание структуры базы данных
    if (jparsed.contains("structure")) {
        createDirectoriesAndFiles(schemePath, jparsed["structure"], tjs);
    }
    tjs.tableSize = jparsed["tuples_limit"]; // Ограничение строк
}
