#ifndef CPPARMC_BWT_DECODE_HPP
#define CPPARMC_BWT_DECODE_HPP

#include "compile_base.h"
#include "utils/darray.hpp"

namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, std::size_t init_buffer_size = 4 * 1024 * 1024>
    class BWTDecode: public Generator<Device> {
        StreamSizeType symbol_bit;
        std::uint64_t total_symbol;

        std::size_t buffer_pos = 0;
        std::vector<SymbolType> buffer;

    public:
        explicit BWTDecode(Device& device) noexcept;

        StreamStatus patch() noexcept final;
    };

    template<typename Device, std::size_t ib>
    BWTDecode<Device, ib>
    ::BWTDecode(Device& device) noexcept:
            Generator<Device>(device),
            symbol_bit(0) {
        buffer.reserve(ib);

        symbol_bit = std::get<1>(this->src.next(8U).value());
        std::uint32_t same_row_index = std::get<1>(this->src.next(24U).value());
        total_symbol = 1U << symbol_bit;

        DEBUG_PRINT("The m0 of current BWT stream is {:d}. symbol bit is {:d}. ", same_row_index, symbol_bit);

        std::vector<std::size_t> M;
        std::vector<std::size_t> L;
        std::vector<std::size_t> stat;

        stat.reserve(total_symbol);
        stat.resize(total_symbol);
        M.reserve(ib);

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

        while (true) {
            const auto frame = this->src.next(symbol_bit);
            if (this->src.eof()) break;
            const auto ch = std::get<1>(frame.value());
            buffer.push_back(ch);
            M.push_back(stat[ch]);
            stat[ch] += 1;
        }

        const auto buffer_size = buffer.size();
        DEBUG_PRINT("read a block with size: {:d}. ", buffer_size);

        std::vector<SymbolType> btw_buffer;
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

        SymbolType now = btw_buffer[0];
        std::size_t count = 0U;
        L.reserve(buffer_size);
        L.resize(buffer_size);

        for (auto i = 0; i < buffer_size; i++) {
            if (btw_buffer[i] != now) {
                count += stat[now];
                now = btw_buffer[i];
            }
            L[btw_buffer[i]] = count;
        }

        std::size_t t = same_row_index;

        for (std::size_t i = buffer_size; i > 0; i--) {
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
    }

    template<typename Device, std::size_t ib>
    auto BWTDecode<Device, ib>
    ::patch() noexcept -> StreamStatus {
        return buffer_pos < buffer.size() ?
               StreamStatus(std::in_place, this->symbol_bit, buffer.at(buffer_pos++)) : std::nullopt;
    }
}

#endif //CPPARMC_BWT_DECODE_HPP
