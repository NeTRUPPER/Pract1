#pragma once

#include <iostream>
#include <fstream>
#include "insert.h"

using namespace std;

// Удаление
void del(const string& command, tableJson& tjs); 

// Поиск колонки
bool isColumnExist(const string& tableName, const string& columnName, tableJson& tjs); 