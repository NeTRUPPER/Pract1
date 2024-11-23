#include "pars.h"
#include "insert.h"
#include "delete.h"
#include "selectWere.h"

#include "pars.cpp"
#include "insert.cpp"
#include "delete.cpp"
#include "selectWere.cpp"

using namespace std;

int main() {
    tableJson tjs;
    parsing(tjs);

    string command;
    while (true) {
        cout << "Введите команду: ";
        getline(cin, command);

        if (command.empty()) continue;

        if (command.find("EXIT") == 0) { // Проверка на команду выхода
            return 0;
        } else if (command.find("INSERT") == 0) { // Проверка на INSERT
            insertData(command, tjs);
        } else if (command.find("DELETE") == 0) { // Проверка на DELETE
            del(command, tjs);
        } else if (command.find("SELECT") == 0) { // Проверка на SELECT
            select(command, tjs);
        } else {
            cerr << "Неизвестная команда.\n"; // Неизвестная команда
        }
    }

    return 0;
}
