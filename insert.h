#pragma once

#include <iostream>
#include <string>
#include "pars.h"
#include "rapidcsv.hpp" // CSV обработка

using namespace std;

struct Node {       // Узел списка колонок
    string column;  // Имя колонки
    Node* next;     // Указатель на следующую колонку
};

struct tNode {      // Узел списка таблиц
    string table;   // Имя таблицы
    Node* column;   // Указатель на первую колонку таблицы
    tNode* next;    // Указатель на следующую таблицу
};

// Проверка состояния блокировки таблицы
bool isLocked(const string& tableName, const string& schemeName);

// Изменение состояния блокировки таблицы
void toggleLock(const string& tableName, const string& schemeName);

// Проверка существования таблицы в списке
bool tableExists(const string& tableName, tNode* tableHead);

// Копирование заголовков колонок из одного CSV файла в другой
void copyColumnNames(const string& sourceFile, const string& destFile);

// Вставка данных в таблицу
void insertData(const string& command);

