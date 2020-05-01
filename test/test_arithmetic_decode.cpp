#include <filesystem>
#include "armc.h"

namespace cf = cpparmc::file;


int main() {
    cpparmc::armc_params _compress_params = {
            8,
    };
    cpparmc::armc_coder_params _common_params = {
            64 * 1024,
    };

    std::string fn_str;
    std::cin >> fn_str;

    std::filesystem::path fn(fn_str);

    cpparmc::InputFileDevice<> inpf{fn.string()};

    std::filesystem::path out_fn(fn);
    out_fn.replace_extension(fn.extension().string() + ".decompress");

    cf::ARMCFileReader armc_file(fn, _compress_params, _common_params);

    armc_file.open();
    START_TIMER(READ_ARMC_FILE);
    const auto buf = armc_file.read();
    END_TIMER_AND_OUTPUT_MS(READ_ARMC_FILE);
    armc_file.close();

    cpparmc::OutputFileDevice outf(out_fn);
    outf.write(buf.c_str(), buf.size());

    return 0;
}
