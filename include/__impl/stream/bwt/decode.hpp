#ifndef CPPARMC_BWT_DECODE_HPP
#define CPPARMC_BWT_DECODE_HPP

namespace cpparmc::stream {

    template<typename Device, typename SymbolType=u_char, typename SizeType=std::uint32_t>
    class BWTDecode : public InputStream<Device> {
        constexpr static u_char m_width = std::numeric_limits<SizeType>::digits;

        SizeType buffer_size;
        SizeType block_size;
        SizeType total_symbol;

        darray<SymbolType> buffer;
        darray<SymbolType> btw_buffer;
        SizeType pos;

        darray<SizeType> M;
        darray<SizeType> L;
        darray<SizeType> stat;

        SizeType last_one_row;

    public:
        BWTDecode(Device& device, SizeType block_size);

        std::pair<std::uint8_t, std::uint64_t> receive() final;
    };

    template<typename Device, typename SymbolType, typename SizeType>
    BWTDecode<Device, SymbolType, SizeType>::BWTDecode(Device& device, SizeType block_size):
            InputStream<Device>(device, 8U, 8U),
            buffer_size(0U),
            block_size(block_size),
            total_symbol(1U << this->input_width),
            buffer(this->block_size),
            btw_buffer(this->block_size),
            pos(0U),
            M(block_size),
            L(total_symbol),
            stat(total_symbol),
            last_one_row(block_size) {
    }

    template<typename Device, typename SymbolType, typename SizeType>
    auto BWTDecode<Device, SymbolType, SizeType>::receive() -> std::pair<std::uint8_t, std::uint64_t> {
        while (pos < buffer_size) {
            return { this->output_width, buffer.at(pos++) };
        }

        buffer_size = 0U;
        pos = 0U;

        this->device.read(last_one_row);

        if (this->device.eof()) {
            this->_eof = true;
            return {0, 0 };
        }

#ifdef CPPARMC_DEBUG_BWT_DECODER
        spdlog::info("The last_one is {:d}", last_one_row);
#endif

        std::fill(stat.begin(), stat.end(), 0);

        std::fill(M.begin(), M.end(), 0);

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

        if (buffer_size == 0U) {
            this->_eof = true;
            return { 0, 0 };
        }

        std::copy(buffer.begin(), buffer.end(), btw_buffer.begin());
        std::sort(btw_buffer.begin(), btw_buffer.begin() + buffer_size);

        SymbolType now = btw_buffer[0];
        SizeType count = 0U;
        L[0U] = 0U;

        for (auto i = 1; i < buffer_size; i++) {
            if (btw_buffer[i] == now) {
                // pass
            } else {
                count += stat[now];
                now = btw_buffer[i];
            }

            L[btw_buffer[i]] = count;
        }


#ifdef CPPARMC_DEBUG_BWT_DECODER
        START_TIMER(REORDER_BWT_TABLE);
#endif

        btw_buffer[buffer_size - 1] = buffer[last_one_row];
        auto t = last_one_row;
        for (auto i = buffer_size - 1; i > 0; i--) {
            auto nt = M[t] + L[buffer[t]];
            btw_buffer[i - 1] = buffer[nt];
            t = nt;
        }

        for (auto i = 0; i < buffer_size; i++) buffer[i] = btw_buffer[i];

#ifdef CPPARMC_DEBUG_BWT_DECODER
        END_TIMER_AND_OUTPUT_MS(REORDER_BWT_TABLE);
#endif
        return { this->output_width, buffer.at(pos++) };
    }
}

#endif //CPPARMC_BWT_DECODE_HPP
