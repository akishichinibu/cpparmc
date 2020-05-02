#ifndef CPPARMC_BWT_ENCODE_HPP
#define CPPARMC_BWT_ENCODE_HPP

#include <numeric>
#include <execution>
#include <map>

#include "__impl/compile_base.h"
#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/utils/darray.hpp"


namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, typename SymbolType=std::uint8_t, typename SizeType=std::uint32_t>
    class BWTEncode: public InputStream<Device> {
        constexpr static std::uint8_t m_width = std::numeric_limits<SizeType>::digits;

        SizeType buffer_size;
        SizeType block_size;

        darray <SymbolType> buffer;
        darray <SymbolType> bwt_buffer;
        darray <SizeType> bwt_index;
        std::map<std::pair<SizeType, SizeType>, bool> cmp_cache;

        SizeType pos;
        SizeType m0;

    public:
        BWTEncode(Device& device, SizeType block_size);

        StreamStatus receive() final;
    };

    template<typename Device, typename SymbolType, typename SizeType>
    BWTEncode<Device, SymbolType, SizeType>::BWTEncode(Device& device, SizeType block_size):
            InputStream<Device>(device, device.output_width, device.output_width),
            buffer_size(0),
            block_size(block_size),
            buffer(block_size),
            bwt_buffer(block_size),
            bwt_index(block_size),
            pos(0),
            m0(block_size) {}

    template<typename Device, typename SymbolType, typename SizeType>
    auto BWTEncode<Device, SymbolType, SizeType>::receive() -> StreamStatus {

        while (pos < buffer_size) {
            return {this->output_width, buffer.at(pos++)};
        }

        for (buffer_size = 0; buffer_size < block_size; buffer_size++) {
            const auto ch = this->device.get();
            if (this->device.eof()) break;
            buffer[buffer_size] = ch;
        }

        if (buffer_size == 0) return {-1, 0};

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        spdlog::info("Read a block with size: [{:d}]. ", buffer_size);
#endif

#ifdef CPPARMC_TIMING
        START_TIMER(SORT_BWT_TABLE);
#endif

        std::iota(bwt_index.begin(), bwt_index.end(buffer_size), 0);

        cmp_cache.clear();

        std::sort(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(), bwt_index.end(buffer_size),
                [&](auto r1, auto r2) {
                    const std::pair<SizeType, SizeType> pair {r1, r2};
                    const auto _p = cmp_cache.find(pair);
                    if (_p != cmp_cache.end()) return cmp_cache.at(pair);

                    for (auto t = 0; t < buffer_size; t++) {
                        const auto a = buffer[(t - r1) % buffer_size];
                        const auto b = buffer[(t - r2) % buffer_size];
                        if (a != b) {
                            if (t >= (buffer_size >> 1U)) cmp_cache[pair] = a < b;
                            return a < b;
                        }
                    }
                    cmp_cache[pair] = true;
                    return true;
                });

        const auto _p = std::find(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(),
                bwt_index.end(buffer_size), 0);

        m0 = std::distance(bwt_index.begin(), _p);

        assert(m0 != buffer_size);

#ifdef CPPARMC_TIMING
        END_TIMER_AND_OUTPUT_MS(SORT_BWT_TABLE);
#endif

//#ifdef CPPARMC_DEBUG_BWT_ENCODER
//        spdlog::info("The m0 of bwt is [{:d}]. ", m0);
//
//        for (auto i = 0; i < buffer_size; i++) {
//            printf("%d: %d, ", i, bwt_index[i]);
//
//            if ((i + 1) % 8 == 0) printf("\n");
//        }
//        printf("\n");
//#endif

        std::transform(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_index.begin(), bwt_index.end(buffer_size), bwt_index.begin(),
                [=](auto r) {
                    return ((buffer_size - 1) - r) % buffer_size;
                });

        for (auto i = 0; i < buffer_size; i++) bwt_buffer[i] = buffer[bwt_index[i]];

        std::copy(
#ifdef USING_PARALLEL_STL
                std::execution::par_unseq,
#endif
                bwt_buffer.begin(), bwt_buffer.end(buffer_size), buffer.begin());

        pos = 0;

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        spdlog::info("After read the size of bwtrle buffer: {:d}. ", buffer_size);
#endif

        return {m_width, m0};
    }
}

#endif //CPPARMC_BWT_ENCODE_HPP
