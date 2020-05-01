#ifndef CPPARMC_STREAM_BASE_HPP
#define CPPARMC_STREAM_BASE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/bit_operation.hpp"

namespace cpparmc::stream {

    typedef std::pair<std::int16_t, std::uint64_t> StreamStatus;

    class BaseStream {

    protected:
        void* _i_device_;

    public:
        std::uint8_t input_width;
        std::uint8_t output_width;

        BaseStream(void* _device, u_char input_width, u_char output_width):
                _i_device_(_device),
                input_width(input_width),
                output_width(output_width) {};

        virtual void reset() = 0;

        virtual std::int64_t get() = 0;

        virtual StreamStatus receive() = 0;
    };

    template<typename InputDevice>
    class InputStream: public BaseStream {
        static_assert(std::is_base_of_v<BaseStream, InputDevice>);

    protected:
        std::int64_t _i_ch_;
        std::uint64_t _i_buffer_;
        int _i_buffer_size_;
        bool _eof;
        std::uint64_t output_count;

    public:
        InputDevice& device;

        InputStream(InputDevice& device, u_char input_width, u_char output_width):
                BaseStream(nullptr, input_width, output_width),
                _i_ch_(0),
                _i_buffer_(0),
                _i_buffer_size_(0),
                _eof(false),
                output_count(0),
                device(device) {
            this->_i_device_ = &this->device;
        };

        void reset() override {
            this->_eof = false;
            this->output_count = 0;
            this->device.reset();
        };

        std::int64_t get() final {
            int size;
            std::uint64_t buf;

            while (_i_buffer_size_ < output_width) {
                assert((0 <= _i_buffer_size_) && (_i_buffer_size_ < 64));
                std::tie(size, buf) = this->receive();
                assert(size < 64);
                if (size < 0) break;

                bits::concat_bits(_i_buffer_, buf, size);
                _i_buffer_size_ += size;
                assert((0 <= _i_buffer_size_) && (_i_buffer_size_ < 64));
            }

            if (_i_buffer_size_ == 0) {
                this->_eof = true;
                return EOF;
            }

            const auto _o_size = _i_buffer_size_;

            std::tie(_i_ch_, _i_buffer_size_) =
                    bits::pop_bits(_i_buffer_, _i_buffer_size_, this->output_width);
            assert((0 <= _i_buffer_size_) && (_i_buffer_size_ < 64));
            output_count += 1;
            return _i_ch_;
        }

        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        std::size_t read(T& buf) {
            const auto bit_size = sizeof(T) << 3U;
            buf = 0;

            int i;
            for (i = 0; i < bit_size / this->output_width; i++) {
                const auto ch = this->get();
                if (this->eof()) break;
                bits::concat_bits(buf, ch, this->input_width);
            }

            return i - 0;
        }

        [[nodiscard]] bool eof() const { return this->_eof; }
    };
}

#endif //CPPARMC_STREAM_BASE_HPP
