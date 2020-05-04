#ifndef CPPARMC_BWT_ENCODE_HPP
#define CPPARMC_BWT_ENCODE_HPP

#include <numeric>
#include <execution>
#include <vector>

#include "__impl/compile_base.h"
#include "__impl/logger.hpp"
#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/utils/darray.hpp"


namespace cpparmc::stream {

    using namespace utils;

    template<typename Device,
            std::size_t init_buffer_size=4 * 1024 * 1024,
            typename SizeType=std::uint32_t>
    class BWTEncode: public Stream<Device> {
        std::uint8_t symbol_bits;
        std::uint64_t total_symbol;
        BitStream<Device> bit_adaptor;

        SizeType buffer_pos;
        std::vector<CommonSymbolType> buffer;
        std::vector<CommonSymbolType> bwt_buffer;
        std::vector<SizeType> bwt_index;

    public:
        BWTEncode(Device& device, std::uint8_t symbol_bits);
        StreamStatus receive() final;
    };

    template<typename Device, std::size_t init_buffer_size, typename SizeType>
    BWTEncode<Device, init_buffer_size, SizeType>
    ::BWTEncode(Device& device, std::uint8_t symbol_bits):
            Stream<Device>(device, 8, 8, true),
            symbol_bits(symbol_bits),
            total_symbol(1U << symbol_bits),
            bit_adaptor(BitStream<Device>(device, symbol_bits, true)),
            buffer_pos(0) {
                buffer.reserve(init_buffer_size);
            }

    template<typename Device, std::size_t init_buffer_size, typename SizeType>
    auto BWTEncode<Device, init_buffer_size, SizeType>::receive() -> StreamStatus {
        if (buffer_pos < buffer.size()) return { symbol_bits, buffer.at(buffer_pos++) };

        buffer.clear();

        while (true) {
            const CommonSymbolType ch = this->bit_adaptor.get();
            if (this->device.eof()) break;
            buffer.push_back(ch);
        }

        if (buffer.empty()) return {-1, 0};

        const auto buffer_size = buffer.size();
        DEBUG_PRINT("read a block with size: [{:d}]. ", buffer_size);

        auto cycle_index = [=](auto r, auto offset) { return r >= offset ? r - offset : r + buffer_size - offset; };

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
        std::uint32_t same_row_index = std::distance(bwt_index.begin(), _p);

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

        same_row_index = same_row_index & (bits::get_n_repeat_bit(true, 24U)) | (symbol_bits << 24U);
        return { 32U, same_row_index };
    }
}

#endif //CPPARMC_BWT_ENCODE_HPP
