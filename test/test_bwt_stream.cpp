#include <iostream>
#include "armc.h"


namespace cs = cpparmc::stream;


int main() {
    std::string fn;
    std::cin >> fn;

    constexpr auto block_size = 128 * 1024;

    cs::InputFileDevice<> inpf{fn};
    cs::BWTEncode<cs::InputFileDevice<>> s2(inpf, block_size);
    cs::RLEEncode<cs::BWTEncode<cs::InputFileDevice<>>> s3(s2, 4);
    cs::BitStream<cs::RLEEncode<cs::BWTEncode<cs::InputFileDevice<>>>> s4(s3, s3.output_width - 4);

//    cs::BitStream<cs::BWTEncode<cs::InputFileDevice<>>> s4(s2, s2.output_width);
//    cs::RLEEncode<cs::InputFileDevice<>> s2(inpf, 4);
//    cs::BitStream<cs::RLEEncode<cs::InputFileDevice<>>> s4(s2, s2.output_width - 4);

    cs::OutputFileDevice outf(fn + ".brl");

    while (true) {
        const u_char ch = s4.get();
        if (s4.eof()) break;
        outf.put(ch);
    }

    outf.flush();

    cs::InputFileDevice<> inpf2{fn + ".brl"};

//    cs::BitStream<cs::InputFileDevice<>> u0(inpf2, inpf2.output_width);
    cs::BitStream<cs::InputFileDevice<>> u0(inpf2, inpf2.output_width + 4U);
//    cs::BWTDecode<cs::BitStream<cs::InputFileDevice<>>> u2(u0, block_size);
    cs::RLEDecode<cs::BitStream<cs::InputFileDevice<>>> u1(u0, 4);
    cs::BWTDecode<cs::RLEDecode<cs::BitStream<cs::InputFileDevice<>>>> u4(u1, block_size);

//    cs::BitStream<cs::InputFileDevice<>> u2(inpf2, inpf2.output_width + 4U);
//    cs::RLEDecode<cs::BitStream<cs::InputFileDevice<>>> u4(u2, 4);

    cs::OutputFileDevice outf2(fn + ".brl.recover");

    while (true) {
        const u_char ch = u4.get();
        if (u4.eof()) break;
        outf2.put(ch);
    }
}
