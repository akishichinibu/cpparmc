#ifndef CPPARMC_READER_H
#define CPPARMC_READER_H

#include <sstream>
#include <CRC.h>

#include "__impl/file/handler_mixin.h"
#include "__impl/stream/file/read.hpp"
#include "__impl/stream/bit_stream.hpp"
#include "__impl/stream/arithmetic/decode.hpp"


namespace cpparmc {

    using namespace stream;
    using namespace setting;

    namespace file {

        class ARMCFileReader: public ARMCFileMixin {
            //+-----------------+-----------------+-----------------+-----------------+
            //|                         PKG LENGTH (exclude self)                     |
            //+-----------------+-----------------+-----------------+-----------------+
            //|   symbol_bit    |                                                     |
            //+-----------------+-----------------+-----------------+-----------------+
            //|                        UNCOMPRESSED LENGTH                            |
            //+-----------------------------------------------------------------------+
            //|                                  CRC                                  |
            //+=======================================================================+
            //|                              ...body...                               |
            //+=======================================================================+

            InputFileDevice<> input_stream;

            std::pair<std::uint8_t, std::uint64_t> read_package_head();

            ARMCFileHeader file_header {};
            ARMCPackageHeader package_header {};

        public:
            explicit ARMCFileReader(const std::string& fn);

            ARMCFileReader& open();

            std::basic_string<u_char> read();

            void close() final;
        };

        std::pair<std::uint8_t, std::uint64_t> ARMCFileReader::read_package_head() {
            this->input_stream.read(package_header.package_length);
            this->input_stream.read(package_header.symbol_bit);
            this->input_stream.read(package_header.uncompress_length);
            this->input_stream.read(package_header.pkg_crc);
            spdlog::info("Read a package with package=[{:d}] uncompress=[{:d}]",
                         package_header.package_length, package_header.uncompress_length);
        }

        ARMCFileReader::ARMCFileReader(const std::string& fn):
                ARMCFileMixin(fn),
                input_stream(InputFileDevice<>(fn)) {}

        ARMCFileReader& ARMCFileReader::open() {
            if (this->has_open) {
                throw std::runtime_error("The file has been opened. ");
            }

            has_open = true;
            input_stream.read(file_header._magic_1);
            input_stream.read(file_header._magic_2);
            input_stream.read(file_header.ver_algo);
            input_stream.read(file_header.platform);
            input_stream.read(file_header.flag);
            input_stream.read(file_header.mtime);
            input_stream.read(file_header.header_crc);

            if (!((file_header._magic_1 == magic_1) && (file_header._magic_2 == magic_2))) {
                spdlog::error("Unknown file type with error magic. ");
                throw std::runtime_error("Unknown file type.");
            }

            std::uint32_t crc =
                    CRC::Calculate(&file_header,
                                   sizeof(ARMCFileHeader) - sizeof(ARMCFileHeader::header_crc),
                                   CRC::CRC_32());

            if (file_header.header_crc != crc) {
                spdlog::error("The CRC of the header has change. ");
            } else {
                spdlog::info("The CRC of the file has not changed. ");
            }

            return *this;
        }

        std::basic_string<std::uint8_t> ARMCFileReader::read() {
            if (!has_open) {
                throw std::runtime_error("This file should be opened first. ");
            }

            this->read_package_head();

            BitStream<InputFileDevice<>> s1 {
                    input_stream,
                    1U
            };

            ArithmeticDecode<BitStream<InputFileDevice<>>> s2 {
                    s1,
                    package_header.uncompress_length,
                    package_header.symbol_bit,
            };

            std::basic_stringstream<u_char> output_stream;

            while (true) {
                const auto c = s2.get();
                if (s2.eof()) break;
                output_stream.put(c);
            }

            return output_stream.str();
        }

        void ARMCFileReader::close() {}
    }
}

#endif //CPPARMC_READER_H
