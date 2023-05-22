#ifndef INCLUDE_DATABASE_H
#define INCLUDE_DATABASE_H

#include <libpq-fe.h>
#include "data.hpp"
#include "common.hpp"

namespace cattus {
namespace db {

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
        if (!conn || PQstatus(conn) != CONNECTION_OK) cmn::error("Falha ao conectar ao banco de dados");
        delete[] params;
    }

    ~Connection() {
        if (conn) PQfinish(conn);
    }

    // criar uma função que retorna os valores e que faz type cast e verifica o tipo do valor usando o tipo na referencia da tabela
    cattus::db::Data* execute(const char* query) {
        data = PQexec(conn, query);
        if (!data.status())
            cmn::error(data.getError());

        return &data;
    }

    bool status() {
        return conn && PQstatus(conn) == CONNECTION_OK;
    }
};

}
}


#endif // INCLUDE_DATABASE_H
