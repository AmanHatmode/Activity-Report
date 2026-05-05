#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

// Full absolute path to your database file
// Use a relative path for portability
#define DB_PATH "hospital_management.db"

sqlite3 *db;

// Open the database connection
int openDB()
{
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK)
    {
        if (db) {
            printf("Cannot open database: %s\n", sqlite3_errmsg(db));
        } else {
            printf("Cannot open database: out of memory\n");
        }
        return 0;
    }
    printf("Connected to %s successfully!\n", DB_PATH);
    return 1;
}

// Robustly clear the input buffer to prevent issues with fgets
void clearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Create the hospital table if it doesn't already exist
void initDB()
{
    const char *sql =
        "CREATE TABLE IF NOT EXISTS Hospital ("
        "Patient_ID INTEGER PRIMARY KEY, "
        "Name TEXT NOT NULL, "
        "Age INTEGER NOT NULL, "
        "Disease TEXT NOT NULL);";

    char *errMsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK)
    {
        printf("Error creating table: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
}

// Close the database connection
void closeDB()
{
    sqlite3_close(db);
}

// Add a new patient (INSERT INTO Hospital)
void addPatient()
{
    int id, age;
    char name[50], disease[50];

    printf("Enter Patient ID: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input for ID.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    printf("Enter Name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Enter Age: ");
    if (scanf("%d", &age) != 1) {
        printf("Invalid input for Age.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    printf("Enter Disease: ");
    fgets(disease, sizeof(disease), stdin);
    disease[strcspn(disease, "\n")] = 0;

    // Build SQL using a prepared statement (safe against SQL injection)
    const char *sql = "INSERT INTO Hospital (Patient_ID, Name, Age, Disease) VALUES (?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, age);
    sqlite3_bind_text(stmt, 4, disease, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        printf("Patient added successfully!\n");
    else
        printf("Error inserting record: %s\n", sqlite3_errmsg(db));

    sqlite3_finalize(stmt);
}

// Display all patients (SELECT * FROM Hospital)
void displayPatients()
{
    const char *sql = "SELECT Patient_ID, Name, Age, Disease FROM Hospital;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- Patient Records (FROM Hospital_management.db) ---\n");
    printf("%-12s %-20s %-6s %-20s\n", "Patient_ID", "Name", "Age", "Disease");
    printf("-------------------------------------------------------------\n");

    int found = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        found = 1;
        int pid  = sqlite3_column_int(stmt, 0);
        const char *pname = (const char *)sqlite3_column_text(stmt, 1);
        int page = sqlite3_column_int(stmt, 2);
        const char *pdis  = (const char *)sqlite3_column_text(stmt, 3);

        printf("%-12d %-20s %-6d %-20s\n", pid, pname ? pname : "N/A", page, pdis ? pdis : "N/A");
    }

    if (!found)
        printf("[!] No patient records found.\n");

    printf("-------------------------------------------------------------\n");
    sqlite3_finalize(stmt);
}

// Discharge (delete) a patient by Patient_ID
void dischargePatient()
{
    int id;
    printf("Enter Patient ID to discharge: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    const char *sql = "DELETE FROM Hospital WHERE Patient_ID = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE)
    {
        if (sqlite3_changes(db) > 0)
            printf("Patient discharged successfully!\n");
        else
            printf("Patient ID %d not found.\n", id);
    }
    else
    {
        printf("Error deleting record: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

// Search for a patient by ID
void searchPatient()
{
    int id;
    printf("Enter Patient ID to search: ");
    if (scanf("%d", &id) != 1) {
        printf("Invalid input.\n");
        clearInputBuffer();
        return;
    }
    clearInputBuffer();

    const char *sql = "SELECT Patient_ID, Name, Age, Disease FROM Hospital WHERE Patient_ID = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        printf("Error preparing statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        printf("\n--- Patient Found ---\n");
        printf("Patient ID : %d\n",  sqlite3_column_int(stmt, 0));
        printf("Name       : %s\n",  (const char *)sqlite3_column_text(stmt, 1));
        printf("Age        : %d\n",  sqlite3_column_int(stmt, 2));
        printf("Disease    : %s\n",  (const char *)sqlite3_column_text(stmt, 3));
    }
    else
    {
        printf("Patient ID %d not found.\n", id);
    }

    sqlite3_finalize(stmt);
}

int main()
{
    if (!openDB())
        return 1;

    initDB(); // Create table if it doesn't exist yet

    int choice;

    while (1)
    {
        printf("\n--- Hospital Management System ---\n");
        printf("1. Add Patient\n");
        printf("2. Discharge Patient\n");
        printf("3. Display All Patients\n");
        printf("4. Search Patient by ID\n");
        printf("5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            clearInputBuffer();
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        clearInputBuffer();

        switch (choice)
        {
        case 1:
            addPatient();
            break;
        case 2:
            dischargePatient();
            break;
        case 3:
            displayPatients();
            break;
        case 4:
            searchPatient();
            break;
        case 5:
            printf("Exiting...\n");
            closeDB();
            exit(0);
        default:
            printf("Invalid choice!\n");
        }
    }

    closeDB();
    return 0;
}
