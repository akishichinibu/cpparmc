#ifndef CPPARMC_BWT_DECODE_HPP
#define CPPARMC_BWT_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename T>
    class BWTRLEDecode : public InputStream<Device> {
        typedef T size_type;
        constexpr static u_char m_width = std::numeric_limits<size_type>::digits;

        size_type buffer_size;
        size_type block_size;

        darray<u_char> buffer;
        darray<u_char> btw_buffer;
        size_type output_pos;

        darray<size_type> K;
        darray<size_type> P;
        darray<u_int32_t> C;

        size_type m0;

    public:
        BWTRLEDecode(Device& device, std::size_t block_size);

        u_int64_t get() final;
    };

    template<typename Device, typename T>
    BWTRLEDecode<Device, T>::BWTRLEDecode(Device& device, std::size_t block_size):
            InputStream<Device>(device, 8U, 8U),
            buffer_size(0U),
            block_size(block_size),
            buffer(this->block_size),
            btw_buffer(this->block_size),
            output_pos(0U),
            K(1U << this->input_width),
            P(1U << this->input_width),
            C(this->block_size),
            m0(0U) {
    }

    template<typename Device, typename T>
    u_int64_t BWTRLEDecode<Device, T>::get() {
        if (output_pos == buffer_size) {
            buffer_size = 0U;

            for (auto i = 0U; i < m_width; i++) {
                const auto ch = this->device.get();

                if (this->device.eof()) {
                    this->_eof = true;

                    if (i != 0U) {
                        spdlog::warn("Not complete. ");
                    }

                    return EOF;
                }

                m0 = bits::set_range(m0, ch, i << 3U, (i + 1U) << 3U);
            }

#ifdef CPPARMC_DEBUG_BWT_DECODER
            std::cout << "m0: " << m0 << std::endl;
#endif

            std::fill(K.begin(), K.end(), 0);

            std::fill(P.begin(), P.end(), block_size);

            std::fill(C.begin(), C.end(), 0);

            while (buffer_size < block_size) {
                const auto ch = this->device.get();
                if (this->device.eof()) break;
                buffer[buffer_size] = ch;
                buffer_size += 1;

                K[ch] += 1;
                P[ch] = std::min(P[ch], buffer_size);
            }

            if (buffer_size > 0U) {
                for (auto i = 0; i < buffer_size; i++) {
                    for (auto j = 0; j < i; j++) {
                        if (buffer[j] == buffer[i]) {
                            C[i] += 1;
                        }
                    }
                }

#ifdef CPPARMC_DEBUG_BWT_DECODER
                START_TIMER(REORDER_BWT_TABLE);
#endif

                btw_buffer[buffer_size - 1] = buffer[m0];

                for (auto i = 0; i < buffer_size - 1; i++) {
                    const auto t = (m0 + i) % buffer_size;
                    const auto ind = C[t] + P[btw_buffer[t]];
                    btw_buffer[ind] = buffer[t - 1];
                }

                for (auto i = 0; i < buffer_size; i++) buffer[i] = btw_buffer[i];

#ifdef CPPARMC_DEBUG_BWT_DECODER
                END_TIMER_AND_OUTPUT_MS(REORDER_BWT_TABLE);
#endif
            }

            output_pos = 0U;
        }

        if (output_pos == buffer_size) {
            this->_eof = true;
            return EOF;
        }

        const auto ch = buffer.at(output_pos);
        output_pos += 1U;
        return ch;
    }
}

#endif //CPPARMC_BWT_DECODE_HPP
