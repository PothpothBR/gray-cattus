#ifndef INCLUDE_DATABASE_DATA_H
#define INCLUDE_DATABASE_DATA_H

#include <string.h>
#include "libpq-fe.h"

#include "common.h"

namespace database {
    class Data {
        PGresult* __res = nullptr;
        unsigned int __index = 0;
        unsigned int __len = 0;
        ExecStatusType __status = PGRES_NONFATAL_ERROR;

        char *__verify_byte = nullptr;

    public:
        Data(PGresult* result) { this->__res = result; }
        Data() {}
        ~Data() {
            PQclear(this->__res);
            if (this->__verify_byte) delete this->__verify_byte;
        }

        Data& operator=(PGresult* res) {
            PQclear(this->__res);
            this->__res = res;
            this->__status = PQresultStatus(res);
            this->__len = PQntuples(this->__res);
            return *this;
        }
        
        inline const char* getString(unsigned int column) { return PQgetvalue(this->__res, this->__index, column); }
        inline const char* getString(const char* column) {
            return PQgetvalue(this->__res, this->__index, PQfnumber(this->__res, column));

        }

        long int getLong(unsigned int column) { return strtol(getString(column), &this->__verify_byte, 10); }
        long int getLong(const char* column) { return strtol(getString(column), &this->__verify_byte, 10); }

        long long int getLLong(unsigned int column) { return strtoll(getString(column), &this->__verify_byte, 10); }
        long long int getLLong(const char* column) { return strtoll(getString(column), &this->__verify_byte, 10); }

        float getFloat(unsigned int column) { return strtof(getString(column), &this->__verify_byte); }
        float getFloat(const char* column) { return strtof(getString(column), &this->__verify_byte); }

        double getDouble(unsigned int column) { return strtod(getString(column), &this->__verify_byte); }
        double getDouble(const char* column) { return strtod(getString(column), &this->__verify_byte); }

        long double getLDouble(unsigned int column) { return strtold(getString(column), &this->__verify_byte); }
        long double getLDouble(const char* column) { return strtold(getString(column), &this->__verify_byte); }

        unsigned long int getULong(unsigned int column) { return strtoul(getString(column), &this->__verify_byte, 10); }
        unsigned long int getULong(const char* column) { return strtoul(getString(column), &this->__verify_byte, 10); }

        unsigned long long int getULLong(unsigned int column) { return strtoull(getString(column), &this->__verify_byte, 10); }
        unsigned long long int getULLong(const char* column) { return strtoull(getString(column), &this->__verify_byte, 10); }

        int getInt(unsigned int column) { return getLong(column); }
        int getInt(const char* column) { return getLong(column); }

        unsigned int getUInt(unsigned int column) { return getULong(column); }
        unsigned int getUInt(const char* column) { return getULong(column); }

        bool getBool(unsigned int column) { return getLong(column); }
        bool getBool(const char* column) { return getLong(column); }

        bool getNull() { return !*this->__verify_byte; }

        bool status() {
            return this->__status == PGRES_TUPLES_OK || this->__status == PGRES_COMMAND_OK || this->__status == PGRES_EMPTY_QUERY;
        }

        bool empty() { return this->__status == PGRES_EMPTY_QUERY; }

        const char* getError() { return PQresultErrorMessage(this->__res); }

        unsigned int lenght() { return this->__len; }

        bool next() {
            if (this->__index < this->__len-1) this->__index++;
            else return false;
            return true;
        }
    };
}

#endif // INCLUDE_DATABASE_DATA_H