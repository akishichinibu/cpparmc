#ifndef CPPARMC_DARRAY_HPP
#define CPPARMC_DARRAY_HPP

#include <memory>

namespace cpparmc {

    template<typename T>
    class darray {
        std::unique_ptr<T[]> buffer;
        std::size_t size{};

    public:
        inline explicit darray(std::size_t size) : size(size), buffer(std::make_unique<T[]>(size)) {}

        template<typename W>
        inline explicit darray(std::size_t size, W val) :
        size(size), darray(size) {
            std::fill(this->begin(), this->end(), val);
        }

        inline T* begin() { return this->buffer.get(); }

        inline T* end() { return this->buffer.get() + this->size; }

        inline const T* cbegin() { return this->buffer.get(); }

        inline const T* cend() { return this->buffer.get() + this->size; }

        inline T& at(std::size_t index) const { return this->buffer[index]; }

        inline T& operator[](std::size_t index) { return this->buffer[index]; }
    };
}

#endif //CPPARMC_DARRAY_HPP
