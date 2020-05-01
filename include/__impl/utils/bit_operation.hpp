#ifndef CPPARMC_BIT_UTILS_HPP
#define CPPARMC_BIT_UTILS_HPP

namespace cpparmc::bits {

    template<typename T>
    inline bool get_nth_bit(T val, std::size_t n) {
        return (val >> n) & 1;
    }

    template<typename T>
    inline void set_nth_bit(T& val, bool bit, std::size_t n) {
        val = val | (bit << n);
    }

    template<typename T>
    inline void append_bit(T& origin, bool bit) {
        origin = (origin << 1U) | bit;
    }

    template<typename T, typename R>
    inline void concat_bits(T& origin, R val, std::size_t width) {
        origin = (origin << width) | val;
    }

    template<typename T>
    inline std::pair<T, std::size_t> pop_bits(T& origin, std::size_t width, std::size_t head) {
        const auto rest_width = width - head;
        T buf;

        if (rest_width <= 0U) {
            buf = origin;
            origin = 0U;
            return {buf, 0U};
        } else {
            buf = origin >> rest_width;
            origin &= (1U << rest_width) - 1U;
            return {buf, rest_width};
        }
    }

    inline std::uint64_t get_n_repeat_bit(bool bit, std::size_t n) {
        return bit ? (1U << (n + 1U)) - 1U : 0U;
    }
}

#endif //CPPARMC_BIT_UTILS_HPP
