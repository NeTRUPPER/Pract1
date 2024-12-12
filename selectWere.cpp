#include "selectWere.h"

void splitDot(const string& word, string& table, string& column, tableJson& tjs) { 
    bool dot = false;  // Поиск точки
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '.') {
            dot = true;
            continue;
        }
        if (word[i] == ',') {
            continue;
        }
        if (!dot) {  // Разделяем таблицу и колонку
            table += word[i];
        } else {
            column += word[i];
        }
    }
    if (!dot) {  // Если точка не найдена
        cerr << "Некорректная команда.\n";
        return;
    }
    if (isTableExist(table, tjs.tablehead) == false) {  // Проверка на существование таблицы
        cerr << "Такой таблицы нет.\n";
        return;
    }
    if (isColumnExist(table, column, tjs.tablehead) == false) {  // Проверка на существование колонки
        cerr << "Такой колонки нет.\n";
        return;
    }
}

string ignoreQuotes(const string& word) {  // Удаление кавычек из строки
    string slovo;
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] != '\'') {
            slovo += word[i];
        }
    }
    return slovo;
}

bool findDot(const string& word) {  // Проверка на наличие точки в слове
    bool dot = false;
    for (size_t i = 0; i < word.size(); i++) {
        if (word[i] == '.') {
            dot = true;
        }
    }
    return dot;
}

int countCsv(tableJson& tjs, const string& table) {
    int amountCsv = 1;  // Поиск количества созданных csv файлов
    while (true) {
        string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table + "/" + to_string(amountCsv) + ".csv";
        ifstream file(filePath);
        if (!file) {
            break; // Нет файла, выходим из цикла
        }
        file.close();
        amountCsv++;
    }
    return amountCsv;
}

void crossJoin(tableJson& tjs, const string& table1, const string& table2, const string& column1, const string& column2) {
    int amountCsv1 = countCsv(tjs, table1); // количество файлов 1 таблицы
    int amountCsv2 = countCsv(tjs, table2); // количество файлов 2 таблицы
    for (int iCsv1 = 1; iCsv1 <= amountCsv1; iCsv1++) {  // <= вместо <
        string filePath1 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table1 + "/" + to_string(iCsv1) + ".csv";
        rapidcsv::Document doc1(filePath1); // открываем файл 1
        int columnIndex1 = doc1.GetColumnIdx(column1); // считываем индекс искомой колонки 1
        if (columnIndex1 == -1) {
            cerr << "Колонка " << column1 << " не найдена в таблице " << table1 << ".\n";
            return;
        }
        int amountRow1 = doc1.GetRowCount(); // считываем количество строк в файле 1
        for (int i = 1; i < static_cast<int>(amountRow1); ++i) { // проходимся по строкам 1
            for (int iCsv2 = 1; iCsv2 <= amountCsv2; iCsv2++) {  // <= вместо <
                string filePath2 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table2 + "/" + to_string(iCsv2) + ".csv";
                rapidcsv::Document doc2(filePath2); // открываем файл 2
                int columnIndex2 = doc2.GetColumnIdx(column2); // считываем индекс искомой колонки 2
                if (columnIndex2 == -1) {
                    cerr << "Колонка " << column2 << " не найдена в таблице " << table2 << ".\n";
                    return;
                }
                int amountRow2 = doc2.GetRowCount(); // считываем количество строк в файле2
                for (int j = 1; j < amountRow2; ++j) {
                    // Сравниваем значения столбцов
                    if (doc1.GetCell<string>(columnIndex1, i) == doc2.GetCell<string>(columnIndex2, j)) {
                        cout << doc1.GetCell<string>(0, i) << ": ";
                        cout << doc1.GetCell<string>(columnIndex1, i) << "  |   ";
                        cout << doc2.GetCell<string>(0, j) << ": ";
                        cout << doc2.GetCell<string>(columnIndex2, j) << endl;
                    }
                }
            }
        }
    }
}



