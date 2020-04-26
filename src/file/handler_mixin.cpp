#include "__impl/file/handler_mixin.h"


namespace cpparmc {

    ARMCFileMixin::ARMCFileMixin(const std::string& fn,
                                 const armc_params& params,
                                 const armc_coder_params& coder_params) :
            fn(fn),
            params(params),
            coder_params(coder_params),
            total_symbol(1U << params.symbol_bit) {}
}
