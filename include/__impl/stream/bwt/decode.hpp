#ifndef CPPARMC_BWT_DECODE_HPP
#define CPPARMC_BWT_DECODE_HPP

#include "__impl/compile_base.h"
#include "__impl/utils/darray.hpp"

namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, typename SymbolType=u_char, typename SizeType=std::uint32_t>
    class BWTDecode: public InputStream<Device> {
        constexpr static u_char m_width = std::numeric_limits<SizeType>::digits;

        SizeType buffer_size;
        SizeType block_size;
        SizeType total_symbol;

        darray <SymbolType> buffer;
        darray <SymbolType> btw_buffer;
        SizeType pos;

        darray <SizeType> M;
        darray <SizeType> L;
        darray <SizeType> stat;

        SizeType last_one_row;

    public:
        BWTDecode(Device& device, SizeType block_size);

        StreamStatus receive() final;
    };

    template<typename Device, typename SymbolType, typename SizeType>
    BWTDecode<Device, SymbolType, SizeType>::BWTDecode(Device& device, SizeType block_size):
            InputStream<Device>(device, 8, 8),
            buffer_size(0),
            block_size(block_size),
            total_symbol(1U << this->input_width),
            buffer(block_size),
            btw_buffer(block_size),
            pos(0),
            M(block_size),
            L(total_symbol),
            stat(total_symbol),
            last_one_row(block_size) {
    }

    template<typename Device, typename SymbolType, typename SizeType>
    auto BWTDecode<Device, SymbolType, SizeType>::receive() -> StreamStatus {
        while (pos < buffer_size) {
            return {this->output_width, buffer.at(pos++)};
        }

        buffer_size = 0;
        pos = 0;

        const auto _size = this->device.read(last_one_row);
        if (this->device.eof()) return {-1, 0};
        assert(_size == sizeof(SizeType));

#ifdef CPPARMC_DEBUG_BWT_DECODER
        spdlog::info("The last_one is {:d}", last_one_row);
#endif

        std::fill(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                stat.begin(), stat.end(), 0);

        std::fill(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                M.begin(), M.end(), 0);

        while (buffer_size < block_size) {
            const auto ch = this->device.get();
            if (this->device.eof()) break;
            buffer[buffer_size] = ch;
            M[buffer_size] = stat[ch];
            buffer_size += 1;
            stat[ch] += 1;
        }

#ifdef CPPARMC_DEBUG_BWT_DECODER
        spdlog::info("The buffer size is {:d}", buffer_size);
#endif

        if (buffer_size == 0) return {-1, 0};

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                buffer.begin(), buffer.end(buffer_size), btw_buffer.begin());
        std::sort(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                btw_buffer.begin(), btw_buffer.end(buffer_size));

        SymbolType now = btw_buffer[0];
        SizeType count = 0U;
        L[0] = 0;

        for (auto i = 1; i < buffer_size; i++) {
            if (btw_buffer[i] != now) {
                count += stat[now];
                now = btw_buffer[i];
            }
            L[btw_buffer[i]] = count;
        }


#ifdef CPPARMC_TIMING
        START_TIMER(REORDER_BWT_TABLE);
#endif

        for (auto i = buffer_size, t = last_one_row; i > 0; i--, t = M[t] + L[buffer[t]]) btw_buffer[i - 1] = buffer[t];

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                btw_buffer.begin(), btw_buffer.end(buffer_size), buffer.begin());

#ifdef CPPARMC_TIMING
        END_TIMER_AND_OUTPUT_MS(REORDER_BWT_TABLE);
#endif
        return {this->output_width, buffer.at(pos++)};
    }
}

#endif //CPPARMC_BWT_DECODE_HPP
