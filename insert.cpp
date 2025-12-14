#include "insert.h"

// Проверяем, заблокирована ли таблица, читая файл состояния блокировки
bool isLocked(const string& tableName, const string& schemeName) {
    int fileCount = 0;
    ifstream file(constructFilePath(schemeName, tableName, "_lock.txt", fileCount));
    if (!file.is_open()) {
        cerr << "Ошибка: файл блокировки не найден.\n";
        return false;
    }

    string current;
    file >> current;
    file.close();

    return current == "locked";
}

// Переключаем состояние блокировки таблицы между "locked" и "unlocked"
void toggleLock(const string& tableName, const string& schemeName) {
    int fileCount = 0;
    ifstream fileIn(constructFilePath(schemeName, tableName, "_lock.txt", fileCount));
    if (!fileIn.is_open()) {
        cerr << "Ошибка: файл блокировки не найден.\n";
        return;
    }

    string current;
    fileIn >> current;
    fileIn.close();

    ofstream fileOut(constructFilePath(schemeName, tableName, "_lock.txt", fileCount));
    fileOut << (current == "locked" ? "unlocked" : "locked");
    fileOut.close();
}

// Проверяем, существует ли таблица с заданным именем в связанном списке
bool isTableExist(const string& tableName, tNode* tableHead) {
    for (tNode* current = tableHead; current; current = current->next) {
        if (current->table == tableName) {
            return true;
        }
    }
    return false;
}

// Получение индекса колонки в таблице (по порядку в связном списке)
int getColumnIndex(const string& tableName, const string& columnName, tNode* tableHead) {
    tNode* table = tableHead;
    while (table && table->table != tableName) {
        table = table->next;
    }
    if (!table) return -1;

    Node* col = table->column;
    int idx = 0;
    while (col) {
        if (col->column == columnName) {
            return idx;
        }
        col = col->next;
        idx++;
    }
    return -1;
}

// Поиск таблицы по имени
tNode* findTable(const string& tableName, tNode* tableHead) {
    for (tNode* current = tableHead; current; current = current->next) {
        if (current->table == tableName) {
            return current;
        }
    }
    return nullptr;
}

// Добавление строки в структуру данных таблицы
void appendRow(tNode* table, RowValue* values) {
    if (!table) return;
    RowNode* newRow = new RowNode{values, nullptr};
    if (!table->rows) {
        table->rows = newRow;
        return;
    }
    RowNode* cur = table->rows;
    while (cur->next) {
        cur = cur->next;
    }
    cur->next = newRow;
}

// Получение значения из строки по индексу
string getValueAt(RowNode* row, int idx) {
    RowValue* valNode = row ? row->values : nullptr;
    while (valNode && idx > 0) {
        valNode = valNode->next;
        --idx;
    }
    return valNode ? valNode->val : "";
}

// Копируем названия колонок из одного CSV-файла в другой
void copyColumnsName(const string& fileFrom, const string& fileTo) {
    ifstream fileF(fileFrom);
    if (!fileF.is_open()) {
        cerr << "Ошибка: не удалось открыть файл для копирования колонок.\n";
        return;
    }

    string columns;
    getline(fileF, columns); // Чтение первой строки - заголовков
    fileF.close();

    ofstream fileT(fileTo);
    if (!fileT.is_open()) {
        cerr << "Не удалось открыть целевой файл: " << fileTo << "\n";
        return;
    }

    fileT << columns << endl; // Запись заголовков в новый файл
    fileT.close();
}

// Обрабатываем SQL-команду INSERT INTO для вставки данных в таблицу
void insert(const string& command, tableJson& tjs) {
    istringstream iss(command);
    string word;

    iss >> word; // "INSERT"
    iss >> word; // "INTO"
    if (word != "INTO") {
        cerr << "Некорректная команда: отсутствует 'INTO'.\n";
        return;
    }
    
    string tableName;
    iss >> tableName; // Чтение имени таблицы
    if (!isTableExist(tableName, tjs.tablehead)) {
        cerr << "Ошибка: таблицы не существует.\n";
        return;
    }
    
    iss >> word; // "VALUES"
    if (word != "VALUES") {
        cerr << "Некорректная команда: отсутствует 'VALUES'.\n";
        return;
    }
    
    // Извлекаем данных для вставки из команды
    string values;
    getline(iss, values, '('); // Пропуск до открывающей скобки
    getline(iss, values, ')'); // Извлечение содержимого между скобками

    if (isLocked(tableName, tjs.schemeName)) {
        cerr << "Таблица заблокирована: " << tableName << "\n";
        return;
    }

    toggleLock(tableName, tjs.schemeName); // Блокировка таблицы перед изменением

    // Читаем текущее значение первичного ключа
    int fileCount1 = 0;
    string pkFile = constructFilePath(tjs.schemeName, tableName, "_pk_sequence.txt", fileCount1);
    
    ifstream pkIn(pkFile);
    int currentPk = 0;
    if (pkIn.is_open()) {
        pkIn >> currentPk; // Чтение текущего значения первичного ключа
        pkIn.close();
    }

    // Обновление значения первичного ключа
    ofstream pkOut(pkFile);
    if (!pkOut.is_open()) {
        cerr << "Не удалось открыть файл первичных ключей: " << pkFile << "\n";
        toggleLock(tableName, tjs.schemeName);
        return;
    }
    pkOut << ++currentPk; // Инкремент и запись нового значения
    pkOut.close();

    // Поиск подходящего CSV-файла для записи данных
    int csvNum = 1;
    string csvFile;
    bool needHeaderCopy = false;
    while (true) {
        csvFile = constructFilePath(tjs.schemeName, tableName, ".csv", csvNum);
        ifstream csvCheck(csvFile);

        if (!csvCheck.is_open()) {
            needHeaderCopy = true; // файл отсутствует — создадим и скопируем заголовок
            break;
        }

        try {
            rapidcsv::Document doc(csvFile);
            if (doc.GetRowCount() < tjs.tableSize) {
                break; // Нашли файл с доступным местом
            }
        } catch (const std::exception& e) {
            cerr << "Ошибка при работе с файлом " << csvFile << ": " << e.what() << "\n";
            break; // Останавливаем обработку при ошибке
        }

        csvNum++;
    }

    if (needHeaderCopy) {
        copyColumnsName(constructFilePath(tjs.schemeName, tableName, ".csv", 1), csvFile);
    }

    // Парсинг данных для записи
    RowValue* rowHead = new RowValue{to_string(currentPk), nullptr};
    RowValue* rowTail = rowHead;
    ofstream csv(csvFile, ios::app);
    if (!csv.is_open()) {
        cerr << "Ошибка: не удалось открыть CSV файл для записи.\n";
        toggleLock(tableName, tjs.schemeName);
        return;
    }
    // Форматирование записи: первичный ключ и значения
    csv << currentPk << ",";
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] == '\'') {
            ++i; // Пропуск символа начала значения
            string val;
            while (i < values.size() && values[i] != '\'') {
                csv << values[i]; // Запись значения
                val += values[i];
                ++i;
            }
            RowValue* newVal = new RowValue{val, nullptr};
            rowTail->next = newVal;
            rowTail = newVal;
            // Добавляем либо разделитель (если есть ещё значения), либо конец строки
            if (i + 1 < values.size() && values[i + 1] == ',') {
                csv << ",";
            } else {
                csv << "\n";
            }
        }
    }
    csv.close();
    appendRow(findTable(tableName, tjs.tablehead), rowHead); // Добавление строки в память
    toggleLock(tableName, tjs.schemeName); // Разблокировка таблицы после изменения

}
