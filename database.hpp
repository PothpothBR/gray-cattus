#ifndef INCLUDE_DATABASE_H
#define INCLUDE_DATABASE_H

#include <libpq-fe.h>
#include "data.hpp"
#include "debug.hpp"

namespace cattus {
namespace db {

namespace dbg = cattus::debug;

class Connection {

    PGconn* conn = nullptr;

    cattus::db::Data data;
public:

    Connection(const char* host, const char* port, const char* db, const char* user, const char* password) {
        char* params = new char[1024];
        sprintf(
            params,
            "host=%s port=%s dbname=%s user=%s password=%s",
            host, port, db, user, password
        );

        conn = PQconnectdb(params);
        if (!conn || PQstatus(conn) != CONNECTION_OK) {
            dbg::error("Falha ao conectar ao banco de dados");
            PQerrorMessage(conn);
            PQfinish(conn);
            conn = nullptr;
        }

        // PQtrace(conn, stderr);
        delete[] params;
    }

    ~Connection() {
        if (conn) PQfinish(conn);
    }

    // criar uma função que retorna os valores e que faz type cast e verifica o tipo do valor usando o tipo na referencia da tabela
    cattus::db::Data* execute(const char* query) {
        data = PQexec(conn, query);
        if (!data.status())
            dbg::error(data.getError());

        return &data;
    }

    void begin() {
        PGresult* res;
        res = PQexec(conn, "BEGIN");

        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            PQfinish(conn); // trocar de finsh para reconect
        }

        PQclear(res);
    }

    void roolback() {
        PGresult* res;
        res = PQexec(conn, "ROOLBACK");

        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            PQfinish(conn); // trocar de finsh para reconect
        }

        PQclear(res);
    }

    void commit() {
        PGresult* res;
        res = PQexec(conn, "COMMIT");

        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
            PQfinish(conn); // trocar de finsh para reconect
        }

        PQclear(res);
    }

    bool status() {
        return conn && PQstatus(conn) == CONNECTION_OK;
    }
} extern global_conn("localhost", "5432", "feneco_database", "postgres", "chopinzinho0202");

}
}


#endif // INCLUDE_DATABASE_H
