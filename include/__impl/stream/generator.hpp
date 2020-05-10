#ifndef CPPARMC_GENERATOR_HPP
#define CPPARMC_GENERATOR_HPP

namespace cpparmc::stream {

    typedef std::uint8_t StreamSizeType;
    typedef std::int64_t SymbolType;
    typedef std::uint64_t BufferType;

    typedef std::optional<std::pair<StreamSizeType, SymbolType>> StreamStatus;

    constexpr StreamStatus empty_frame = StreamStatus(std::in_place, 0, 0);

    struct OutsideSource {};

    template<typename Source, std::size_t init_bit_buffer_size = 256>
    class Generator {
        constexpr static auto max_buffer_width = std::numeric_limits<BufferType>::digits;

        bool _eof {};

        std::vector<bool> _i_buffer;
        std::size_t _i_buffer_pos {};
        std::size_t _output_count {};

    protected:
        explicit Generator(Source& src) noexcept;

        virtual StreamStatus patch() noexcept = 0;

        void send(StreamSizeType size, SymbolType buf) noexcept;

        [[nodiscard]] bool src_eof() const noexcept;

        Source& src;

    public:
        [[nodiscard]] bool eof() const noexcept;

        bool next() noexcept;

        StreamStatus next(std::uint8_t width, bool force_align=false) noexcept;
    };

    template<typename Source, std::size_t ib>
    Generator<Source, ib>::Generator(Source& src) noexcept:
    _i_buffer_pos(0),
    _output_count(0),
    src(src) {
        _i_buffer.reserve(ib);
    }

    template<typename Source, std::size_t ib>
    bool Generator<Source, ib>::eof() const noexcept {
        return this->_eof;
    }

    template<typename Source, std::size_t ib>
    void Generator<Source, ib>
    ::send(StreamSizeType size, SymbolType buf) noexcept {
        const auto _origin_tail = _i_buffer.size();
        _i_buffer.resize(_origin_tail + size);
        for (auto i = 0; i < size; i++) _i_buffer[_origin_tail + i] = bits::get_nth_bit(buf, size - 1 - i);
    }

    template<typename Source, std::size_t ib>
    bool Generator<Source, ib>::next() noexcept {
        if (_i_buffer_pos != _i_buffer.size()) {
            return _i_buffer.at(_i_buffer_pos++);
        }

        _i_buffer_pos = 0;
        _i_buffer.resize(0);

        StreamSizeType size;
        BufferType buf;

        while (_i_buffer.empty()) {
            const auto frame = this->patch();

            if (!frame) {
                break;
            }

            std::tie(size, buf) = frame.value();
            assert((size < max_buffer_width) && (0LU <= buf));
            this->send(size, buf);
        }

        if (_i_buffer.empty()) {
            this->_eof = true;
            return false;
        }

        return _i_buffer.at(_i_buffer_pos++);
    }

    template<typename Source, std::size_t ib>
    StreamStatus Generator<Source, ib>::next(std::uint8_t width, bool force_align) noexcept {
        BufferType buf = 0UL;
        std::uint8_t i;

        for (i = 0U; i < width; i++) {
            bits::set_nth_bit(buf, this->next(), width - 1LU - i);
            if (this->eof()) break;
        }

        if (force_align) {
            return StreamStatus(std::in_place, width, buf);
        } else {
            return i == 0 ? std::nullopt : StreamStatus(std::in_place, i, buf >> (width - i));
        }
    }

    template<typename Source, std::size_t ib>
    bool Generator<Source, ib>::src_eof() const noexcept {
        if constexpr (std::is_same_v<Source, OutsideSource>) {
            return this->eof();
        } else {
            return this->src.eof();
        }
    }
}

#endif //CPPARMC_GENERATOR_HPP
