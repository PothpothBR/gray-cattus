#ifndef INCLUDE_DATABASE_H
#define INCLUDE_DATABASE_H

#include "libpq-fe.h"
#include <stdlib.h>
#include "data.h"
#include "common.h"

#define DATABASE_HOST "localhost"
#define DATABASE_PORT "5432"
#define DATABASE_NAME "guia_ipminas"
#define DATABASE_USER "guia"
#define DATABASE_PSWD "chopinzinho0202"

class Database {

    PGconn* __conn = nullptr;

    database::Data __res;
public:

    Database() {
        char* params = new char[1024];
        sprintf(params,
            "host=%s port=%s dbname=%s user=%s password=%s",
            DATABASE_HOST,
            DATABASE_PORT, // nao testei com isso
            DATABASE_NAME,
            DATABASE_USER,
            DATABASE_PSWD
        );

        this->__conn = PQconnectdb(params);
        if (!this->__conn || PQstatus(this->__conn) != CONNECTION_OK) error("Falha ao conectar ao banco de dados");
        delete[] params;
    }

    ~Database() {
        if (this->__conn) PQfinish(this->__conn);
    }

    // criar uma função que retorna os valores e que faz type cast e verifica o tipo do valor usando o tipo na referencia da tabela
    database::Data* execute(const char* query) {
        this->__res = PQexec(this->__conn, query);
        if (!this->__res.status())
            error(this->__res.getError());

        return &this->__res;
    }

    bool status() {
        return this->__conn && PQstatus(this->__conn) == CONNECTION_OK;
    }
};

#endif // INCLUDE_DATABASE_H
