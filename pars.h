#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>  // Директории
#include "json.hpp"    // Работа с json
#include "insert.h"    // Структура таблиц

using namespace std;

using json = nlohmann::json; 
namespace fs = filesystem;

struct tableJson {
    tNode* tablehead; // Указатель на голову списка таблиц
    string schemeName;
    int tableSize;
};

// Удаление директории
void removeDirectory(const fs::path& directoryPath); 

// Создание полной директории и файлов
void createDirectoriesAndFiles(const fs::path& schemePath, const json& structure, tableJson& tjs); 

// Парсинг json
void parsing(tableJson& tjs); 