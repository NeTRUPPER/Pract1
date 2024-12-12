#pragma once

#include <iostream>
#include "insert.h"
#include "delete.h"

using namespace std;

void splitDot(const string& word, string& table, string& column, tableJson& tjs);

string ignoreQuotes(const string& word);

bool findDot(const string& word);

int countCsv(tableJson& tjs, const string& table);

void crossJoin(tableJson& tjs, const string& table1, const string& table2, const string& column1, const string& column2);

bool checkCond(tableJson& tjs, const string& table, const string& column, const string& tcond, const string& ccond, const string& s);

void select(const string& command, tableJson& tjs);
