#include "armc.h"
#include "__impl/stream/bit_stream.hpp"


int main() {
    cpparmc::armc_params _compress_params = {
            8,
    };
    cpparmc::armc_coder_params _common_params = {
            64 * 1024,
    };

    std::string fn;
    std::cin >> fn;

    cpparmc::InputFileDevice inpf { fn };
    inpf.open();

    cpparmc::ARMCFileWriter armc_file(fn + ".arithmetic", _compress_params, _common_params);
    armc_file.open();
    armc_file.write(inpf);
    armc_file.close();

    inpf.close();

    cpparmc::ARMCFileReader armc_file2(fn + ".arithmetic", _compress_params, _common_params);
    armc_file2.open();
    const auto ss = armc_file2.read();

    std::cout << " !!!" << ss.size() << std::endl;

    cpparmc::OutputFileDevice outf(fn + ".arithmetic.decompress");
    outf.open();

    for (auto c: ss) {
        outf.put(c);
    }

    outf.close();
    return 0;
}
