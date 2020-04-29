#ifndef CPPARMC_READER_HPP
#define CPPARMC_READER_HPP

#include <sstream>
#include <CRC.h>

#include "__impl/file/reader.h"
#include "__impl/stream/arithmetic/decode.hpp"


namespace cpparmc {

    using namespace stream;

    namespace file {

        u_int64_t ARMCFileReader::read_package_head() {
            ARMCPackageHeader package_header{};
            this->input_stream.read(package_header);

            spdlog::info("Read a package with package=[{:d}] uncompress=[{:d}]",
                         package_header.package_length, package_header.uncompress_length);

            return package_header.uncompress_length;
        }

        ARMCFileReader::ARMCFileReader(const std::string& fn,
                                       const armc_params& params,
                                       const armc_coder_params& coder_params) :
                ARMCFileMixin(fn, params, coder_params),
                input_stream(InputFileDevice(fn)) {}

        ARMCFileReader& ARMCFileReader::open() {
            if (this->has_open) {
                throw std::runtime_error("The file has been opened. ");
            }

            has_open = true;

            ARMCFileHeader file_header{};
            input_stream.read(file_header);

            if (!((file_header._magic_1 == magic_1) && (file_header._magic_2 == magic_2))) {
                spdlog::error("Unknown file type with error magic. ");
                throw std::runtime_error("Unknown file type.");
            }

            u_int32_t crc = CRC::Calculate(&file_header,
                                           sizeof(ARMCFileHeader) - sizeof(ARMCFileHeader::header_crc),
                                           CRC::CRC_32());

            if (file_header.header_crc != crc) {
                spdlog::error("The CRC of the header has change. ");
            } else {
                spdlog::info("The CRC of the file has not changed. ");
            }

            return *this;
        }

        std::basic_string<u_char> ARMCFileReader::read() {
            if (!has_open) {
                throw std::runtime_error("This file should be opened first. ");
            }

            const auto uncompressed_length = this->read_package_head();

            BitStream<InputFileDevice> s1{
                    input_stream,
                    1U
            };

            ArithmeticDecode<BitStream<InputFileDevice>> s2{
                    s1,
                    uncompressed_length,
                    params,
                    coder_params
            };

            std::basic_stringstream<u_char> output_stream;

            while (true) {
                const auto c = s2.get();
                if (s2.eof()) break;
                output_stream.put(c);
            }

            return output_stream.str();
        }

        void ARMCFileReader::close() {

        }
    }
}

#endif //CPPARMC_READER_HPP
