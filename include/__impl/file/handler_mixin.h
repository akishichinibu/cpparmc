#ifndef CPPARMC_HANDLER_MIXIN_H
#define CPPARMC_HANDLER_MIXIN_H

#include <string>
#include "setting.h"


namespace cpparmc {

    using namespace setting;

    class ARMCFileMixin {
    protected:
        bool has_open = false;

        std::string fn;

        armc_params params {};
        armc_coder_params coder_params {};

        std::uint64_t total_symbol = 0;

        ARMCFileMixin(const std::string& fn,
                      const armc_params& params,
                      const armc_coder_params& coder_params);

        ARMCFileMixin(const std::string& fn);

        struct ARMCFileHeader {
            std::uint8_t _magic_1, _magic_2;
            std::uint8_t ver_algo;
            std::uint8_t platform;
            std::uint8_t flag;
            std::uint64_t mtime;
            std::uint32_t header_crc;
        };

        struct ARMCPackageHeader {
            std::uint64_t package_length;
            std::uint8_t symbol_bit;
            std::uint64_t uncompress_length;
            std::uint32_t pkg_crc;
        };

        virtual void close() = 0;
    };

    ARMCFileMixin::ARMCFileMixin(const std::string& fn,
                                 const armc_params& params,
                                 const armc_coder_params& coder_params):
            fn(fn),
            params(params),
            coder_params(coder_params),
            total_symbol(1U << params.symbol_bit) {}


    ARMCFileMixin::ARMCFileMixin(const std::string& fn): fn(fn) {}
}

#endif //CPPARMC_HANDLER_MIXIN_H
