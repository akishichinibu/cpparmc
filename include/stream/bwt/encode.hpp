#ifndef CPPARMC_BWT_ENCODE_HPP
#define CPPARMC_BWT_ENCODE_HPP

#include <numeric>
#include <execution>
#include <vector>

#include "compile_base.h"
#include "logger.hpp"

#include "stream/generator.hpp"
#include "utils/timer.hpp"
#include "utils/darray.hpp"


namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, std::size_t init_buffer_size = 4 * 1024 * 1024>
    class BWTEncode: public Generator<Device> {
        StreamSizeType symbol_bits;
        std::uint64_t total_symbol;

        std::size_t buffer_pos;
        std::vector<SymbolType> buffer;
        std::vector<SymbolType> bwt_buffer;
        std::vector<std::size_t> bwt_index;

    public:
        BWTEncode(Device& device, StreamSizeType symbol_bits) noexcept;

        StreamStatus patch() noexcept final;
    };

    template<typename Device, std::size_t ib>
    BWTEncode<Device, ib>
    ::BWTEncode(Device& device, StreamSizeType symbol_bits) noexcept :
            Generator<Device>(device),
            symbol_bits(symbol_bits),
            total_symbol(1U << symbol_bits),
            buffer_pos(0) {

        buffer.reserve(ib);

        while (true) {
            const auto frame = this->src.next(symbol_bits);
            if (this->src.eof()) break;
            assert(std::get<0>(frame.value()) == symbol_bits);
            buffer.push_back(std::get<1>(frame.value()));
        }

        const auto buffer_size = buffer.size();
        DEBUG_PRINT("read a block with size: [{:d}]. ", buffer_size);

        auto cycle_index = [=](auto r, auto offset) {
            return r >= offset ? r - offset : r + buffer_size - offset;
        };

        bwt_index.reserve(buffer_size);
        bwt_index.resize(buffer_size);
        std::iota(bwt_index.begin(), bwt_index.end(), 0);

#ifdef CPPARMC_TIMING
        START_TIMER(SORT_BWT_TABLE);
#endif

        std::sort(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(),
                bwt_index.end(),
                [&](auto r1, auto r2) {
                    for (auto t = 0; t < buffer_size; t++) {
                        const auto a = buffer[cycle_index(t, r1)];
                        const auto b = buffer[cycle_index(t, r2)];
                        if (a != b) { return a < b; }
                    }
                    return true;
                });

#ifdef CPPARMC_TIMING
        END_TIMER_AND_OUTPUT_MS(SORT_BWT_TABLE);
#endif

        const auto _p = std::find(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(),
                bwt_index.end(),
                0);

        assert(_p != bwt_index.end());
        std::size_t same_row_index = std::distance(bwt_index.begin(), _p);

        DEBUG_PRINT("The m0 of bwt is [{:d}]. ", same_row_index);

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        for (auto i = 0; i < buffer_size; i++) {
            for (auto j = 0; j < buffer_size; j++) DEBUG_PRINT("%c ", buffer[cycle_index(j, i)]);
            DEBUG_PRINT("\n");
        }
#endif

        std::transform(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(),
                bwt_index.end(),
                bwt_index.begin(),
                [=](auto r) { return ((buffer_size - 1) - r) % buffer_size; });

        bwt_buffer.reserve(buffer_size);
        bwt_buffer.resize(buffer_size);
        for (auto i = 0; i < buffer_size; i++) bwt_buffer[i] = buffer[bwt_index[i]];

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_buffer.begin(),
                bwt_buffer.end(),
                buffer.begin());

        buffer_pos = 0;

        this->send(8U, symbol_bits);
        this->send(24U, same_row_index);
    }

    template<typename Device, std::size_t ib>
    auto BWTEncode<Device, ib>::patch() noexcept -> StreamStatus {
        return buffer_pos < buffer.size() ?
               StreamStatus(std::in_place, symbol_bits, buffer.at(buffer_pos++)) : std::nullopt;
    }
}

#endif //CPPARMC_BWT_ENCODE_HPP
