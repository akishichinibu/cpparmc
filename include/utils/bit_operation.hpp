#ifndef CPPARMC_BIT_UTILS_HPP
#define CPPARMC_BIT_UTILS_HPP

namespace cpparmc::bits {

    template<typename T>
    inline bool get_nth_bit(T val, std::size_t n) {
        return (val >> n) & 1;
    }

    template<typename T>
    inline void set_nth_bit(T& val, bool bit, std::size_t n) {
        val = val | (static_cast<std::uint64_t>(bit) << n);
    }

    template<typename T>
    inline void append_bit(T& origin, bool bit) {
        origin = (origin << 1U) | static_cast<std::uint64_t>(bit);
    }

    inline std::uint64_t get_n_repeat_bit(bool bit, std::size_t n) {
        return bit ? (1UL << n) - 1UL : 0U;
    }

    template<typename T, typename R>
    inline void concat_bits(T& origin, R val, std::size_t width) {
        origin = (origin << width) | (val & get_n_repeat_bit(true, width));
    }

    template<typename T>
    inline std::pair<T, std::size_t> pop_bits(T& origin, std::size_t width, std::size_t head) {
        if (width <= head) {
            const T buf = (origin & get_n_repeat_bit(true, head)) << (head - width);
            origin = 0U;
            return {buf, 0U};
        } else {
            const auto rest_width = width - head;
            const T buf = (origin >> rest_width) & get_n_repeat_bit(true, head);
            origin &= get_n_repeat_bit(true, rest_width);
            return {buf, rest_width};
        }
    }
}

#endif //CPPARMC_BIT_UTILS_HPP
