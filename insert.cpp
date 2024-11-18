#include "insert.h"
using namespace std;

string constructFilePath(const string& schemeName, const string& tableName, const string& fileType) {
    return "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + schemeName + "/" + tableName + "/" + tableName + fileType;
}

bool isLocked(const string& tableName, const string& schemeName) {
    ifstream file(constructFilePath(schemeName, tableName, "_lock.txt"));
    if (!file.is_open()) {
        cerr << "Ошибка: файл блокировки не найден.\n";
        return false;
    }
    string lockStatus;
    file >> lockStatus;
    return lockStatus == "locked";
}

void toggleLock(const string& tableName, const string& schemeName) {
    ifstream fileIn(constructFilePath(schemeName, tableName, "_lock.txt"));
    if (!fileIn.is_open()) {
        cerr << "Ошибка: файл блокировки не найден.\n";
        return;
    }

    string lockStatus;
    fileIn >> lockStatus;
    fileIn.close();

    ofstream fileOut(constructFilePath(schemeName, tableName, "_lock.txt"));
    fileOut << (lockStatus == "locked" ? "unlocked" : "locked");
}

bool tableExists(const string& tableName, tNode* tableHead) {
    for (tNode* current = tableHead; current; current = current->next) {
        if (current->table == tableName) return true;
    }
    return false;
}

void copyColumnNames(const string& sourceFile, const string& destFile) {
    ifstream source(sourceFile);
    ofstream dest(destFile);
    if (!source.is_open() || !dest.is_open()) {
        cerr << "Ошибка: не удалось открыть файл для копирования колонок.\n";
        return;
    }
    string columns;
    source >> columns;
    dest << columns << endl;
}

int getCurrentPrimaryKey(const string& pkFilePath) {
    ifstream file(pkFilePath);
    if (!file.is_open()) {
        cerr << "Ошибка: файл последовательности PK не найден.\n";
        return -1;
    }
    int pk;
    file >> pk;
    return pk;
}

void updatePrimaryKey(const string& pkFilePath, int newPk) {
    ofstream file(pkFilePath);
    if (!file.is_open()) {
        cerr << "Ошибка: не удалось открыть файл для записи PK.\n";
        return;
    }
    file << newPk;
}

string extractValues(const string& valuesStr) {
    if (valuesStr.front() != '(' || valuesStr.back() != ')') {
        cerr << "Ошибка: некорректный формат значений.\n";
        return "";
    }
    return valuesStr.substr(1, valuesStr.size() - 2); // удаление скобок
}

void insertData(const string& command, tableJson& tjs) {
    istringstream iss(command);
    string keyword, tableName, values;
    
    iss >> keyword >> keyword; // пропускаем "INSERT INTO"
    if (keyword != "INTO") {
        cerr << "Ошибка: некорректная команда.\n";
        return;
    }

    iss >> tableName;
    if (!tableExists(tableName, tjs.tablehead)) {
        cerr << "Ошибка: таблица не существует.\n";
        return;
    }

    iss >> keyword; // пропускаем "VALUES"
    if (keyword != "VALUES") {
        cerr << "Ошибка: некорректная команда.\n";
        return;
    }

    getline(iss, values);
    values = extractValues(values);
    if (values.empty()) return;

    if (isLocked(tableName, tjs.schemeName)) {
        cerr << "Ошибка: таблица заблокирована.\n";
        return;
    }

    toggleLock(tableName, tjs.schemeName); // блокируем таблицу

    // Обработка первичного ключа
    string pkFile = constructFilePath(tjs.schemeName, tableName, "_pk_sequence.txt");
    int currentPk = getCurrentPrimaryKey(pkFile);
    if (currentPk == -1) return;
    updatePrimaryKey(pkFile, currentPk + 1);

    // Поиск или создание подходящего CSV файла
    int csvNum = 1;
    string csvFile;
    while (true) {
        csvFile = constructFilePath(tjs.schemeName, tableName, "/" + to_string(csvNum) + ".csv");
        ifstream fileIn(csvFile);

        if (!fileIn.is_open()) { // если файла нет, создаем его
            ofstream fileOut(csvFile);
            if (!fileOut.is_open()) {
                cerr << "Ошибка: не удалось создать CSV файл.\n";
                return;
            }
            fileOut.close();
        }

        rapidcsv::Document doc(csvFile);
        if (doc.GetRowCount() < tjs.tableSize) break; // если файл не переполнен, используем его

        ++csvNum; // иначе продолжаем искать свободный файл
    }

    // Копируем заголовки, если файл пуст
    if (rapidcsv::Document(csvFile).GetRowCount() == 0) {
        copyColumnNames(constructFilePath(tjs.schemeName, tableName, "/1.csv"), csvFile);
    }

    // Записываем данные в CSV
    ofstream csv(csvFile, ios::app);
    if (!csv.is_open()) {
        cerr << "Ошибка: не удалось открыть CSV файл для записи.\n";
        return;
    }
    csv << currentPk << ",";
    
    for (size_t i = 0; i < values.size(); ++i) {
        if (values[i] == '\'') {
            ++i;
            while (i < values.size() && values[i] != '\'') {
                csv << values[i++];
            }
            csv << (values[i + 1] == ')' ? '\n' : ',');
        }
    }
    csv.close();

    toggleLock(tableName, tjs.schemeName); // разблокируем таблицу
}
