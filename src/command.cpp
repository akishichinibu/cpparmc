#include <string>
#include <CLI/CLI.hpp>

#include "armc.h"
#include <filesystem>


int main(int argc, char** argv) {
    CLI::App app {"ARMC Compression Command Line Tool"};

    std::string inp_fn;
    std::string out_fn;
    std::uint64_t block_size = 64 * 1024;
    std::uint8_t symbol_bit = 12;
    bool verbose = false;

    auto cmd_compress = app.add_subcommand("c", "Compress a file. ");

    cmd_compress
            ->add_option("-i,--input", inp_fn, "input file path")
            ->check(CLI::Validator(CLI::ExistingFile));

    cmd_compress
            ->add_option("-o,--output", out_fn, "output file path");

    cmd_compress
            ->add_option("-b,--block_size", block_size, "block size")
            ->check(CLI::Validator(CLI::PositiveNumber));

    cmd_compress
            ->add_option("-s,--symbol_bit", symbol_bit, "symbol bit nums")
            ->check(CLI::Validator(CLI::Range(4, 32)));

    cmd_compress
            ->add_flag("-v,--verbose", verbose, "verbose output");

    cmd_compress->callback([&]() {
        namespace cf = cpparmc::file;

        cpparmc::armc_params _compress_params = {symbol_bit};
        cpparmc::armc_coder_params _common_params = {block_size};

        std::filesystem::path _inp_path(inp_fn);
        cpparmc::InputFileDevice<> inpf {_inp_path.string()};


        std::filesystem::path _out_path;

        if (out_fn.empty()) {
            _out_path = std::filesystem::path(inp_fn);
            _out_path.replace_extension(_inp_path.extension().string() + ".armc");
        } else {
            _out_path = std::filesystem::path(out_fn);
        }

        cf::ARMCFileWriter armc_file(_out_path, _compress_params, _common_params);

        armc_file.open();
        START_TIMER(WRITE_ARMC_FILE);
        armc_file.write(inpf);
        END_TIMER_AND_OUTPUT_MS(WRITE_ARMC_FILE);
        armc_file.close();
    });

    auto cmd_decompress = app.add_subcommand("d", "Decompress a file. ");
    cmd_decompress
            ->add_option("-f,--file", inp_fn, "The input file path. ")
            ->check(CLI::Validator(CLI::ExistingFile).application_index(0));

    cmd_decompress
            ->add_option("-o,--output", out_fn, "output file path");

    cmd_decompress->callback([&]() {
        namespace cf = cpparmc::file;

        std::filesystem::path _inp_path(inp_fn);
        std::filesystem::path _out_path;

        if (out_fn.empty()) {
            _out_path = std::filesystem::path(inp_fn);
            _out_path.replace_extension(_inp_path.extension().string() + ".decompress");
        } else {
            _out_path = std::filesystem::path(out_fn);
        }

        cf::ARMCFileReader armc_file(inp_fn);

        armc_file.open();
        START_TIMER(READ_ARMC_FILE);
        const auto buf = armc_file.read();
        END_TIMER_AND_OUTPUT_MS(READ_ARMC_FILE);
        armc_file.close();

        cpparmc::OutputFileDevice outf(_out_path.string());
        outf.write(buf.c_str(), buf.size());
    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
