#ifndef INCLUDE_DATABASE_DATA_H
#define INCLUDE_DATABASE_DATA_H

#include <string.h>
#include "libpq-fe.h"
#include <boost/json.hpp>
#include "debug.hpp"

namespace json = boost::json;

namespace cattus {
namespace db {

class Data {
    PGresult *res;
    unsigned int index = 0;
    unsigned int rows = 0;
    ExecStatusType status_bit = PGRES_NONFATAL_ERROR;

    char* verify_bit = nullptr;

public:
    Data(PGresult* result) : res(result) {
        status_bit = PQresultStatus(res);
        rows = PQntuples(res);
    }
    ~Data() {
        if (res) PQclear(res);
    }
    Data& operator=(Data&& other) {
        res = std::move(other.res);
        other.res = nullptr;
        return *this;
    }

    Data(Data&& other) {
        res = std::move(other.res);
        other.res = nullptr;
    }

    inline const char* getString(unsigned int column) {
        const char* val = PQgetvalue(res, index, column);
        if (val) { return val; }
        return "";
    }
    inline const char* getString(const char* column) {
        const char* val = PQgetvalue(res, index, PQfnumber(res, column));
        if (val) { return val; }
        return "";
    }

    template<typename T>
    inline void get(T column, long int *res) { *res = strtol(getString(column), &verify_bit, 10); }

    template<typename T>
    inline void get(T column, long long int* res) { *res = strtoll(getString(column), &verify_bit, 10); }

    template<typename T>
    inline void get(T column, float *res) { *res = strtof(getString(column), &verify_bit); }

    template<typename T>
    inline void get(T column, double* res) { *res = strtod(getString(column), &verify_bit); }

    template<typename T>
    inline void get(T column, long double* res) { *res = strtold(getString(column), &verify_bit); }

    template<typename T>
    inline void get(T column, unsigned long int* res) { *res = strtoul(getString(column), &verify_bit, 10); }

    template<typename T>
    inline void get(T column, unsigned long long int* res) { *res = strtoull(getString(column), &verify_bit, 10); }
    
    template<typename T>
    inline void get(T column, int* res) { *res = static_cast<int>(strtol(getString(column), &verify_bit, 10)); }
    
    template<typename T>
    inline void get(T column, unsigned int* res) { *res = static_cast<unsigned int>(strtoul(getString(column), &verify_bit, 10)); }

    template<typename T>
    inline void get(T column, bool* res) { *res = static_cast<bool>(strtol(getString(column), &verify_bit, 10));  }

    template <typename L>
    inline L get(unsigned int column) {
        L res;
        get(column, &res);
        return res;
    }

    template <typename L>
    inline L get(const char* column) {
        L res;
        get(column, &res);
        return res;
    }

    bool isNull() { return !*verify_bit; }

    bool status() {
        return status_bit == PGRES_TUPLES_OK || status_bit == PGRES_COMMAND_OK || status_bit == PGRES_EMPTY_QUERY;
    }

    bool empty() { return status_bit == PGRES_EMPTY_QUERY; }

    const char* getError() { return PQresultErrorMessage(res); }

    unsigned int lenght() { return rows; }

    json::object getJson() {
        json::object json_result;
        for (unsigned int i = 0, cols = PQnfields(res); i < cols; i++) {
            json_result.insert_or_assign(PQfname(res, i), getString(i));
        }
        return json_result;
    }

    json::array getJsonData() {
        unsigned int idx = index;
        json::array json_results;
        for (unsigned int l = 0; l < rows; l++) {
            json_results.push_back(getJson());
            next();
        }
        index = idx;
        return json_results;
    }

    bool next() {
        if (index < rows - 1) index++;
        else return false;
        return true;
    }

    void begin() {
        index = 0;
    }

    void end() {
        index = rows - 1;
    }
};

}
}

#endif // INCLUDE_DATABASE_DATA_H