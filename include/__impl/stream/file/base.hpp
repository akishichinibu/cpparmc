#ifndef CPPARMC_STREAM_FILE_BASE_HPP
#define CPPARMC_STREAM_FILE_BASE_HPP

namespace cpparmc::stream {

    class FileDeviceBase {
    protected:
        std::string fn;
        std::FILE* file;

        void open(const std::string& mode);

        static void check(int status) {
            if (status) {
                throw std::runtime_error("File status error");
            }
        }

    public:
        explicit FileDeviceBase(const std::string& fn);

        void check() const;

        ~FileDeviceBase();
    };

    FileDeviceBase::FileDeviceBase(const std::string& fn):
            fn(fn), file(nullptr) {}

    void FileDeviceBase::open(const std::string& mode) {
        this->file = std::fopen(fn.c_str(), mode.c_str());
        this->check();
    }

    void FileDeviceBase::check() const {
        FileDeviceBase::check(std::ferror(file));
    }

    FileDeviceBase::~FileDeviceBase() {
        std::fclose(file);
        this->file = nullptr;
    }
}

#endif //CPPARMC_STREAM_FILE_BASE_HPP
