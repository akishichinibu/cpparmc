#ifndef CPPARMC_DARRAY_HPP
#define CPPARMC_DARRAY_HPP

#include <memory>
#include <execution>

namespace cpparmc::utils {

    template<typename T>
    class darray {
        constexpr static auto element_type = sizeof(T);

        std::unique_ptr<T[]> buffer;
        std::size_t size = 0;

    public:
        inline explicit darray(): size(0) {};

        inline explicit darray(std::size_t size):
                size(size), buffer(std::make_unique<T[]>(size)) {}

        template<typename W>
        inline explicit darray(std::size_t size, W val) :
                darray(size) {
            std::fill(
#ifdef USING_PARALLEL_STL
                    std::execution::par_unseq,
#endif
                    this->begin(),
                    this->end(),
                    val);
        }

        inline T& at(std::size_t index) const {
            return this->buffer[index];
        }

        inline T& operator[](std::size_t index) {
            return this->buffer[index];
        }

//        class iterator {
//            typedef std::ptrdiff_t difference_type;
//            typedef T value_type;
//            typedef T* pointer;
//            typedef T& reference;
//            typedef std::random_access_iterator_tag iterator_category;
//            std::size_t index;
//
//            explicit iterator(std::size_t index): index(index) {}
//
//            inline iterator operator++() {
//                return iterator(index == this->size - 1 ? 0 : index + 1);
//            };
//
//            inline T& operator*() const {
//                return this->buffer[index];
//            }
//        };

        inline T* begin() {
            return this->buffer.get();
        }

        inline T* end() {
            return this->end(this->size);
        }

        inline T* end(std::size_t offset) {
            return this->begin() + offset;
        }

        inline const T* cbegin() const {
            return this->buffer.get();
        }

        inline const T* cend() const {
            return this->buffer.get() + size;
        }

        inline const T* cend(std::size_t offset) const {
            return this->buffer.get() + offset;
        }

//        inline iterator cycle_begin(std::size_t offset) {
//            return iterator(offset);
//        }
    };
}

#endif //CPPARMC_DARRAY_HPP
