#include "pars.h"
#include "insert.h"
#include "delete.h"
#include "selectWere.h"

#include "pars.cpp"
#include "insert.cpp"
#include "delete.cpp"
#include "selectWere.cpp"

void processCommand(const string& command, tableJson& tjs) { // Обработка команды
    if (command == "EXIT") {
        exit(0); // Выход из программы
    }
    else if (command.find("INSERT") == 0) {
        insert(command, tjs); // Вставка данных
    }
    else if (command.find("DELETE") == 0) {
        del(command, tjs); // Удаление данных
    }
    else if (command.find("SELECT") == 0) {
        select(command, tjs); // Выбор данных
    }
    else {
        cerr << "Неизвестная команда.\n"; // Неизвестная команда
    }
}

int main() {
    tableJson tjs;
    parsing(tjs); // Парсинг схемы
    cout << "\n\n";
    string command; // Строка для ввода команды
    while (true) {
        cout << "Введите команду: ";
        getline(cin, command); // Чтение команды с консоли
        if (command == "") { // Если введена пустая строка
            continue;
        }
        processCommand(command, tjs); // Обработка команды
    }
    return 0;
}
