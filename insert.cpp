#include "insert.h"

// Проверяет, заблокирована ли таблица, читая файл состояния блокировки
bool isLocked(const string& tableName, const string& schemeName) {
    string fileName = "/home/vlad/Documents/VC Code/SecondSemestr/TEST/" + schemeName + "/" + tableName + "/" + tableName + "_lock.txt";
    ifstream file(fileName);
    if (!file.is_open()) {
        cerr << "Не удалось открыть файл блокировки: " << fileName << "\n";
        return false;
    }

    string current;
    file >> current;
    file.close();

    return current == "locked";
}

// Переключает состояние блокировки таблицы между "locked" и "unlocked"
void toggleLock(const string& tableName, const string& schemeName) {
    string fileName = "/home/vlad/Documents/VC Code/SecondSemestr/TEST/" + schemeName + "/" + tableName + "/" + tableName + "_lock.txt";

    // Чтение текущего состояния блокировки
    ifstream fileIn(fileName);
    if (!fileIn.is_open()) {
        cerr << "Не удалось открыть файл блокировки для чтения: " << fileName << "\n";
        return;
    }

    string current;
    fileIn >> current;
    fileIn.close();

    // Переключение состояния
    ofstream fileOut(fileName);
    if (!fileOut.is_open()) {
        cerr << "Не удалось открыть файл блокировки для записи: " << fileName << "\n";
        return;
    }

    fileOut << (current == "locked" ? "unlocked" : "locked");
    fileOut.close();
}

// Проверяет, существует ли таблица с заданным именем в связанном списке
bool isTableExist(const string& tableName, tNode* tableHead) {
    for (tNode* current = tableHead; current; current = current->next) {
        if (current->table == tableName) {
            return true;
        }
    }
    return false;
}

// Копирует названия колонок из одного CSV-файла в другой
void copyColumnsName(const string& fileFrom, const string& fileTo) {
    ifstream fileF(fileFrom);
    if (!fileF.is_open()) {
        cerr << "Не удалось открыть исходный файл: " << fileFrom << "\n";
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

// Обрабатывает SQL-команду INSERT INTO для вставки данных в таблицу
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
        cerr << "Таблица не существует: " << tableName << "\n";
        return;
    }

    iss >> word; // "VALUES"
    if (word != "VALUES") {
        cerr << "Некорректная команда: отсутствует 'VALUES'.\n";
        return;
    }

    // Извлечение данных для вставки из команды
    string values;
    getline(iss, values, '('); // Пропуск до открывающей скобки
    getline(iss, values, ')'); // Извлечение содержимого между скобками

    if (isLocked(tableName, tjs.schemeName)) {
        cerr << "Таблица заблокирована: " << tableName << "\n";
        return;
    }

    toggleLock(tableName, tjs.schemeName); // Блокировка таблицы перед изменением

    // Чтение текущего значения первичного ключа
    string pkFile = "/home/vlad/Documents/VC Code/SecondSemestr/TEST/" + tjs.schemeName + "/" + tableName + "/" + tableName + "_pk_sequence.txt";

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
    while (true) {
        csvFile = "/home/vlad/Documents/VC Code/SecondSemestr/TEST/" + tjs.schemeName + "/" + tableName + "/" + to_string(csvNum) + ".csv";
        ifstream csvCheck(csvFile);

        // Если файл не существует или он пустой
        if (!csvCheck.is_open()) {
            break; // Новый файл будет создан
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


    // Копирование структуры заголовков, если файл новый
    string csvOne = "/home/vlad/Documents/VC Code/SecondSemestr/TEST/" + tjs.schemeName + "/" + tableName + "/1.csv";
    ifstream csvTemplate(csvOne);
    if (csvTemplate.is_open() && rapidcsv::Document(csvFile).GetRowCount() == 0) {
        copyColumnsName(csvOne, csvFile);
    }

    // Запись данных в CSV-файл
    ofstream csv(csvFile, ios::app);
    if (!csv.is_open()) {
        cerr << "Не удалось открыть CSV-файл: " << csvFile << "\n";
        toggleLock(tableName, tjs.schemeName);
        return;
    }

    // Форматирование записи: первичный ключ и значения
    csv << currentPk << ",";
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] == '\'') {
            ++i; // Пропуск символа начала значения
            while (values[i] != '\'') {
                csv << values[i++]; // Запись значения
            }
            // Добавляем либо разделитель (если есть ещё значения), либо конец строки
            if (i + 1 < values.size() && values[i + 1] == ',') {
                csv << ",";
            } else {
                csv << "\n";
            }
        }
    }

    csv.close();
    toggleLock(tableName, tjs.schemeName); // Разблокировка таблицы после изменения

}
