#ifndef CPPARMC_BIT_UTILS_HPP
#define CPPARMC_BIT_UTILS_HPP

namespace cpparmc {
    template<typename T>
    bool get_nth_bit(T val, std::size_t n) {
        return (val >> n) & 1;
    }

    template<typename T>
    T set_nth_bit(T val, bool bit, std::size_t n) {
        return val | (bit << n);
    }
}

#endif //CPPARMC_BIT_UTILS_HPP
