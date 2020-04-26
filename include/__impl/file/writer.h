#ifndef CPPARMC_WRITER_H
#define CPPARMC_WRITER_H

#include <memory>

#include "handler_mixin.h"
#include "__impl/darray.hpp"
#include "__impl/stream/bit_stream.hpp"



namespace cpparmc {

    class ARMCFileWriter: public ARMCFileMixin {
        OutputFileDevice output_stream;

        void write_package(u_int64_t uncompress_length,
                std::pair<u_int64_t, u_int64_t> final_range,
                std::basic_string<u_char>&& s);

    public:
        ARMCFileWriter(const std::string& fn,
                       const armc_params& params,
                       const armc_coder_params& coder_params);

        void open();

        void write_header();

        void write(InputFileDevice& s);

        void close();
    };
}

#endif //CPPARMC_WRITER_H
