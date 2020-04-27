#ifndef CPPARMC_BWT_ENCODE_HPP
#define CPPARMC_BWT_ENCODE_HPP

#include <vector>
#include <string>
#include <string_view>
#include <numeric>

#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/darray.hpp"


namespace cpparmc::stream {

    template<typename Device, typename T>
    class BWTRLEEncode : public InputStream<Device> {

        typedef T size_type;
        constexpr static u_char m_width = std::numeric_limits<size_type>::digits;

        size_type buffer_size;
        size_type block_size;

        darray<u_char> buffer;
        darray<u_char> bwt_buffer;
        darray<size_type> bwt_index;
        size_type output_pos;

        size_type header_output_count;
        size_type m0;

    public:
        BWTRLEEncode(Device& device, std::size_t block_size);

        u_int64_t get() final;
    };

    template<typename Device, typename T>
    BWTRLEEncode<Device, T>::BWTRLEEncode(Device& device, std::size_t block_size):
            InputStream<Device>(device, 8U, 8U),
            buffer_size(0U),
            block_size(block_size + 1U),
            buffer(this->block_size),
            bwt_buffer(this->block_size),
            bwt_index(this->block_size),
            output_pos(0U),
            header_output_count(0),
            m0(block_size) {}

    template<typename Device, typename T>
    u_int64_t BWTRLEEncode<Device, T>::get() {
        if (output_pos == buffer_size) {
            buffer_size = 0U;

            for (auto i = 0; i < block_size; i++) {
                const auto ch = this->device.get();
                if (this->device.eof()) break;
                buffer[i] = ch;
                buffer_size += 1;
            }

            if (buffer_size > 0U) {

#ifdef CPPARMC_DEBUG_BWT_ENCODER
                spdlog::info("Read a block with size: [{:d}]. ", buffer_size);
#endif

#ifdef CPPARMC_DEBUG_BWT_ENCODER
                START_TIMER(SORT_BWT_TABLE);
#endif

                std::iota(bwt_index.begin(), bwt_index.begin() + buffer_size, 0);

                std::sort(bwt_index.begin(),
                          bwt_index.begin() + buffer_size,
                          [&](auto& r1, auto& r2) {
                              for (auto i = 0; i < buffer_size; i++) {
                                  if (buffer[(r1 + i) % buffer_size] > buffer[(r2 + i) % buffer_size]) return false;
                              }
                              return true;
                          });

                for (auto i = 0; i < buffer_size; i++) {
                    if (bwt_index[i] == 0U) {
                        m0 = i;
                        break;
                    }
                }

                if (m0 == block_size) {
                    throw std::runtime_error("");
                }

#ifdef CPPARMC_DEBUG_BWT_ENCODER
                END_TIMER_AND_OUTPUT_MS(SORT_BWT_TABLE);
#endif

#ifdef CPPARMC_DEBUG_BWT_ENCODER
                std::cout << "m0: " << m0 << std::endl;

                for (auto i = 0; i < buffer_size; i++) {
                    printf("%d: %d, ", i, bwt_index[i]);

                    if ((i + 1) % 8 == 0) printf("\n");
                }
                printf("\n");
#endif

                for (auto i = 0; i < buffer_size; i++) bwt_buffer[i] = buffer[(bwt_index[i] + (buffer_size - 1)) % buffer_size];
                for (auto i = 0; i < buffer_size; i++) buffer[i] = bwt_buffer[i];

                output_pos = 0U;
                header_output_count = 0U;

#ifdef CPPARMC_DEBUG_BWT_ENCODER
                spdlog::info("After read the size of bwtrle buffer: {:d}. ", buffer_size);
#endif
            }
        }

        if (output_pos >= buffer_size) {
            this->_eof = true;
            return EOF;
        }

        if (header_output_count < m_width) {
            const u_char ch = bits::get_range(m0, header_output_count << 3U, (header_output_count + 1) << 3U);
            header_output_count += 1U;
            return ch;
        }

        const u_char ch = buffer.at(output_pos);
        output_pos += 1U;
        return ch;
    }
}

#endif //CPPARMC_BWT_ENCODE_HPP
