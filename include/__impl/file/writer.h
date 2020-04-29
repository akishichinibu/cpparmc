#ifndef CPPARMC_WRITER_H
#define CPPARMC_WRITER_H

#include <memory>

#include "handler_mixin.h"
#include "__impl/darray.hpp"
#include "__impl/stream/bit_stream.hpp"


namespace cpparmc {
    using namespace stream;

    namespace file {

        class ARMCFileWriter : public ARMCFileMixin {
            OutputFileDevice output_stream;

            void write_package(u_int64_t uncompress_length, std::basic_string<u_char>&& s);

        public:
            ARMCFileWriter(const std::string& fn,
                           const armc_params& params,
                           const armc_coder_params& coder_params);

            ARMCFileWriter& open();

            void write_header();

            void write(InputFileDevice& s);

            void close() final;
        };
    }
}

#endif //CPPARMC_WRITER_H
