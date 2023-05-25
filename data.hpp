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
    PGresult* res = nullptr;
    unsigned int index = 0;
    unsigned int rows = 0;
    ExecStatusType status_bit = PGRES_NONFATAL_ERROR;

    char* verify_bit = nullptr;

public:
    Data(PGresult* result) { res = result; }
    Data() {}
    ~Data() {
        PQclear(res);
        if (this->verify_bit) delete this->verify_bit;
    }

    Data& operator=(PGresult* result) {
        PQclear(res);
        res = result;
        status_bit = PQresultStatus(res);
        rows = PQntuples(res);
        return *this;
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

    long int getLong(unsigned int column) { return strtol(getString(column), &verify_bit, 10); }
    long int getLong(const char* column) { return strtol(getString(column), &verify_bit, 10); }

    long long int getLLong(unsigned int column) { return strtoll(getString(column), &verify_bit, 10); }
    long long int getLLong(const char* column) { return strtoll(getString(column), &verify_bit, 10); }

    float getFloat(unsigned int column) { return strtof(getString(column), &verify_bit); }
    float getFloat(const char* column) { return strtof(getString(column), &verify_bit); }

    double getDouble(unsigned int column) { return strtod(getString(column), &verify_bit); }
    double getDouble(const char* column) { return strtod(getString(column), &verify_bit); }

    long double getLDouble(unsigned int column) { return strtold(getString(column), &verify_bit); }
    long double getLDouble(const char* column) { return strtold(getString(column), &verify_bit); }

    unsigned long int getULong(unsigned int column) { return strtoul(getString(column), &verify_bit, 10); }
    unsigned long int getULong(const char* column) { return strtoul(getString(column), &verify_bit, 10); }

    unsigned long long int getULLong(unsigned int column) { return strtoull(getString(column), &verify_bit, 10); }
    unsigned long long int getULLong(const char* column) { return strtoull(getString(column), &verify_bit, 10); }

    int getInt(unsigned int column) { return getLong(column); }
    int getInt(const char* column) { return getLong(column); }

    unsigned int getUInt(unsigned int column) { return getULong(column); }
    unsigned int getUInt(const char* column) { return getULong(column); }

    bool getBool(unsigned int column) { return getLong(column); }
    bool getBool(const char* column) { return getLong(column); }

    bool getNull() { return !*verify_bit; }

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

    bool begin() {
        index = 0;
    }

    bool end() {
        index = rows - 1;
    }
};

}
}

#endif // INCLUDE_DATABASE_DATA_H