#include "delete.h"

// Проверка наличия колонки в таблице
bool isColumnExist(const string& tableName, const string& columnName, tNode* tableHead) {
    tNode* currentTable = tableHead; // Начинаем с головы списка таблиц
    while (currentTable) {
        if (currentTable->table == tableName) { // Ищем нужную таблицу
            Node* currentColumn = currentTable->column; // Переходим к колонкам таблицы
            while (currentColumn) {
                if (currentColumn->column == columnName) { // Нашли колонку
                    return true;
                }
                currentColumn = currentColumn->next;
            }
            return false; // Колонка не найдена
        }
        currentTable = currentTable->next;
    }
    return false;
}

// Удаление строк из таблицы по условию
void del(const string& command, tableJson& tjs) {
    istringstream iss(command); // Поток для чтения команды
    string word;
    iss >> word; // Пропускаем "DELETE"
    iss >> word; // Пропускаем "FROM"
    if (word != "FROM") {
        cerr << "Некорректная команда.\n";
        return;
    }

    iss >> word; // Чтение имени таблицы
    if (!tableExists(word, tjs.tablehead)) {
        cerr << "Такой таблицы нет.\n";
        return;
    }
    string tableName = word;

    // Чтение условия WHERE
    string secondCmd;
    getline(cin, secondCmd);
    istringstream iss2(secondCmd); // Поток для чтения условий команды
    iss2 >> word; // Пропускаем "WHERE"
    if (word != "WHERE") {
        cerr << "Некорректная команда.\n";
        return;
    }

    iss2 >> word; // Чтение имени таблицы и колонки (table.column)
    string table, column;
    bool dotFound = false;
    for (char ch : word) {
        if (ch == '.') {
            dotFound = true;
            continue;
        }
        (dotFound ? column : table) += ch;
    }
    if (!dotFound || table != tableName) { // Проверка, что имя таблицы совпадает и есть точка
        cerr << "Некорректная команда.\n";
        return;
    }

    // Проверка, существует ли указанная колонка
    if (!isColumnExist(tableName, column, tjs.tablehead)) {
        cerr << "Такой колонки нет.\n";
        return;
    }

    // Проверка на равенство значения
    iss2 >> word;
    if (word != "=") {
        cerr << "Некорректная команда.\n";
        return;
    }

    // Чтение удаляемого значения
    string value;
    iss2 >> word;
    if (word.front() != '\'' || word.back() != '\'') { // Проверка на кавычки
        cerr << "Некорректная команда.\n";
        return;
    }
    value = word.substr(1, word.size() - 2); // Убираем кавычки

    // Проверка блокировки таблицы перед удалением
    if (isLocked(tableName, tjs.schemeName)) {
        cerr << "Таблица заблокирована.\n";
        return;
    }
    toggleLock(tableName, tjs.schemeName); // Блокируем таблицу

    // Подсчет файлов CSV
    int csvCount = 1;
    while (true) {
        string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + tableName + "/" + to_string(csvCount) + ".csv";
        ifstream file(filePath);
        if (!file.is_open()) break; // Если файл не открыт, его нет
        file.close();
        csvCount++;
    }

    // Удаление строк по значению из CSV файлов
    bool deleted = false;
    for (int iCsv = 1; iCsv < csvCount; iCsv++) {
        string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + tableName + "/" + to_string(iCsv) + ".csv";
        rapidcsv::Document doc(filePath);
        int columnIndex = doc.GetColumnIdx(column);
        size_t rowCount = doc.GetRowCount();

        for (size_t i = 0; i < rowCount; ++i) {
            if (doc.GetCell<string>(columnIndex, i) == value) { // Удаление строки
                doc.RemoveRow(i);
                deleted = true;
                --rowCount;
                --i;
            }
        }
        doc.Save(filePath);
    }

    if (!deleted) { // Если ничего не удалено, выводим сообщение
        cout << "Указанное значение не найдено.\n";
    }
    toggleLock(tableName, tjs.schemeName); // Разблокировка таблицы
}
