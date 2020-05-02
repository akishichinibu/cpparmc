#ifndef CPPARMC_WRITER_H
#define CPPARMC_WRITER_H

#include <memory>
#include <sstream>
#include <vector>
#include <string_view>
#include <ctime>
#include <CRC.h>

#include "__impl/file/handler_mixin.h"
#include "__impl/utils/darray.hpp"

#include "__impl/stream/bit_stream.hpp"
#include "__impl/stream/file/write.hpp"
#include "__impl/stream/arithmetic/encode.hpp"


namespace cpparmc {

    using namespace stream;
    using namespace setting;

    namespace file {

        class ARMCFileWriter: public ARMCFileMixin {
            //+-----------------+-----------------+-----------------+-----------------+
            //|               MAGIC               |   VER  |  ALGO  |   TAIL MAGIC    |
            //+-----------------+-----------------+-----------------+-----------------+
            //|    PLATFORM     |      FLAG       |                                   |
            //+-----------------+-----------------+-----------------+-----------------+
            //|                                 MTIME                                 |
            //+-----------------+-----------------+-----------------+-----------------+
            //|                               HEADER CRC                              |
            //+=================+=================+=================+=================+
            //|                              ...pkg_1...                              |
            //+=================+=================+=================+=================+
            //|                              ...pkg_2...                              |
            //+=================+=================+=================+=================+
            //|                                  ...                                  |
            //+=================+=================+=================+=================+
            //|                              ...pkg_n...                              |
            //+=================+=================+=================+=================+
            OutputFileDevice<> output_stream;

            void write_package(std::uint64_t uncompress_length, std::vector<std::uint8_t>&& s);

        public:
            ARMCFileWriter(const std::string& fn,
                           const armc_params& params,
                           const armc_coder_params& coder_params);

            ARMCFileWriter& open();

            void write_header();

            void write(InputFileDevice<>& s);

            void close() final;
        };

        void ARMCFileWriter::write_package(std::uint64_t uncompress_length, std::vector<std::uint8_t>&& s) {

            ARMCPackageHeader package_header {
                    sizeof(ARMCPackageHeader) + s.size(),
                    this->params.symbol_bit,
                    uncompress_length,
                    CRC::Calculate(s.data(), s.size(), CRC::CRC_32()),
            };

            spdlog::info("Write a package with package=[{:d}] body=[{:d}] uncompress=[{:d}]. ",
                         package_header.package_length, s.size(), uncompress_length);

            output_stream.write(&package_header.package_length);
            output_stream.write(&package_header.symbol_bit);
            output_stream.write(&package_header.uncompress_length);
            output_stream.write(&package_header.pkg_crc);
            output_stream.write(s.data(), s.size());
        }

        ARMCFileWriter::ARMCFileWriter(const std::string& fn,
                                       const armc_params& params,
                                       const armc_coder_params& coder_params):
                ARMCFileMixin(fn, params, coder_params),
                output_stream(OutputFileDevice(this->fn)) {
            spdlog::info("The file {:s} has been open. ", fn);
        }

        ARMCFileWriter& ARMCFileWriter::open() {
            if (this->has_open) {
                throw std::runtime_error("The file cannot be opened twice. ");
            }

            this->has_open = true;
            this->write_header();
            return *this;
        }

        void ARMCFileWriter::write_header() {

            ARMCFileHeader file_header {
                    magic_1,
                    magic_2,
                    0b01001001,
                    0b10101111,
                    0b011110001,
                    static_cast<std::uint64_t>(std::time(nullptr)),
                    CRC::Calculate(&file_header,
                                   sizeof(ARMCFileHeader) - sizeof(ARMCFileHeader::header_crc),
                                   CRC::CRC_32()),
            };

            output_stream.write(&file_header._magic_1);
            output_stream.write(&file_header._magic_2);
            output_stream.write(&file_header.ver_algo);
            output_stream.write(&file_header.platform);
            output_stream.write(&file_header.flag);
            output_stream.write(&file_header.mtime);
            output_stream.write(&file_header.header_crc);
            this->output_stream.flush();
        }

        void ARMCFileWriter::write(InputFileDevice<>& s) {
            if (!this->has_open) {
                throw std::runtime_error("This file should be opened first. ");
            }

            BitStream<InputFileDevice<>> s1 {s, this->params.symbol_bit};

            ArithmeticEncode<BitStream<InputFileDevice<>>> s2 {
                    s1,
                    this->params.symbol_bit,
                    this->coder_params.pkg_size
            };

            std::vector<std::uint8_t> pkg_buffer;
            pkg_buffer.reserve(this->coder_params.pkg_size);

            while (true) {
                const auto ch = s2.get();
                if (s2.eof()) break;
                pkg_buffer.push_back(ch);
            }

            this->write_package(s2.input_count, std::move(pkg_buffer));
        }

        void ARMCFileWriter::close() {
            this->output_stream.flush();
        }
    }
}

#endif //CPPARMC_WRITER_H
