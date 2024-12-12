#pragma once

#include <iostream>
#include <string>
#include "insert.h"

using namespace std;

string constructFilePath(const string& schemeName, const string& tableName, const string& fileType, int fileNumber);