bool checkCond(tableJson& tjs, const string& table, const string& column, const string& tcond, const string& ccond, const string& s) {
    if (s != "") {
        int amountCsv = countCsv(tjs, table);
        for (int iCsv = 1; iCsv < amountCsv; iCsv++) { // просматриваем все созданные файлы csv
            string filePath = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc(filePath); // открываем файл
            int columnIndex = doc.GetColumnIdx(column); // считываем индекс искомой колонки
            int amountRow = doc.GetRowCount(); // считываем количество строк в файле
            for (int i = 1; i < amountRow; ++i) { // проходимся по строкам
                if (doc.GetCell<string>(columnIndex, i) == s) { // извлекаем значение (индекс колонки, номер строки)
                    return true;
                }
            }
        }
    }
    else {
        bool condition = true;
        int amountCsv = countCsv(tjs, table);
        for (int iCsv = 1; iCsv < amountCsv; iCsv++) {
            string pk1, pk2;
            string pk1Path = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table + "/" + table + "_pk_sequence.txt";
            string pk2Path = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + tcond + "/" + tcond + "_pk_sequence.txt";
            ifstream file1(pk1Path);
            if (!file1.is_open()) {
                cerr << "Не удалось открыть файл для проверки первичных ключей.\n";
                return false;
            }
            file1 >> pk1;
            file1.close();
            ifstream file2(pk2Path);
            if (!file2.is_open()) {
                cerr << "Не удалось открыть файл для проверки первичных ключей.\n";
                return false;
            }
            file2 >> pk2;
            file2.close();
            if (pk1 != pk2) {
                return false;
            }

            string filePath1 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + table + "/" + to_string(iCsv) + ".csv";
            string filePath2 = "/home/vlad/Documents/VC Code/SecondSemestr/pract1/" + tjs.schemeName + "/" + tcond + "/" + to_string(iCsv) + ".csv";
            rapidcsv::Document doc1(filePath1); // открываем файл
            int columnIndex1 = doc1.GetColumnIdx(column); // считываем индекс искомой колонки
            int amountRow1 = doc1.GetRowCount(); // считываем количество строк в файле
            rapidcsv::Document doc2(filePath2); // открываем файл
            int columnIndex2 = doc2.GetColumnIdx(ccond); // считываем индекс искомой колонки
            for (int i = 1; i < amountRow1; ++i) { // проходимся по строкам
                if (doc1.GetCell<string>(columnIndex1, i) != doc2.GetCell<string>(columnIndex2, i)) {
                    condition = false;
                }
            }
        }
        if (condition) {
            return true;
        }
    }
    return false;
}


void select(const string& command, tableJson& tjs) {  // Выбор данных
    istringstream iss(command);  // Поток ввода для обработки строки команды
    string word;
    iss >> word;  // "SELECT"
    if (word.empty()) {
    cerr << "Некорректная команда.\n";
    return;
}
    iss >> word;  // "таблица 1"
    if (word.empty()) {
    cerr << "Некорректная команда.\n";
    return;
}
    string table1, column1;  // Строка для 1 таблицы и колонки
    splitDot(word, table1, column1, tjs);  // Разделяем таблицу и колонку
    iss >> word;  // "таблица 2"
    string table2, column2;  // Строка для 2 таблицы и колонки
    splitDot(word, table2, column2, tjs);  // Разделяем таблицу и колонку

    string secondCmd;  // Вторая часть команды с FROM
    getline(cin, secondCmd); 
    istringstream iss2(secondCmd);
    iss2 >> word;  // "FROM"
    if (word != "FROM") {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss2 >> word;  // Таблица 1
    string tab1;
    for (int i = 0; i < word.size(); i++) {
        if (word[i] != ',') {
            tab1 += word[i];
        }
    }
    if (tab1 != table1) {
        cerr << "Некорректная команда.\n";
        return;
    }
    iss2 >> word;  // Таблица 2
    if (word != table2) {
        cerr << "Некорректная команда.\n";
        return;
    }

    string thirdCmd;  // Третья часть команды
    getline(cin, thirdCmd);
    istringstream iss3(thirdCmd);
    string condition;
    iss3 >> condition;
    if (condition == "WHERE") {
        iss3 >> condition;
        if (condition != "") {
            if (checkCond(tjs, table1, column1, table2, column2, condition)) {
                crossJoin(tjs, table1, table2, column1, column2);
            } else {
                cerr << "Нет таких данных.\n";
            }
        }
    }
}
