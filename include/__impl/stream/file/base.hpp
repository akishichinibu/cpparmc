#ifndef CPPARMC_STREAM_FILE_BASE_HPP
#define CPPARMC_STREAM_FILE_BASE_HPP

namespace cpparmc::stream {

    class FileDeviceBase {
    protected:
        std::string fn;
        std::FILE* file;

        void open(const std::string& mode);

    private:
        static void check(int status) {
            if (status) {
                throw std::runtime_error(fmt::format("File status error: {:d}", status));
            }
        }

    public:
        explicit FileDeviceBase(const std::string& fn);

        void check() const;

        void flush() const;

        ~FileDeviceBase();
    };

    FileDeviceBase::FileDeviceBase(const std::string& fn) :
            fn(fn), file(nullptr) {}

    void FileDeviceBase::open(const std::string& mode) {
        this->file = std::fopen(fn.c_str(), mode.c_str());
        this->check();
    }

    void FileDeviceBase::check() const {
        FileDeviceBase::check(std::ferror(file));
    }

    void FileDeviceBase::flush() const {
        std::fflush(this->file);
        this->check();
    }

    FileDeviceBase::~FileDeviceBase() {
        this->flush();
        std::fclose(file);
        this->file = nullptr;
    }
}

#endif //CPPARMC_STREAM_FILE_BASE_HPP
