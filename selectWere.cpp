#include "selectWere.h"
#include <sstream>

// Разделение строки вида <таблица>.<колонка> на таблицу и колонку
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

// Удаление одинарных кавычек из строки
std::string ignoreQuotes(const std::string& word) {
    std::string result;
    for (char ch : word) {
        if (ch != '\'') {
            result += ch;
        }
    }
    return result;
}

// Удаление пробелов, табуляций и переносов строк с начала и конца строки
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

// Проверка на наличие точки в строке
bool findDot(const std::string& word) {
    return word.find('.') != std::string::npos;
}

// Подсчет количества CSV-файлов, относящихся к таблице
int countCsv(tableJson& tjs, const std::string& table) {
    int csvCount = 0;

    while (true) {
        std::string filePath = constructFilePath(tjs.schemeName, table, ".csv", csvCount + 1);

        std::ifstream file(filePath);
        if (!file) {
            break; // очередной файл не найден
        }
        file.close();
        csvCount++;
    }

    return csvCount;
}

// Выполнение CROSS JOIN между двумя таблицами по заданным колонкам
void crossJoin(tableJson& tjs, const std::string& table1, const std::string& table2, const std::string& column1, const std::string& column2) {
    tNode* t1 = findTable(table1, tjs.tablehead);
    tNode* t2 = findTable(table2, tjs.tablehead);
    if (!t1 || !t2) {
        cerr << "Таблица не найдена.\n";
        return;
    }
    int idx1 = getColumnIndex(table1, column1, tjs.tablehead);
    int idx2 = getColumnIndex(table2, column2, tjs.tablehead);
    if (idx1 < 0 || idx2 < 0) {
        cerr << "Колонка не найдена.\n";
        return;
    }

    for (RowNode* r1 = t1->rows; r1; r1 = r1->next) {
        for (RowNode* r2 = t2->rows; r2; r2 = r2->next) {
            cout << getValueAt(r1, 0) << ": " << getValueAt(r1, idx1) << "  |   ";
            cout << getValueAt(r2, 0) << ": " << getValueAt(r2, idx2) << "\n";
        }
    }
}

// Вывод одной таблицы и выбранной колонки или всех колонок
void singleSelect(tableJson& tjs, const std::string& table, const std::string& column, bool allColumns) {
    tNode* t = findTable(table, tjs.tablehead);
    if (!t) {
        cerr << "Таблица не найдена.\n";
        return;
    }
    if (!allColumns) {
        int idx = getColumnIndex(table, column, tjs.tablehead);
        if (idx < 0) {
            cerr << "Колонка не найдена.\n";
            return;
        }
        for (RowNode* r = t->rows; r; r = r->next) {
            cout << getValueAt(r, 0) << ": " << getValueAt(r, idx) << "\n";
        }
        return;
    }

    // Вывод всех колонок
    for (RowNode* r = t->rows; r; r = r->next) {
        cout << getValueAt(r, 0) << ": ";
        int colIdx = 1;
        for (Node* col = t->column; col; col = col->next, ++colIdx) {
            cout << getValueAt(r, colIdx - 1); // pk идет первым в списке
            if (col->next) cout << ", ";
        }
        cout << "\n";
    }
}

// Функция проверки условий для SQL-выражения WHERE
bool checkCond(tableJson& tjs, const string& table, const string& column, const string& tcond, const string& ccond, const string& s)
{
    // 1) Случай table.column = 'строка'
    if (!s.empty()) {
        tNode* t = findTable(table, tjs.tablehead);
        int idx = getColumnIndex(table, column, tjs.tablehead);
        for (RowNode* r = t ? t->rows : nullptr; r; r = r->next) {
            if (getValueAt(r, idx) == s) {
                return true;
            }
        }
    }

    // 2) Случай (сравнение двух колонок)
    tNode* t1 = findTable(table, tjs.tablehead);
    tNode* t2 = findTable(tcond, tjs.tablehead);
    int idx1 = getColumnIndex(table, column, tjs.tablehead);
    int idx2 = getColumnIndex(tcond, ccond, tjs.tablehead);
    for (RowNode* r1 = t1 ? t1->rows : nullptr; r1; r1 = r1->next) {
        for (RowNode* r2 = t2 ? t2->rows : nullptr; r2; r2 = r2->next) {
            if (getValueAt(r1, idx1) == getValueAt(r2, idx2)) {
                return true; // нашли совпадение между колонками
            }
        }
    }

    return false; // ни одной пары совпадающих значений не найдено
}


