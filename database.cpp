#include <iostream>
#include <sqlite3.h>

int main() {
    // Инициализация базы данных SQLite
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    else {
        std::cout << "Opened database successfully" << std::endl;
    }

    // Создание таблицы для сохранения вычислений
    char* errorMsg;
    const char* createTableQuery = "CREATE TABLE calculations (id INTEGER PRIMARY KEY AUTOINCREMENT, expression TEXT, result REAL);";
    rc = sqlite3_exec(db, createTableQuery, NULL, 0, &errorMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errorMsg << std::endl;
        sqlite3_free(errorMsg);
    }
    else {
        std::cout << "Table created successfully" << std::endl;
    }

    // Закрытие базы данных
    sqlite3_close(db);

    return 0;
}
