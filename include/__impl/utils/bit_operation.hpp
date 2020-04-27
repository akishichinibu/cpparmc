#ifndef CPPARMC_BIT_UTILS_HPP
#define CPPARMC_BIT_UTILS_HPP

namespace cpparmc::bits {

    template<typename T>
    bool get_nth_bit(T val, std::size_t n) {
        return (val >> n) & 1;
    }

    template<typename T>
    u_int64_t set_nth_bit(T val, bool bit, std::size_t n) {
        return val | (bit << n);
    }

    template<typename T, typename R>
    u_int64_t append_bit(T origin, R val) {
        return (origin << 1U) | static_cast<bool>(val);
    }

    template<typename T, typename R>
    u_int64_t append_bits(T origin, R val, std::size_t width) {
        return (origin << width) | val;
    }

    template<typename T>
    std::pair<u_int64_t, std::size_t> pop_bits(T& origin, std::size_t width, std::size_t head) {
        const auto rest_width = width - head;
        u_int64_t buf;

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

    template<typename T>
    u_int64_t get_range(T origin, std::size_t a, std::size_t b) {
        const auto mask = (1U << (b - a)) - 1U;
        return (origin >> a) & mask;
    }

    template<typename T, typename R>
    u_int64_t set_range(T origin, R value, std::size_t a, std::size_t b) {
        const auto mask = (1U << (b - a)) - 1U;
        return origin | ((value & mask) << a);
    }

    template<typename T>
    u_char get_bits_width(T origin) {
        u_char count = 0U;
        while (origin > 0U) {
            origin >>= 1U;
            count += 1;
        }
        return count;
    }

}

#endif //CPPARMC_BIT_UTILS_HPP
