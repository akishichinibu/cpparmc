#ifndef CPPARMC_STREAM_BASE_HPP
#define CPPARMC_STREAM_BASE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/bit_operation.hpp"

namespace cpparmc::stream {

    class BaseStream {

    protected:
        void* _i_device_;

    public:
        std::uint8_t input_width;
        std::uint8_t output_width;

        BaseStream(void* _device, u_char input_width, u_char output_width) :
                _i_device_(_device),
                input_width(input_width),
                output_width(output_width) {};

        virtual void reset() = 0;
        virtual std::int64_t get() = 0;
        virtual std::pair<std::uint8_t, std::uint64_t> receive() = 0;
    };

    template<typename InputDevice>
    class InputStream : public BaseStream {
        static_assert(std::is_base_of_v<BaseStream, InputDevice>);

        std::int64_t _i_ch_;
        std::uint64_t _i_buffer_;
        std::uint8_t _i_buffer_size_;

    protected:
        bool _eof;

    public:
        InputDevice& device;

        InputStream(InputDevice& device, u_char input_width, u_char output_width) :
                BaseStream(nullptr, input_width, output_width),
                _i_ch_(0U),
                _i_buffer_(0U),
                _i_buffer_size_(0U),
                _eof(false),
                device(device) {
            this->_i_device_ = &this->device;
        };

        void reset() override {
            this->_eof = false;
            this->device.reset();
        };

        std::int64_t get() final {
            std::uint8_t size;
            std::uint64_t buf;

            while (_i_buffer_size_ < output_width) {
                std::tie(size, buf) = this->receive();
                if (size == 0) { break; }

                bits::concat_bits(_i_buffer_, buf, size);
                _i_buffer_size_ += size;
            }

            if (_i_buffer_size_ == 0) {
                this->_eof = true;
                return EOF;
            }

            std::tie(_i_ch_, _i_buffer_size_) =
                    bits::pop_bits(_i_buffer_, _i_buffer_size_, this->output_width);
            return _i_ch_;
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
