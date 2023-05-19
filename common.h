#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore>

using namespace std;

#ifdef __debug_all__
#define __debug__
#define __debug_info__
#define __debug_prime__
#define __debug_datainfo__
#define __debug_warn__
#define __debug_badwarn__
#define __debug_error__
#endif

#ifdef __debug__

#ifdef __debug_info__
	void info(const char* _Format, ...) {
        char* _RFormat = new char[strlen(_Format)+11];
        sprintf(_RFormat, "[info] %s\n", _Format);
        va_list _ArgList;
        va_start(_ArgList, _Format);
        vfprintf(stderr, _RFormat, _ArgList);
        va_end(_ArgList);
        delete[] _RFormat;
	}
#else
    #define info(_)
#endif

#ifdef __debug_prime__
    void prime(const char* type, const char* _Format, ...) {
        char* _RFormat;
        if (_Format) { _RFormat = new char[strlen(_Format) + 40]; }
        else         { _RFormat = new char[40]; }

        
        sprintf(_RFormat, "[%s] %s\n", type, _Format);
        va_list _ArgList;
        va_start(_ArgList, _Format);
        vfprintf(stderr, _RFormat, _ArgList);
        va_end(_ArgList);
        delete[] _RFormat;
    }
#else
    #define prime(_)
#endif

#ifdef __debug_datainfo__
    void dataInfo(const char* _Format, ...) {
        if (_Format) { //TODO: adicionar verificação em todos os printers
            char* _RFormat = new char[strlen(_Format) + 35];
            sprintf(_RFormat, "\n---- inicio ----\n%s\n---- fim ----\n\n", _Format);
            va_list _ArgList;
            va_start(_ArgList, _Format);
            vfprintf(stderr, _RFormat, _ArgList);
            va_end(_ArgList);
            delete[] _RFormat;
        }
        else {
            fprintf(stderr, "\n----inicio----\n\n----fim----\n\n");
        }
    }
#else
    #define dataInfo(_)
#endif

#ifdef __debug_warn__
    void warn(const char* _Format, ...) {
        char* _RFormat = new char[strlen(_Format) + 13];
        sprintf(_RFormat, "[alerta] %s\n", _Format);
        va_list _ArgList;
        va_start(_ArgList, _Format);
        vfprintf(stderr, _RFormat, _ArgList);
        va_end(_ArgList);
        delete[] _RFormat;
    }
#else
    #define warn(_)
#endif

#ifdef __debug_badwarn__
void badWarn(const char* _Format, ...) {
    char* _RFormat = new char[strlen(_Format) + 11 + 2048];
    sprintf(_RFormat, "[alerta] %s\n", _Format);
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vfprintf(stderr, _RFormat, _ArgList);
    vsprintf(_RFormat, _Format, _ArgList);
    va_end(_ArgList);
    throw _RFormat;
}
#else
void badWarn(const char* _Format, ...) {
    char* _RFormat = new char[strlen(_Format) + 11 + 2048];
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(_RFormat, _Format, _ArgList);
    va_end(_ArgList);
    throw _RFormat;
}
#endif

#ifdef __debug_error__
//!!! a mensagem de erro gerada deve ser deletada (delete[]) apos o uso
    void error(const char* _Format, ...) {
        char* _RFormat = new char[strlen(_Format) + 11 + 4086];
        sprintf(_RFormat, "[erro] %s\n", _Format);
        va_list _ArgList;
        va_start(_ArgList, _Format);
        vfprintf(stderr, _RFormat, _ArgList);
        vsprintf(_RFormat, _Format, _ArgList);
        va_end(_ArgList);
        throw _RFormat;
    }
#else
    void error(const char* _Format, ...) {
        char* _RFormat = new char[strlen(_Format) + 11 + 4086];
        va_list _ArgList;
        va_start(_ArgList, _Format);
        vsprintf(_RFormat, _Format, _ArgList);
        va_end(_ArgList);
        throw _RFormat;
    }
#endif

#else
#define info(...)
#define prime(...)
#define dataInfo(...)
#define warn(...)
#define badWarn(...)
void error(const char* _Format, ...) {
    char* _RFormat = new char[strlen(_Format) + 11 + 4086];
    va_list _ArgList;
    va_start(_ArgList, _Format);
    vsprintf(_RFormat, _Format, _ArgList);
    va_end(_ArgList);
    throw _RFormat;
}
#endif

#endif // INCLUDE_COMMON_H