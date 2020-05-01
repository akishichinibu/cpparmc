#ifndef CPPARMC_STREAM_BASE_HPP
#define CPPARMC_STREAM_BASE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/bit_operation.hpp"

namespace cpparmc::stream {

    class BaseStream {
    protected:
        std::int64_t _ch;
        std::uint64_t _buffer;
        std::uint8_t _buffer_size;

    public:
        void* _device;

        u_char input_width;
        u_char output_width;

        BaseStream(void* _device, u_char input_width, u_char output_width) :
                _device(_device),
                input_width(input_width),
                output_width(output_width),
                _ch(0U),
                _buffer(0U),
                _buffer_size(0U) {};

        virtual void reset() = 0;
        virtual std::int64_t get() = 0;
        virtual std::pair<std::uint8_t, std::uint64_t> receive() = 0;
    };

    template<typename InputDevice>
    class InputStream : public BaseStream {
        static_assert(std::is_base_of_v<BaseStream, InputDevice>);

    protected:
        bool _eof;

    public:
        InputDevice& device;

        InputStream(InputDevice& device, u_char input_width, u_char output_width) :
                BaseStream(nullptr, input_width, output_width),
                device(device),
                _eof(false) {
            this->_device = &this->device;
        };

        void reset() override {
            this->_eof = false;
            this->device.reset();
        };

        std::int64_t get() final {
            std::uint8_t size;
            std::uint64_t buf;

            while (_buffer_size < output_width) {
                std::tie(size, buf) = this->receive();

                if (size == 0) {
                    break;
                } else {
                    _buffer = bits::append_bits(_buffer, buf, size);
                    _buffer_size += size;
                }
            }

            if (_buffer_size == 0) {
                this->_eof = true;
                return EOF;
            }

            std::tie(_ch, _buffer_size) = bits::pop_bits(_buffer, _buffer_size, this->output_width);
            return _ch;
        }

        template<typename T>
        std::size_t read(T& buf) {
            const auto char_size = sizeof(T);

            int i;
            for (i = 0; i < char_size / this->output_width; i++) {
                const auto ch = this->get();
                if (this->eof()) break;
                *(&buf + i) = ch;
            }

            return i - 0 + 1;
        }

        [[nodiscard]] bool eof() const { return this->_eof; }
    };
}


#endif //CPPARMC_STREAM_BASE_HPP
