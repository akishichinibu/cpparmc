#ifndef CPPARMC_READER_H
#define CPPARMC_READER_H

#include "setting.h"
#include "handler_mixin.h"
#include "__impl/stream/file_device.hpp"
#include "__impl/stream/bit_stream.hpp"


namespace cpparmc {

    using namespace stream;

    namespace file {

        class ARMCFileReader : public ARMCFileMixin {

            stream::InputFileDevice input_stream;

            u_int64_t read_package_head();

        public:
            ARMCFileReader(const std::string& fn,
                           const armc_params& params,
                           const armc_coder_params& coder_params);

            ARMCFileReader& open();

            std::basic_string<u_char> read();

            void close() final;
        };
    }
}

#endif //CPPARMC_READER_H
