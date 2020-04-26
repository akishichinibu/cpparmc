#include <iostream>
#include "__impl/stream/bit_stream.hpp"
#include "__impl/stream/fabonacci/encode.hpp"


int main() {
    cpparmc::InputFileDevice inpf { "./test2" };

    cpparmc::BitStream<cpparmc::InputFileDevice> s(inpf, 7);



    while (!s.eof()) {
        std::cout << s.get() << std::endl;
    }

    return 0;
}
