//#ifndef CPPARMC_STRING_STREAM_HPP
//#define CPPARMC_STRING_STREAM_HPP
//
//#include <cstdio>
//#include <utility>
//#include <fmt/format.h>
//
//namespace cpparmc {
//
//    class StringStream {
//    public:
//        std::string& src;
//
//        u_char input_width = 8U;
//        u_char output_width = 8U;
//
//        explicit StringStream(const std::string& src) :
//                src(src) {}
//
//        void open() {
//            this->file = std::fopen(fn.c_str(), "rb");
//            this->check();
//        }
//
//        [[nodiscard]] int get() {
//            const auto ch = std::fgetc(this->file);
//            _eof = std::feof(this->file);
//            return ch;
//        }
//
//        template<typename T>
//        InputFileDevice& read(T& val) {
//            std::fread(&val, sizeof(T), 1, this->file);
//            this->check();
//            return *this;
//        }
//
//        void close() const {
//            std::fclose(file);
//        }
//
//        ~FileDeviceBase() {
//            this->close();
//        }
//    };
//
//}
//#endif //CPPARMC_STRING_STREAM_HPP
