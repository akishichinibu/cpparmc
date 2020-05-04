#ifndef CPPARMC_BWT_DECODE_HPP
#define CPPARMC_BWT_DECODE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/darray.hpp"

namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, std::size_t init_buffer_size = 4 * 1024 * 1024, typename SizeType=std::uint32_t>
    class BWTDecode: public Stream<Device> {
        std::uint8_t symbol_bit;
        SizeType total_symbol;

        std::vector<CommonSymbolType> buffer;
        std::vector<CommonSymbolType> btw_buffer;
        SizeType buffer_pos;

        std::vector<SizeType> M;
        std::vector<SizeType> L;
        std::vector<SizeType> stat;
    public:
        explicit BWTDecode(Device& device);

        StreamStatus receive() final;
    };

    template<typename Device, std::size_t init_buffer_size, typename SizeType>
    BWTDecode<Device, init_buffer_size, SizeType>
    ::BWTDecode(Device& device):
            Stream<Device>(device, 8, 8, false),
            symbol_bit(0),
            total_symbol(1U << this->input_width),
            buffer_pos(0) {
        buffer.reserve(init_buffer_size);
        stat.reserve(total_symbol);
        stat.resize(total_symbol);
        M.reserve(init_buffer_size);
    }

    template<typename Device, std::size_t init_buffer_size, typename SizeType>
    auto BWTDecode<Device, init_buffer_size, SizeType>
    ::receive() -> StreamStatus {
        if (buffer_pos < buffer.size()) {
            return { symbol_bit, buffer.at(buffer_pos++) };
        }

        std::uint32_t same_row_index;
        const auto _size = this->device.read(same_row_index);
        if (this->device.eof()) return {-1, 0};
        assert(_size == sizeof(same_row_index));

        symbol_bit = std::get<0>(bits::pop_bits(same_row_index, sizeof(same_row_index) << 3U, 8U));
        DEBUG_PRINT("The m0 of current BWT stream is {:d}. symbol bit is {:d}. ",
                same_row_index, symbol_bit);

        total_symbol = 1U << symbol_bit;

#ifdef CPPARMC_TIMING
        START_TIMER(REORDER_BWT_TABLE);
#endif

        std::fill(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                stat.begin(),
                stat.end(),
                0);

        buffer.clear();
        M.clear();
        BitStream<Device> bit_input_adaptor(this->device, symbol_bit, false);

        while (true) {
            const auto ch = bit_input_adaptor.get();
            if (this->device.eof()) break;
            buffer.push_back(ch);
            M.push_back(stat[ch]);
            stat[ch] += 1;
        }

        if (buffer.empty()) return { -1, 0 };

        const auto buffer_size = buffer.size();
        DEBUG_PRINT("read a block with size: {:d}. ", buffer_size);

        btw_buffer.reserve(buffer_size);
        btw_buffer.resize(buffer_size);

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                buffer.begin(),
                buffer.end(),
                btw_buffer.begin());

        std::sort(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                btw_buffer.begin(),
                btw_buffer.end());

        CommonSymbolType now = btw_buffer[0];
        SizeType count = 0U;
        L.reserve(buffer_size);
        L.resize(buffer_size);

        for (auto i = 0; i < buffer_size; i++) {
            if (btw_buffer[i] != now) {
                count += stat[now];
                now = btw_buffer[i];
            }
            L[btw_buffer[i]] = count;
        }

        SizeType t = same_row_index;

        for (SizeType i = buffer_size; i > 0; i--) {
            btw_buffer[i - 1] = buffer[t];
            t = M[t] + L[buffer[t]];
        }

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                btw_buffer.begin(),
                btw_buffer.end(),
                buffer.begin());

        buffer_pos = 0;

#ifdef CPPARMC_TIMING
        END_TIMER_AND_OUTPUT_MS(REORDER_BWT_TABLE);
#endif
        return { 0, 0 };
    }
}

#endif //CPPARMC_BWT_DECODE_HPP
