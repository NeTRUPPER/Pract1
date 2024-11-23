#pragma once

#include <iostream>
#include <string>
#include "insert.h"
#include "delete.h"

using namespace std;

// Разделяет строку на таблицу и колонку по точке (.)
void splitDot(const string& word, string& table, string& column, tableJson& tjs);

// Убирает кавычки из строки
string removeQuotes(const string& word);

// Проверяет наличие точки в строке
bool findDot(const string& word);

// Подсчитывает количество CSV файлов для таблицы
int countCsvFiles(tableJson& tjs, const string& table);

// Выполняет операцию пересечения (cross join) для двух таблиц
void crossJoin(tableJson& tjs, const string& table1, const string& table2, const string& column1, const string& column2);

// Проверяет выполнение условия для таблицы и колонки
bool validateCondition(tableJson& tjs, const string& table, const string& column, const string& tcond, const string& ccond, const string& s);

// Основная функция для выполнения SELECT запроса
void select(const string& command, tableJson& tjs);