#ifndef CPPARMC_HANDLER_MIXIN_H
#define CPPARMC_HANDLER_MIXIN_H

#include <string>
#include "setting.h"


namespace cpparmc {

    class ARMCFileMixin {
    protected:
        bool has_open = false;

        std::string fn;

        armc_params params;
        armc_coder_params coder_params;

        std::size_t total_symbol;

        ARMCFileMixin(const std::string& fn,
                      const armc_params& params,
                      const armc_coder_params& coder_params);

        struct ARMCFileHeader {
            u_int8_t _magic_1, _magic_2;
            u_int8_t ver_algo;
            u_int8_t platform;
            u_int8_t flag;
            u_int64_t mtime;
            u_int32_t header_crc;
        };

        struct ARMCPackageHeader {
            u_int64_t package_length;
            u_char symbol_bit;
            u_int64_t uncompress_length;
            u_int32_t pkg_crc;
        };
    };
}

#endif //CPPARMC_HANDLER_MIXIN_H
