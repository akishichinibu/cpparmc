#ifndef CPPARMC_FABONACCI_DECODE_H
#define CPPARMC_FABONACCI_DECODE_H

namespace cpparmc {

    template<typename Device>
    class FabonacciDecode : public InputStream<Device> {

    private:
        bool ch;
        std::uint64_t bit_buffer;
        u_char bit_buffer_size;

    public:
        FabonacciDecode(Device& device, u_char output_width);
        StreamStatus receive() final;
    };

    template<typename Device>
    FabonacciDecode<Device>::FabonacciDecode(Device& device, u_char output_width) :
            InputStream<Device>(device, 1, output_width),
            ch(false),
            bit_buffer(0),
            bit_buffer_size(0) {};

    template<typename Device>
    auto FabonacciDecode<Device>::receive() -> StreamStatus {
        ch = this->device.get();

        if (this->device.eof()) {
            this->_eof = true;
            return { bit_buffer_size, bit_buffer };
        }

        bit_buffer = (bit_buffer << this->input_width) | ch;
        bit_buffer_size += this->input_width;

        if ((bit_buffer & 0b11U) != 0U) {
            const auto temp = bit_buffer_size;
            return { temp, bit_buffer };
        }
    }
}

#endif //CPPARMC_FABONACCI_DECODE_H
