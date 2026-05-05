#include <stdio.h>
#include "sqlite3.h"

int main() {
    sqlite3 *db;
    int rc = sqlite3_open("hospital_management.db", &db);
    if (rc != SQLITE_OK) {
        printf("Cannot open database\n");
        return 1;
    }

    const char *sql = "SELECT name FROM sqlite_master WHERE type='table';";
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

    printf("Tables in database:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("- %s\n", sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
