#ifndef CPPARMC_DARRAY_HPP
#define CPPARMC_DARRAY_HPP

#include <memory>
#include <execution>

namespace cpparmc::utils {

    template<typename T>
    class darray {
        std::unique_ptr<T[]> buffer;
        std::size_t size = 0;

    public:
        inline explicit darray(std::size_t size):
                size(size), buffer(std::make_unique<T[]>(size)) {}

        template<typename W>
        inline explicit darray(std::size_t size, W val) :
                darray(size) {
            std::fill(
#ifdef USING_PARALLEL_STL
                    std::execution::par_unseq,
#endif
                    this->begin(), this->end(), val);
        }

        inline T* begin() { return this->buffer.get(); }

        inline T* end() { return this->end(this->size); }

        inline T* end(std::size_t offset) { return this->begin() + offset; }

        inline const T* cbegin() { return this->buffer.get(); }

        inline const T* cend() { return this->end(); }

        inline const T* cend(std::size_t offset) { return this->end(offset); }

        inline T& at(std::size_t index) const { return this->buffer[index]; }

        inline T& operator[](std::size_t index) { return this->buffer[index]; }
    };
}

#endif //CPPARMC_DARRAY_HPP
