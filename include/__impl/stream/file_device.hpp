#ifndef CPPARMC_FILE_DEVICE_HPP
#define CPPARMC_FILE_DEVICE_HPP

#include <cstdio>
#include <utility>
#include <fmt/format.h>

namespace cpparmc::stream {

    class FileDeviceBase {
    protected:
        std::string fn;
        std::FILE* file;

        void open(const std::string& mode) {
            this->file = std::fopen(fn.c_str(), mode.c_str());
            this->check();
        }

    private:
        static void check(int status) {
            if (status) {
                throw std::runtime_error(fmt::format("File status error: {:d}", status));
            }
        }

    public:
        u_char input_width = 8U;
        u_char output_width = 8U;

        explicit FileDeviceBase(const std::string& fn) :
                fn(fn), file(nullptr) {}

        void check() const {
            FileDeviceBase::check(std::ferror(file));
        }

        void flush() const {
            std::fflush(this->file);
            this->check();
        }

        ~FileDeviceBase() {
            this->flush();
            std::fclose(file);
            this->file = nullptr;
        }
    };

    class InputFileDevice : public FileDeviceBase {
        bool _eof;

    public:
        explicit InputFileDevice(const std::string& fn) :
                FileDeviceBase(fn), _eof(false) {
            this->open("rb");
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

        void reset() {
            std::fseek(file, 0, SEEK_SET);
            _eof = false;
        }
    };

    class OutputFileDevice : public FileDeviceBase {

    public:
        explicit OutputFileDevice(const std::string& fn) : FileDeviceBase(fn) {
            this->open("wb");
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
    };

}

#endif //CPPARMC_FILE_DEVICE_HPP
