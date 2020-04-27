#include <iostream>
#include "__impl/stream/bit_stream.hpp"
#include "__impl/stream/bwtrle/encode.hpp"
#include "__impl/stream/bwtrle/decode.hpp"

namespace cs = cpparmc::stream;


int main() {
    std::string fn;
    std::cin >> fn;

    constexpr auto block_size = 16 * 1024;

    cs::InputFileDevice inpf { fn };
    cs::BitStream<cs::InputFileDevice> s1(inpf, 8U);
    cs::BWTRLEEncode<cs::BitStream<cs::InputFileDevice>, u_int64_t> s2(s1, block_size);
    cs::OutputFileDevice outf(fn + ".bwtrle");

    while (true) {
        const u_char ch = s2.get();
        if (s2.eof()) break;
        outf.put(ch);
    }

    outf.flush();

    cs::InputFileDevice inpf2 { fn + ".bwtrle" };
    cs::BitStream<cs::InputFileDevice> s3(inpf2, 8U);
    cs::BWTRLEDecode<cs::BitStream<cs::InputFileDevice>, u_int64_t> s4(s3, block_size);
    cs::OutputFileDevice outf2(fn + ".bwtrle.recover");

    while (true) {
        const u_char ch = s4.get();
        if (s4.eof()) break;
        outf2.put(ch);
    }
}
