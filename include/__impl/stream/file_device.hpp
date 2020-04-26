#ifndef CPPARMC_FILE_DEVICE_HPP
#define CPPARMC_FILE_DEVICE_HPP

#include <cstdio>
#include <utility>
#include <fmt/format.h>

namespace cpparmc {

    class FileDeviceBase {
    public:
        std::string fn;
        std::FILE* file;

        u_char input_width = 8U;
        u_char output_width = 8U;

        explicit FileDeviceBase(const std::string& fn):
        fn(fn), file(nullptr) {}

        static void check(int status) {
            if (status) {
                throw std::runtime_error(fmt::format("File status error: {:d}", status));
            }
        }

        void check() const {
            FileDeviceBase::check(std::ferror(file));
        }

        void close() const {
            std::fclose(file);
        }

        ~FileDeviceBase() {
            this->close();
        }
    };


    class InputFileDevice: public FileDeviceBase {
        bool _eof;

    public:
        explicit InputFileDevice(const std::string& fn):
        FileDeviceBase(fn), _eof(false) {}

        void open() {
            this->file = std::fopen(fn.c_str(), "rb");
            this->check();
        }

        [[nodiscard]] int get() {
            const auto ch = std::fgetc(this->file);
            _eof = std::feof(this->file);
            return ch;
        }

        template<typename T>
        InputFileDevice& read(T& val) {
            std::fread(&val, sizeof(T), 1, this->file);
            this->check();
            return *this;
        }

        [[nodiscard]] u_long tell() const {
            return std::ftell(this->file);
        }

        [[nodiscard]] bool eof() const {
            return this->_eof;
        }
    };

    class OutputFileDevice: public FileDeviceBase {

    public:
        explicit OutputFileDevice(const std::string& fn): FileDeviceBase(fn) {}

        void open() {
            this->file = std::fopen(this->fn.c_str(), "wb");
            this->check();
        }

        [[nodiscard]] OutputFileDevice& put(u_char c) {
            std::putc(c, this->file);
            this->check();
            return *this;
        }

        template<typename T>
        OutputFileDevice& write(T* val, std::size_t n) {
            std::fwrite(val, sizeof(T), n, this->file);
            this->check();
            return *this;
        }

        template<typename T>
        OutputFileDevice& write(T& val) {
            std::fwrite(&val, sizeof(T), 1, this->file);
            this->check();
            return *this;
        }

        void flush() const {
            std::fflush(this->file);
            this->check();
        }
    };

}
#endif //CPPARMC_FILE_DEVICE_HPP
