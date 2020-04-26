#ifndef CPPARMC_READER_H
#define CPPARMC_READER_H

#include "setting.h"
#include "handler_mixin.h"
#include "__impl/stream/file_device.hpp"
#include "__impl/stream/bit_stream.hpp"


namespace cpparmc {

    class ARMCFileReader: public ARMCFileMixin {

        InputFileDevice input_stream;

        u_int read_package_head();

    public:
        ARMCFileReader(const std::string& fn,
                       const armc_params& params,
                       const armc_coder_params& coder_params);

        void open();

        std::basic_string<u_char> read();
    };

}

#endif //CPPARMC_READER_H
