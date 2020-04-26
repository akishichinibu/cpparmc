#ifndef CPPARMC_FABONACCI_DECODE_H
#define CPPARMC_FABONACCI_DECODE_H

namespace cpparmc {
    template<typename Device>
    class FabonacciDecode : public InputStream<Device> {

    private:
        bool ch;
        u_int64_t buffer;
        u_char buffer_length;

        FabonacciDecode(Device& device, u_char output_width) :
                InputStream<Device>(device, 1U, output_width),
                ch(0U),
                buffer(0U),
                buffer_length(0U) {}

        u_int32_t get() final {
            while (true) {
                ch = this->device.get();
                if (this->device.eof()) {
                    if (buffer_length == 0) {
                        this->_eof = false;
                        return EOF;
                    } else {
                        buffer_length = 0;
                        return buffer;
                    }
                }

                buffer = (buffer << this->input_width) | ch;
                buffer_length += this->input_width;

                if (buffer & 0b11 != 0U) {
                    buffer_length = 0;
                    return buffer;
                }
            }
        }
    };
}

#endif //CPPARMC_FABONACCI_DECODE_H
