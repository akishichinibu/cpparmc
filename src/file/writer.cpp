#ifndef CPPARMC_WRITER_HPP
#define CPPARMC_WRITER_HPP

#include <sstream>
#include <ctime>
#include <CRC.h>

#include "__impl/file/writer.h"
#include "__impl/stream/arithmetic/encode.hpp"


namespace cpparmc {

    using namespace stream;

    namespace file {

        void ARMCFileWriter::write_package(u_int64_t uncompress_length, std::basic_string<u_char>&& s) {

            ARMCPackageHeader package_header{
                    sizeof(ARMCPackageHeader) + s.size(),
                    this->params.symbol_bit,
                    uncompress_length,
                    CRC::Calculate(s.c_str(), s.size(), CRC::CRC_32()),
            };

            spdlog::info("Write a package with package=[{:d}] body=[{:d}] uncompress=[{:d}]. ",
                         package_header.package_length, s.size(), uncompress_length);

            output_stream
                    .write(package_header)
                    .write(s.c_str(), s.size());
        }

        ARMCFileWriter::ARMCFileWriter(const std::string& fn,
                                       const armc_params& params,
                                       const armc_coder_params& coder_params) :
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

            ARMCFileHeader file_header{
                    magic_1,
                    magic_2,
                    0b01001001,
                    0b10101111,
                    0b011110001,
                    static_cast<u_int64_t>(std::time(nullptr)),
                    CRC::Calculate(&file_header,
                                   sizeof(ARMCFileHeader) - sizeof(ARMCFileHeader::header_crc),
                                   CRC::CRC_32()),
            };

            output_stream.write(file_header);
            this->output_stream.flush();
        }

        void ARMCFileWriter::write(InputFileDevice& s) {
            if (!this->has_open) {
                throw std::runtime_error("This file should be opened first. ");
            }

            BitStream<InputFileDevice> s1{
                    s,
                    this->params.symbol_bit
            };

            ArithmeticEncode<BitStream<InputFileDevice>> s2{
                    s1,
                    this->params,
                    this->coder_params
            };

            std::basic_stringstream<u_int8_t> pkg_buffer;

            while (true) {
                const auto ch = s2.get();
                if (s2.eof()) break;

                pkg_buffer.put(ch);
            }

            this->write_package(s2.input_count, std::move(pkg_buffer.str()));
        }

        void ARMCFileWriter::close() {}
    }
}

#endif //CPPARMC_WRITER_HPP
