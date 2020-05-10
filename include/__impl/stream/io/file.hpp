#ifndef CPPARMC_STREAM_IO_FILE_HPP
#define CPPARMC_STREAM_IO_FILE_HPP

#include <string>
#include <tuple>

#include "__impl/stream/generator.hpp"


namespace cpparmc::stream {

    class FileMixin {

    protected:
        std::string fn;
        std::FILE* file;

        void open(const std::string& mode);

        static void check(int status) {
            if (status) throw std::runtime_error("File status error");
        }

    public:
        explicit FileMixin(const std::string& fn);

        void check() const;

        ~FileMixin();
    };

    FileMixin::FileMixin(const std::string& fn):
            fn(fn), file(nullptr) {}

    void FileMixin::open(const std::string& mode) {
        this->file = std::fopen(fn.c_str(), mode.c_str());
        this->check();
    }

    void FileMixin::check() const {
        FileMixin::check(std::ferror(file));
    }

    FileMixin::~FileMixin() {
        std::fclose(file);
        this->file = nullptr;
    }

    template<std::size_t max_buffer_size = 64 * 1024>
    class FileInputStream: public FileMixin, public BufferIOMixin<max_buffer_size> {

        char buffer[max_buffer_size] {};

        std::size_t cursor;
        std::size_t buffer_len;
        std::size_t count;

    public:
        explicit FileInputStream(const std::string& fn):
        FileMixin(fn),
        BufferIOMixin<max_buffer_size>(*this),
        cursor(0),
        buffer_len(0),
        count(0) {
            this->open("rb");
            std::setvbuf(this->file, nullptr, _IONBF, max_buffer_size);
        }

        std::size_t fill() noexcept {
//            std::fread()
            return max_buffer_size;
        }
    };
}

#endif //CPPARMC_STREAM_IO_FILE_HPP
