#ifndef CPPARMC_ARMC_FILE_H
#define CPPARMC_ARMC_FILE_H

#include <ctime>
#include <list>
#include "armc_package.h"


namespace cpparmc {

    class armc_file {
        //+-----------------+-----------------+-----------------+-----------------+
        //|               MAGIC               |   VER  |  ALGO  |   TAIL MAGIC    |
        //+-----------------+-----------------+-----------------+-----------------+
        //|    PLATFORM     |      FLAG       |               DIGEST              |
        //+-----------------+-----------------+-----------------+-----------------+
        //|                                 MTIME                                 |
        //+-----------------+-----------------+-----------------+-----------------+
        //|                               HEADER CRC                              |
        //+=================+=================+=================+=================+
        //|                              ...pkg_1...                              |
        //+=================+=================+=================+=================+
        //|                              ...pkg_2...                              |
        //+=================+=================+=================+=================+
        //|                                  ...                                  |
        //+=================+=================+=================+=================+
        //|                              ...pkg_n...                              |
        //+=================+=================+=================+=================+

    public:
        u_char magic[2];
        u_char ver_algo;
        u_char platform;
        u_char flag;
        uint mtime;
        uint uncompressed_length = 0U;
        uint package_nums = 0U;
        uint CRC32;

        std::list<armc_package> packages;
        ushort digest;
        u_char tail_magic;

        inline explicit armc_file() {
            this->magic[0] = 0;
            this->magic[1] = 1;

            this->ver_algo = 0b00100010;
            this->platform = 0b00100010;
            this->flag = 0b00000000;

            this->mtime = std::time(nullptr);
            this->CRC32 = 0U;

            this->digest = 0;
            this->tail_magic = 0;
        };
    };
}

#endif //CPPARMC_ARMC_FILE_H
