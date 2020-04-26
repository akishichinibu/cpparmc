#ifndef CPPARMC_ARMC_PACKAGE_H
#define CPPARMC_ARMC_PACKAGE_H

#include <memory>
#include <vector>


namespace cpparmc {
    class armc_package {
        //+-----------------+-----------------+-----------------+-----------------+
        //|                         PKG LENGTH (exclude self)                     |
        //+-----------------+-----------------+-----------------+-----------------+
        //|   symbol_bit    |   counter_bit   |                                   |
        //+-----------------+-----------------+-----------------+-----------------+
        //|                        UNCOMPRESSED LENGTH                            |
        //+-----------------------------------------------------------------------+
        //|                                  CRC                                  |
        //+=======================================================================+
        //|                              ...body...                               |
        //+=======================================================================+
    public:
        std::size_t total_symbols = 0;
        std::size_t hist_table_length = 0;
        std::size_t reshift_table_length = 0;

        std::size_t uncompressed_length = 0;
        std::size_t compressed_length = 0;

        u_char symbol_bit = 0;
        u_char counter_bit = 0;

        std::size_t CRC32 = 0;
        std::size_t pkg_length = 0;

        std::unique_ptr<std::size_t[]> hist_table = nullptr;
        std::unique_ptr<std::size_t[]> reshift_table = nullptr;
        std::unique_ptr<u_char[]> body = nullptr;

    public:

        inline armc_package() = default;

        inline armc_package(std::size_t uncompressed_length,
                            std::size_t compressed_length,
                            std::size_t symbol_bit,
                            std::size_t counter_bit,
                            const std::vector<u_char>& _body,
                            const std::size_t* hist_table,
                            const std::size_t* reshift_table) :
                total_symbols(1U << symbol_bit),
                hist_table_length((total_symbols * counter_bit) >> 3U),
                reshift_table_length((total_symbols * symbol_bit) >> 3U),
                uncompressed_length(uncompressed_length),
                compressed_length(compressed_length),
                symbol_bit(symbol_bit),
                counter_bit(counter_bit) {

            this->body = std::make_unique<u_char[]>(compressed_length);
            std::copy(_body.cbegin(), _body.cend(), this->body.get());

            this->CRC32 = 0;

            for (auto i = 0; i < total_symbols; i++) this->hist_table.get()[i] = hist_table[i];
            for (auto i = 0; i < total_symbols; i++) this->reshift_table.get()[i] = reshift_table[i];

            this->pkg_length = 1 // symbol_bit
                               + 1 // counter_bit
                               + 1 // uncompressed length
                               + 1 // CRC32
                               + hist_table_length // hist table
                               + reshift_table_length  // reshift table
                               + compressed_length;  // body
        }
    };
}

#endif //CPPARMC_ARMC_PACKAGE_H
