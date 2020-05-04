#ifndef CPPARMC_STREAM_BASE_HPP
#define CPPARMC_STREAM_BASE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/bit_operation.hpp"

namespace cpparmc::stream {
    typedef std::uint64_t CommonSymbolType;
    typedef std::pair<std::int16_t, CommonSymbolType> StreamStatus;

    class BaseStream {

    protected:
        void* _i_device_{};

    public:
        std::uint8_t input_width{};
        std::uint8_t output_width{};

        BaseStream() = default;

        BaseStream(void* _device, std::uint8_t input_width, std::uint8_t output_width):
                _i_device_(_device),
                input_width(input_width),
                output_width(output_width) {};

        virtual void reset() = 0;

        virtual std::int64_t get() = 0;

        virtual StreamStatus receive() = 0;
    };

    template<typename InputDevice>
    class Stream: public BaseStream {
        static_assert(std::is_base_of_v<BaseStream, InputDevice>);

    protected:
        std::int64_t _i_ch_{};
        std::uint64_t _i_buffer_{};
        int _i_buffer_size_{};
        bool _eof{};
        std::uint64_t output_count{};
        std::uint64_t _symbol_limit{};

        bool greedy{};

    public:
        InputDevice& device;

        Stream() = default;

        Stream(InputDevice& device, std::uint8_t input_width, std::uint8_t output_width, bool greedy=true):
                BaseStream(nullptr, input_width, output_width),
                _i_ch_(0),
                _i_buffer_(0),
                _i_buffer_size_(0),
                _eof(false),
                output_count(0),
                _symbol_limit(1U << output_width),
                device(device),
                greedy(greedy) {
            this->_i_device_ = &this->device;
        };

        void reset() override {
            this->_eof = false;
            this->output_count = 0;
            this->device.reset();
        };

        std::int64_t get() final {
            while (_i_buffer_size_ < output_width) {
                std::int16_t size;
                std::uint64_t buf;
                std::tie(size, buf) = this->receive();
                assert((size < 0) || ((size < 64) && (0LU <= buf)));
//                DEBUG_PRINT("width: {:d}~{:d} size: {:d} buffer: {:d} buf=[{:B}] buf=[{:B}]", this->input_width, this->output_width, size, _i_buffer_size_, buf, _i_buffer_);
                if (size < 0) {
//                    DEBUG_PRINT("EOF@!@ width: {:d}~{:d} size: {:d} buffer: {:d} buf=[{:B}] buf=[{:B}]", this->input_width, this->output_width, size, _i_buffer_size_, buf, _i_buffer_);

                    if ((!greedy) && (_i_buffer_size_ < this->input_width)) _i_buffer_size_ = 0;
                    break;
                }

                bits::concat_bits(_i_buffer_, buf, size);
                _i_buffer_size_ += size;
                assert((0 <= _i_buffer_size_) && (_i_buffer_size_ < 64));
            }

            if (_i_buffer_size_ == 0) {
                this->_eof = true;
                return EOF;
            }

            std::tie(_i_ch_, _i_buffer_size_) =
                    bits::pop_bits(_i_buffer_, _i_buffer_size_, this->output_width);
            assert((0 <= _i_buffer_size_) && (_i_buffer_size_ < 64));
            output_count += 1;

            assert((0 <= _i_ch_) && (_i_ch_ < _symbol_limit));
            return _i_ch_;
        }

        template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> = true>
        std::size_t read(T& buf) {
            const auto bit_size = sizeof(T) << 3U;
            buf = 0;

            int i;
            for (i = 0; i < bit_size / this->output_width; i++) {
                const std::uint64_t ch = this->get();
                if (this->eof()) break;
                bits::concat_bits(buf, ch, this->input_width);
            }

            return i - 0;
        }

        [[nodiscard]] bool eof() const { return this->_eof; }
    };
}

#endif //CPPARMC_STREAM_BASE_HPP
