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
        inline explicit darray() noexcept: size(0) {};

        inline explicit darray(std::size_t size) noexcept:
                size(size), buffer(std::make_unique<T[]>(size)) {}

        template<typename W>
        inline explicit darray(std::size_t size, W val) noexcept :
                darray(size) {
            std::fill(
#ifdef USING_PARALLEL_STL
                    std::execution::par_unseq,
#endif
                    this->begin(),
                    this->end(),
                    val);
        }

        inline T& at(std::size_t index) const noexcept {
            return this->buffer[index];
        }

        inline T& operator[](std::size_t index) noexcept {
            return this->buffer[index];
        }

        inline T* begin() noexcept {
            return this->buffer.get();
        }

        inline T* end() noexcept {
            return this->end(this->size);
        }

        inline T* end(std::size_t offset) noexcept {
            return this->begin() + offset;
        }

        inline const T* cbegin() const noexcept {
            return this->buffer.get();
        }

        inline const T* cend() const noexcept {
            return this->buffer.get() + size;
        }

        inline const T* cend(std::size_t offset) const noexcept {
            return this->buffer.get() + offset;
        }
    };
}

#endif //CPPARMC_DARRAY_HPP
