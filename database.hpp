#ifndef INCLUDE_DATABASE_H
#define INCLUDE_DATABASE_H

#include <libpq-fe.h>
#include "data.hpp"
#include "debug.hpp"
#include <semaphore>
#include <iostream>
#include <thread>
#include <vector>

namespace cattus {
namespace db {

namespace dbg = cattus::debug;

class Connection {
    PGconn* conn = nullptr;

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

        std::cout << "libpq (thread safety) " << PQisthreadsafe() << std::endl;

        delete[] params;
    }

   // Connection(Connection&& other) {
        //conn = std::move(other.conn);
        //other.conn = nullptr;
    //}

    ~Connection() {
        //if (conn) PQfinish(conn);
    }

    // criar uma função que retorna os valores e que faz type cast e verifica o tipo do valor usando o tipo na referencia da tabela
    cattus::db::Data execute(const char* query) {
        cattus::db::Data data(PQexec(conn, query));
        if (!data.status()) dbg::error(data.getError());
        return data;
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
};

class ConnectionPool{
    static Connection** connPool;
    static std::binary_semaphore lock;
    static unsigned int index;
public:
    static void start(const char* host, const char* port, const char* db, const char* user, const char* password){
        const unsigned int concurrency = std::thread::hardware_concurrency();
        lock.acquire();
        connPool = new Connection*[concurrency];
        index = concurrency;
        for (unsigned int i = 0; i < concurrency; i++) {
            connPool[i] = new Connection(host, port, db, user, password);
        }
        lock.release();
    }

    static auto *take(){
        lock.acquire();
        Connection* conn = connPool[--index];
        lock.release();
        return conn;
    }

    static void reuse(auto conn){
        lock.acquire();
        connPool[index++] = conn;
        lock.release();
    }

    ConnectionPool() = delete;
};

Connection** ConnectionPool::connPool;
std::binary_semaphore ConnectionPool::lock{ 1 };
unsigned int ConnectionPool::index;

}
}


#endif // INCLUDE_DATABASE_H
