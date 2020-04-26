#ifndef CPPARMC_WRITER_HPP
#define CPPARMC_WRITER_HPP

#include <sstream>
#include <ctime>
#include <CRC.h>

#include "__impl/file/writer.h"
#include "__impl/stream/arithmetic/encode.hpp"


namespace cpparmc {

    void ARMCFileWriter::write_package(u_int64_t uncompress_length,
            std::pair<u_int64_t, u_int64_t> final_range,
            std::basic_string<u_char>&& s) {

        const u_int64_t CRC = 0;

        ARMCPackageHeader package_header {
            1 + 8 + 8 + 8 + 8 + s.size(),
            this->params.symbol_bit,
            std::get<0>(final_range),
            std::get<1>(final_range),
                    uncompress_length,
                    CRC
        };

        printf("The pkg_len=[%lu] body_len=[%lu]\n",
                package_header.package_length, s.size());

        output_stream
        .write(package_header)
        .write(s.c_str(), s.size());
    }

    ARMCFileWriter::ARMCFileWriter(const std::string& fn,
                                   const armc_params& params,
                                   const armc_coder_params& coder_params) :
            ARMCFileMixin(fn, params, coder_params),
            output_stream(OutputFileDevice(this->fn)) {}

    void ARMCFileWriter::open() {
        if (this->has_open) {
            throw std::runtime_error("The file cannot be opened twice. ");
        }

        this->output_stream.open();
        this->has_open = true;
        this->write_header();
    }

    void ARMCFileWriter::write_header() {

        ARMCFileHeader file_header {
                0b00010001, 0b00010001,
                0b00010001,
                0b00010001,
                0b00010001,
                0b00010001,
                static_cast<u_int64_t>(std::time(nullptr)),
                0b00010001
        };

        output_stream.write(file_header);
        this->output_stream.flush();
    }

    void ARMCFileWriter::write(InputFileDevice& s) {
        if (!this->has_open) {
            throw std::runtime_error("The file should be opened first. ");
        }

        const auto init_pos = s.tell();

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

        this->write_package(s2.input_count, {s2.L, s2.R}, std::move(pkg_buffer.str()));
    }

    void ARMCFileWriter::close() {
        this->output_stream.close();
    }
}

#endif //CPPARMC_WRITER_HPP
