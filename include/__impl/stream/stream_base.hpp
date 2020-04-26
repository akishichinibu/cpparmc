#ifndef CPPARMC_STREAM_BASE_HPP
#define CPPARMC_STREAM_BASE_HPP

#include "__impl/compile_base.h"


template<typename Device>
class BaseStream {
public:
    Device& device;

    u_char input_width;
    u_char output_width;

    BaseStream(Device& device, u_char input_width, u_char output_width):
    device(device), input_width(input_width), output_width(output_width) {};
};


template<typename InputDevice>
class InputStream: public BaseStream<InputDevice> {

public:
    bool _eof;

    InputStream(InputDevice& device, u_char input_width, u_char output_width) :
            BaseStream<InputDevice>(device, input_width, output_width), _eof(false) {};

    virtual u_int32_t get() = 0;

    [[nodiscard]] bool eof() const {
        return _eof;
    }
};

#endif //CPPARMC_STREAM_BASE_HPP
