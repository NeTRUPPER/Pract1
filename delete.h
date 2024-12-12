#pragma once
#include <iostream>
#include <fstream>
#include "insert.h"

using namespace std;

// Удаление строк из таблицы на основе команды
void del(const string& command, tableJson& tjs);

// Проверка наличия колонки в таблице
bool isColumnExist(const string& tableName, const string& columnName, tNode* tableHead);


