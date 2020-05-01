#include <iostream>
#include <sstream>
#include <filesystem>

#include "armc.h"

namespace cs = cpparmc::stream;


int main() {
    std::string fn_str;
    std::cin >> fn_str;

    cs::InputFileDevice inpf { fn_str };

    cs::BitStream<cs::InputFileDevice<>> s(inpf, 8U);

    cs::ConditionalFibonacciEncode<cs::BitStream<cs::InputFileDevice<>>> s1(s, 8U);

    std::basic_stringstream<u_char> out_buffer;

    START_TIMER(WRITE_FIBONACCI_FILE);
    while (true) {
        const auto ch = s1.get();
        if (s1.eof()) break;
        out_buffer.put(ch);
    }
    END_TIMER_AND_OUTPUT_MS(WRITE_FIBONACCI_FILE);

    std::cout << inpf.tell() << std::endl;
    std::cout << out_buffer.str().size() << std::endl;

    std::filesystem::path out_fn(fn_str);
    out_fn.replace_extension(out_fn.extension().string() + ".fib");

    cpparmc::OutputFileDevice outf(out_fn.string());
    const auto content = out_buffer.str();
    outf.write(content.c_str(), content.size());
    outf.flush();

    return 0;
}
