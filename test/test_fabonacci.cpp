//#include <iostream>
//#include <sstream>
//
//#include "__impl/stream/bit_stream.hpp"
//#include "__impl/stream/fabonacci/encode.hpp"
//
//
//int main() {
//    cpparmc::InputFileDevice inpf { "./test.txt" };
//
//    cpparmc::BitStream<cpparmc::InputFileDevice> s(inpf, 4U);
//
//    cpparmc::FabonacciEncode<cpparmc::BitStream<cpparmc::InputFileDevice>> s1(s, 8U);
//
//    std::basic_stringstream<u_char> outf;
//
//    inpf.open();
//
//    while (true) {
//        const auto ch = s1.get();
//        if (s1.eof()) break;
//        outf.put(ch);
//    }
//
//    std::cout << inpf.tell() << std::endl;
//    std::cout << outf.str().size() << std::endl;
//
//    inpf.close();
//    return 0;
//}