// Функция обработки команды SQL SELECT
void select(const string& command, tableJson& tjs) {
    istringstream iss(command);
    string word;
    iss >> word;  // Ожидается "SELECT"
    if (word != "SELECT") {
        cerr << "Неверная команда.\n";
        return;
    }

    // Разбор первой таблицы и колонки
    iss >> word;
    string table1, column1;
    bool selectAllColumns = false;
    if (findDot(word)) {
        splitDot(word, table1, column1, tjs);
    } else {
        table1 = word;
        selectAllColumns = true;
    }

    // Попытка разобрать вторую таблицу и колонку
    bool hasSecondTable = false;
    string table2, column2;
    if (iss >> word) {
        hasSecondTable = true;
        splitDot(word, table2, column2, tjs);
    }

    // Чтение и разбор второй строки команды
    string secondCmd;
    getline(cin, secondCmd);
    istringstream iss2(secondCmd);

    iss2 >> word;  // Ожидается "FROM"
    if (word != "FROM") {
        cerr << "Неверная команда.\n";
        return;
    }

    vector<string> fromTables;
    while (iss2 >> word) {
        string cleaned;
        for (char ch : word) {
            if (ch != ',') cleaned += ch;
        }
        if (!cleaned.empty()) {
            fromTables.push_back(cleaned);
        }
    }

    if (hasSecondTable) {
        if (fromTables.size() != 2 || fromTables[0] != table1 || fromTables[1] != table2) {
            cerr << "Неверная команда.\n";
            return;
        }
    } else {
        if (fromTables.size() != 1 || fromTables[0] != table1) {
            cerr << "Неверная команда.\n";
            return;
        }
    }

    // Чтение и разбор третьей строки команды
    string thirdCmd;
    getline(cin, thirdCmd);
    istringstream iss3(thirdCmd);

    if (!(iss3 >> word)) {
        if (hasSecondTable) {
            crossJoin(tjs, table1, table2, column1, column2);
        } else {
            singleSelect(tjs, table1, column1, selectAllColumns);
        }
        return;
    }

    if (word != "WHERE") {
        cerr << "Неверная команда.\n";
        return;
    }

    if (!hasSecondTable) {
        // Разбор первого условия
        iss3 >> word;
        string t1, c1;
        splitDot(word, t1, c1, tjs);

        iss3 >> word;
        if (word != "=") {
            cerr << "Неверная команда.\n";
            return;
        }

        iss3 >> word;
        string t1cond = "", c1cond = "", s1 = "";
        if (findDot(word)) {
            splitDot(word, t1cond, c1cond, tjs);
        } else {
            s1 = ignoreQuotes(word);
        }

        string oper;
        if (!(iss3 >> oper)) {
            if (checkCond(tjs, t1, c1, t1cond, c1cond, s1)) {
                singleSelect(tjs, table1, column1, selectAllColumns);
            } else {
                cout << "Условие не выполнено.\n";
            }
            return;
        }

        if (oper != "AND" && oper != "OR") {
            cerr << "Неверная команда.\n";
            return;
        }

        iss3 >> word;
        string t2, c2;
        splitDot(word, t2, c2, tjs);

        iss3 >> word;
        if (word != "=") {
            cerr << "Неверная команда.\n";
            return;
        }

        iss3 >> word;
        string t2cond = "", c2cond = "", s2 = "";
        if (findDot(word)) {
            splitDot(word, t2cond, c2cond, tjs);
        } else {
            s2 = ignoreQuotes(word);
        }

        bool cond1 = checkCond(tjs, t1, c1, t1cond, c1cond, s1);
        bool cond2 = checkCond(tjs, t2, c2, t2cond, c2cond, s2);
        if ((oper == "AND" && cond1 && cond2) || (oper == "OR" && (cond1 || cond2))) {
            singleSelect(tjs, table1, column1, selectAllColumns);
        } else {
            cout << "Условие не выполнено.\n";
        }
        return;
    }

    // Разбор первого условия в выражении WHERE
    iss3 >> word;
    string t1, c1;
    splitDot(word, t1, c1, tjs);

    iss3 >> word;  // Ожидается "="
    if (word != "=") {
        cerr << "Неверная команда.\n";
        return;
    }

    iss3 >> word;
    string t1cond = "", c1cond = "", s1 = "";
    if (findDot(word)) {
        splitDot(word, t1cond, c1cond, tjs);
    } else {
        s1 = ignoreQuotes(word); // Извлечение значения без кавычек
    }

    // Разбор опционального оператора AND/OR
    iss3 >> word;
    string oper = word;
    if (oper != "AND" && oper != "OR") {
        // Случай одного условия
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1)) {
            crossJoin(tjs, table1, table2, column1, column2);
        } else {
            cout << "Условие не выполнено.\n";
        }
        return;
    }

    // Разбор второго условия в выражении WHERE
    iss3 >> word;
    string t2, c2;
    splitDot(word, t2, c2, tjs);

    iss3 >> word;  // Ожидается "="
    if (word != "=") {
        cerr << "Неверная команда.\n";
        return;
    }

    iss3 >> word;
    string t2cond = "", c2cond = "", s2 = "";
    if (findDot(word)) {
        splitDot(word, t2cond, c2cond, tjs);
    } else {
        s2 = ignoreQuotes(word); // Извлечение значения без кавычек
    }

    // Оценка условий AND/OR
    if (oper == "AND") {
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1) && checkCond(tjs, t2, c2, t2cond, c2cond, s2)) {
            if (hasSecondTable) {
                crossJoin(tjs, table1, table2, column1, column2);
            } else {
                singleSelect(tjs, table1, column1, selectAllColumns);
            }
        } else {
            cout << "Условие не выполнено.\n";
        }
    } else if (oper == "OR") {
        if (checkCond(tjs, t1, c1, t1cond, c1cond, s1) || checkCond(tjs, t2, c2, t2cond, c2cond, s2)) {
            if (hasSecondTable) {
                crossJoin(tjs, table1, table2, column1, column2);
            } else {
                singleSelect(tjs, table1, column1, selectAllColumns);
            }
        } else {
            cout << "Условие не выполнено.\n";
        }
    }
}
