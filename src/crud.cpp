#include "crud.h"
#include <iostream>
#include <libpq-fe.h>

int selectFromDatabase(const std::string& connInfo, const std::string& selectQuery)
{
    PGconn* conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database Connection Failed: " << PQerrorMessage(conn) << '\n';
        PQfinish(conn);
        return -1;
    }

    PGresult* res = PQexec(conn, selectQuery.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "SELECT failed: " << PQerrorMessage(conn) << '\n';
        PQclear(res);
        PQfinish(conn);
        return -1;
    }

    int nrows = PQntuples(res);
    int nfields = PQnfields(res);
    for (int i = 0; i < nrows; i++) {
        for (int j = 0; j < nfields; j++) {
            std::cout << PQgetvalue(res, i, j) << " ";
        }
        std::cout << '\n';
    }

    PQclear(res);
    PQfinish(conn);
    return 0;
}

static PGconn* connectOrNull(const std::string& connInfo)
{
    PGconn* conn = PQconnectdb(connInfo.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Database Connection Failed: " << PQerrorMessage(conn) << '\n';
        PQfinish(conn);
        return nullptr;
    }
    return conn;
}

void insertData(const std::string& connInfo,
                const std::string& tableName, int id,
                const std::string& fName, const std::string& lName)
{
    PGconn* conn = connectOrNull(connInfo);
    if (!conn) return;

    std::string sql = "INSERT INTO " + tableName +
        " (id, fName, lName) VALUES (" +
        std::to_string(id) + ", '" + fName + "', '" + lName + "');";

    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        std::cerr << "INSERT failed: " << PQerrorMessage(conn) << '\n';
    else
        std::cout << "INSERT succeeded.\n";

    PQclear(res);
    PQfinish(conn);
}

void deleteData(const std::string& connInfo,
                const std::string& tableName,
                const std::string& fName, const std::string& lName)
{
    PGconn* conn = connectOrNull(connInfo);
    if (!conn) return;

    std::string sql = "DELETE FROM " + tableName +
        " WHERE fName='" + fName + "' AND lName='" + lName + "';";

    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        std::cerr << "DELETE failed: " << PQerrorMessage(conn) << '\n';
    else
        std::cout << "DELETE succeeded.\n";

    PQclear(res);
    PQfinish(conn);
}

void updateData(const std::string& connInfo,
                const std::string& tableName, int id,
                const std::string& fName, const std::string& lName)
{
    PGconn* conn = connectOrNull(connInfo);
    if (!conn) return;

    std::string sql = "UPDATE " + tableName +
        " SET fName='" + fName + "', lName='" + lName +
        "' WHERE id=" + std::to_string(id) + ";";

    PGresult* res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
        std::cerr << "UPDATE failed: " << PQerrorMessage(conn) << '\n';
    else
        std::cout << "UPDATE succeeded.\n";

    PQclear(res);
    PQfinish(conn);
}
