#ifndef CPPARMC_BWT_ENCODE_HPP
#define CPPARMC_BWT_ENCODE_HPP

#include <numeric>

#include "__impl/compile_base.h"
#include "__impl/stream/stream_base.hpp"
#include "__impl/utils/timer.hpp"
#include "__impl/utils/darray.hpp"


namespace cpparmc::stream {

    using namespace utils;

    template<typename Device, typename SymbolType=u_char, typename SizeType=std::uint32_t>
    class BWTEncode: public InputStream<Device> {
        constexpr static u_char m_width = std::numeric_limits<SizeType>::digits;

        SizeType buffer_size;
        SizeType block_size;

        darray<SymbolType> buffer;
        darray<SymbolType> bwt_buffer;
        darray<SizeType> bwt_index;

        SizeType pos;
        SizeType m0;

    public:
        BWTEncode(Device& device, SizeType block_size);

        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SymbolType, typename SizeType>
    BWTEncode<Device, SymbolType, SizeType>::BWTEncode(Device& device, SizeType block_size):
            InputStream<Device>(device, device.output_width, device.output_width),
            buffer_size(0U),
            block_size(block_size),
            buffer(block_size),
            bwt_buffer(block_size),
            bwt_index(block_size),
            pos(0U),
            m0(block_size) {}

    template<typename Device, typename SymbolType, typename SizeType>
    auto BWTEncode<Device, SymbolType, SizeType>::receive() -> std::pair<std::uint8_t, std::uint64_t> {

        while (pos < buffer_size) {
            return { this->output_width, buffer.at(pos++) };
        }

        for (buffer_size = 0U; buffer_size < block_size; buffer_size++) {
            const auto ch = this->device.get();
            if (this->device.eof()) break;
            buffer[buffer_size] = ch;
        }

        if (buffer_size == 0U) {
            this->_eof = true;
            return { 0, 0 };
        }

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        spdlog::info("Read a block with size: [{:d}]. ", buffer_size);
#endif

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        START_TIMER(SORT_BWT_TABLE);
#endif

        std::iota(bwt_index.begin(), bwt_index.end(buffer_size), 0);

        std::sort(bwt_index.begin(), bwt_index.end(buffer_size),
                  [&](auto r1, auto r2) {
                      for (auto t = 0; t < buffer_size; t++) {
                          const auto a = buffer[(t - r1) % buffer_size];
                          const auto b = buffer[(t - r2) % buffer_size];
                          if (a != b) return a < b;
                      }
                      return true;
                  });

        const auto _p = std::find(bwt_index.begin(), bwt_index.end(buffer_size), 0U);
        m0 = std::distance(bwt_index.begin(), _p);

        assert(m0 != buffer_size);

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        END_TIMER_AND_OUTPUT_MS(SORT_BWT_TABLE);
#endif

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        spdlog::info("The m0 of bwt is [{:d}]. ", m0);

        for (auto i = 0; i < buffer_size; i++) {
            printf("%d: %d, ", i, bwt_index[i]);

            if ((i + 1) % 8 == 0) printf("\n");
        }
        printf("\n");
#endif

        std::transform(bwt_index.begin(), bwt_index.end(buffer_size),
                [=](auto r) {
            return ((buffer_size - 1) - r) % buffer_size;
        });

        for (auto i = 0; i < buffer_size; i++) bwt_buffer[i] = buffer[bwt_index[i]];

        std::copy(bwt_buffer.begin(), bwt_buffer.end(buffer_size), buffer.begin());

        pos = 0U;

#ifdef CPPARMC_DEBUG_BWT_ENCODER
        spdlog::info("After read the size of bwtrle buffer: {:d}. ", buffer_size);
#endif

        return { m_width, m0 };
    }
}

#endif //CPPARMC_BWT_ENCODE_HPP
