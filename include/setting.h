#ifndef CPPARMC_SETTING_H
#define CPPARMC_SETTING_H

#include "__impl/compile_base.h"

namespace cpparmc {
    struct armc_params {
        u_char symbol_bit;
    };

    struct armc_coder_params {
        std::size_t pkg_size;
    };
}

#endif //CPPARMC_SETTING_H
