#include "selectWere.h"

// Разделение строки формата "table.column" на table и column
void splitDot(const string& word, string& table, string& column, tableJson& tjs) { 
    bool hasDot = false; // Проверка, что есть точка
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '.') {
            hasDot = true; // Нашли точку, переключаемся на column
            continue;
        }
        if (word[i] == ',') {
            continue;
        }
        if (!hasDot) { // Заполняем table, пока нет точки
            table += word[i];
        }
        else { // После точки заполняем column
            column += word[i];
        }
    }

    // Проверка, что вообще была точка в слове
    if (!hasDot) { 
        cerr << "Некорректная команда.\n";
        return;
    }
    
    // Проверка существования таблицы и колонки
    if (!tableExists(table, tjs.tablehead)) { 
        cerr << "Такой таблицы нет.\n";
        return;
    }
    if (isColumnExist(table, column, tjs.tablehead) == false) {
        cerr << "Такой колонки нет.\n";
        return;
    }
}

// Убирает одинарные кавычки из строки
string removeQuotes(const string& word) { 
    string result;
    for (char ch : word) {
        if (ch != '\'') {
            result += ch;
        }
    }
    return result;
}

// Проверка, есть ли точка в строке
bool findDot(const string& word) { 
    return word.find('.') != string::npos;
}

// Подсчет количества CSV файлов для таблицы
int countCsvFiles(tableJson& tjs, const string& table) {
    int fileCount = 1; // Начинаем с 1
    while (true) {
        string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table + "/" + to_string(fileCount) + ".csv";
        ifstream file(filePath);
        if (!file.is_open()) { // Если файл не открывается, значит его нет
            break;
        }
        file.close();
        fileCount++;
    }
    return fileCount;
}

// Выполнение операции CROSS JOIN между двумя таблицами
void crossJoin(tableJson& tjs, const string& table1, const string& table2, const string& column1, const string& column2) {
    int csvCount1 = countCsvFiles(tjs, table1); // Количество CSV у первой таблицы
    int csvCount2 = countCsvFiles(tjs, table2); // Количество CSV у второй таблицы
    
    for (int iCsv1 = 1; iCsv1 < csvCount1; iCsv1++) {
        string filePath1 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table1 + "/" + to_string(iCsv1) + ".csv";
        rapidcsv::Document doc1(filePath1);
        int colIdx1 = doc1.GetColumnIdx(column1);
        size_t rowCount1 = doc1.GetRowCount();
        
        for (size_t row1 = 0; row1 < rowCount1; ++row1) {
            for (int iCsv2 = 1; iCsv2 < csvCount2; iCsv2++) {
                string filePath2 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table2 + "/" + to_string(iCsv2) + ".csv";
                rapidcsv::Document doc2(filePath2);
                int colIdx2 = doc2.GetColumnIdx(column2);
                size_t rowCount2 = doc2.GetRowCount();

                for (size_t row2 = 0; row2 < rowCount2; ++row2) {
                    // Вывод данных из обеих таблиц в терминал
                    cout << doc1.GetCell<string>(0, row1) << ": ";
                    cout << doc1.GetCell<string>(colIdx1, row1) << "  |   ";
                    cout << doc2.GetCell<string>(0, row2) << ": ";
                    cout << doc2.GetCell<string>(colIdx2, row2) << endl;
                }
            }
        }
    }
}

// Проверка выполнения условия для SELECT с WHERE
bool validateCondition(tableJson& tjs, const string& table, const string& column, const string& conditionTable, const string& conditionColumn, const string& conditionValue) {
    if (!conditionValue.empty()) {
        int csvCount = countCsvFiles(tjs, table);
        for (int iCsv = 1; iCsv < csvCount; iCsv++) {
            string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc(filePath);
            int colIdx = doc.GetColumnIdx(column);
            size_t rowCount = doc.GetRowCount();

            for (size_t row = 0; row < rowCount; ++row) {
                if (doc.GetCell<string>(colIdx, row) == conditionValue) { // Совпадение значения с условием
                    return true;
                }
            }
        }
    } else {
        bool conditionMet = true;
        int csvCount = countCsvFiles(tjs, table);
        for (int iCsv = 1; iCsv < csvCount; iCsv++) {
            string pk1, pk2;
            // Файлы с PK для сравнения
            string pkPath1 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table + "/" + table + "_pk_sequence.txt";
            string pkPath2 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + conditionTable + "/" + conditionTable + "_pk_sequence.txt";

            ifstream file1(pkPath1);
            if (!file1.is_open()) {
                cerr << "Не удалось открыть файл.\n";
                return false;
            }
            file1 >> pk1;
            file1.close();
            
            ifstream file2(pkPath2);
            if (!file2.is_open()) {
                cerr << "Не удалось открыть файл.\n";
                return false;
            }
            file2 >> pk2;
            file2.close();

            if (pk1 != pk2) { // Сравнение PK
                return false;
            }

            // Проверка по каждому CSV файлу
            string filePath1 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            string filePath2 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1" + tjs.schemeName + "/" + conditionTable + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc1(filePath1);
            int colIdx1 = doc1.GetColumnIdx(column);
            size_t rowCount1 = doc1.GetRowCount();
            rapidcsv::Document doc2(filePath2);
            int colIdx2 = doc2.GetColumnIdx(conditionColumn);

            for (size_t row = 0; row < rowCount1; ++row) {
                if (doc1.GetCell<string>(colIdx1, row) != doc2.GetCell<string>(colIdx2, row)) {
                    conditionMet = false;
                }
            }
        }
        if (conditionMet) {
            return true;
        }
    }
    return false;
}

// Основная функция для SELECT команды
void select(const string& command, tableJson& tjs) {
    istringstream iss(command);
    string word;
    
    iss >> word; // SELECT
    iss >> word; // "таблица 1"
    string table1, column1;
    splitDot(word, table1, column1, tjs); // Разделение table1 и column1
    
    iss >> word; // "таблица 2"
    string table2, column2;
    splitDot(word, table2, column2, tjs); // Разделение table2 и column2

    string secondCmd;
    getline(cin, secondCmd); 
    istringstream iss2(secondCmd);
    iss2 >> word; // FROM
    if (word != "FROM") {
        cerr << "Некорректная команда.\n";
        return;
    }

    // Проверка совпадения таблиц
    iss2 >> word;
    string tab1;
    for (char ch : word) {
        if (ch != ',') {
            tab1 += ch;
        }
    }
    if (tab1 != table1) {
        cerr << "Некорректная команда.\n";
        return;
    }
    
    iss2 >> word;
    if (word != table2) {
        cerr << "Некорректная команда.\n";
        return;
    }

    string thirdCmd;
    getline(cin, thirdCmd);
    istringstream iss3(thirdCmd);
    iss3 >> word; // WHERE
    if (word != "WHERE") {
        crossJoin(tjs, table1, table2, column1, column2); // Выполнение CROSS JOIN без условий
    } else {
        iss3 >> word;
        string conditionTable, conditionColumn;
        splitDot(word, conditionTable, conditionColumn, tjs); // Разделение table.column
        string conditionValue;
        iss3 >> word; // Операция равенства "="
        iss3 >> word;
        conditionValue = removeQuotes(word);
        
        if (validateCondition(tjs, table1, column1, conditionTable, conditionColumn, conditionValue)) {
            crossJoin(tjs, table1, table2, column1, column2); // Выполнение CROSS JOIN с условием
        } else {
            cerr << "Условие не выполнено.\n";
        }
    }
}
