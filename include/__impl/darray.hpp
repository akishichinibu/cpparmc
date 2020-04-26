#ifndef CPPARMC_DARRAY_HPP
#define CPPARMC_DARRAY_HPP

#include <memory>

namespace cpparmc {

    template<typename T>
    class darray {
        std::unique_ptr<T[]> buffer;
        std::size_t size;

    public:
        explicit darray(std::size_t size) : size(size), buffer(std::make_unique<T[]>(size)) {}

        template<typename W>
        explicit darray(std::size_t size, W val) : darray(size) {
            std::fill(this->begin(), this->end(), val);
        }

        explicit darray() = default;

        T* begin() { return this->buffer.get(); }

        T* end() { return this->buffer.get() + this->size; }

        const T* cbegin() { return this->buffer.get(); }

        const T* cend() { return this->buffer.get() + this->size; }

        T& at(std::size_t index) const { return this->buffer[index]; }

        T& operator[](std::size_t index) { return this->buffer[index]; }
    };
}

#endif //CPPARMC_DARRAY_HPP
