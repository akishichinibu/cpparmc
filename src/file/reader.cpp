#ifndef CPPARMC_READER_HPP
#define CPPARMC_READER_HPP

#include <sstream>

#include "__impl/file/reader.h"
#include "__impl/stream/arithmetic/decode.hpp"


namespace cpparmc {

    u_int ARMCFileReader::read_package_head() {
        ARMCPackageHeader package_header{};
        this->input_stream.read(package_header);

        printf("The pkg_len=[%lu] uncompress_len=[%lu]\n",
                package_header.package_length, package_header.uncompress_length);

        return package_header.uncompress_length;
    }

    ARMCFileReader::ARMCFileReader(const std::string& fn,
                                   const armc_params& params,
                                   const armc_coder_params& coder_params) :
            ARMCFileMixin(fn, params, coder_params),
            input_stream(InputFileDevice(fn)) {}

    void ARMCFileReader::open() {
        if (this->has_open) {
            throw std::runtime_error("The file has been opened. ");
        }

        has_open = true;

        input_stream.open();

        ARMCFileHeader file_header{};
        input_stream.read(file_header);
    }

    std::basic_string<u_char> ARMCFileReader::read() {
        if (!has_open) {
            throw std::runtime_error("The file should be opened first. ");
        }

        const auto uncompressed_length = this->read_package_head();

        BitStream<InputFileDevice> s1 {
                input_stream,
                1U
        };

        ArithmeticCDecode<BitStream<InputFileDevice>> s2 {
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
}

#endif //CPPARMC_READER_HPP
