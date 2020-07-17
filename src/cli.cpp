#include <CLI/CLI.hpp>
#include "armc.h"

namespace cs = cpparmc::stream;
namespace cu = cpparmc::utils;


int main(int argc, char** argv) {
    CLI::App app {"ARMC Compression Command Line Tool"};

    bool verbose = false;
    app.add_flag("-v,--verbose", verbose, "More noisy. ");

    // BWT
    std::size_t bwt_block_size {1 * 1024 * 1024};
    std::size_t bwt_symbol_bit {8};

    auto sc_bwt = app.add_subcommand(
            "bwt",
            "The Burrows–Wheeler transform of the input. ");

    bool bwt_is_encode = false;

    sc_bwt->add_flag("-e,!-d,--encode,!--decode", bwt_is_encode)->required();

    sc_bwt->add_option<std::size_t>("-s,--symbol_bit", bwt_symbol_bit, "The symbol bit for BWT. ")
            ->check(CLI::Validator(CLI::Range(0, 32)));

    sc_bwt->callback([&]() {
        cs::StdInputStream<> inp;

        if (bwt_is_encode) {
            cs::BWTEncode<cs::StdInputStream<>> s1(inp, bwt_symbol_bit);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        } else {
            cs::BWTDecode<cs::StdInputStream<>> s1(inp);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        }
    });

    //FIB
    bool fib_is_encode = false;
    auto sc_fib = app.add_subcommand("fib", "The Fibonacci encoding of the input. ");

    sc_fib->add_flag("-e,!-d,--encode,!--decode", fib_is_encode)->required();

    sc_fib->callback([&]() {
        cs::StdInputStream<> inp;

        if (fib_is_encode) {
            cs::FibonacciEncode<cs::StdInputStream<>> s1(inp);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        } else {
//            cs::F<cs::StdInputStream<>> s1(inp);
//            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        }
    });

    // RLE
    std::uint64_t rle_counter_bit {4};
    std::size_t rle_symbol_bit {8};

    bool rle_is_encode = false;
    auto sc_rle = app.add_subcommand("rle", "The Run-length encoding of the input. ");

    sc_rle->add_flag("-e,!-d,--encode,!--decode", rle_is_encode)->required();

    sc_rle->add_option<std::size_t>("-s,--symbol_bit", rle_symbol_bit, "The symbol bit for RLE. ")
            ->check(CLI::Validator(CLI::Range(0, 32)));

    sc_rle->add_option<std::uint64_t>("-c,--counter_bit", rle_counter_bit, "The counter bit nums for RLE. ")
            ->check(CLI::Validator(CLI::Range(0, 64)));

    sc_rle->callback([&]() {
        cs::StdInputStream<> inp;

        if (rle_is_encode) {
            cs::RLEEncode<cs::StdInputStream<>> s1(inp, rle_symbol_bit, rle_counter_bit);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        } else {
            cs::RLEDecode<cs::StdInputStream<>> s1(inp);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); });
        }
    });

    // ARMC
    std::size_t arith_symbol_bit {8};
    bool arith_is_encode = false;

    auto sc_arith = app.add_subcommand("ari", "The Arithmetic encoding of the input. ");

    sc_arith->add_flag("-e,!-d,--encode,!--decode", arith_is_encode)->required();

    sc_arith->add_option<std::size_t>("-s,--symbol_bit", arith_symbol_bit, "The symbol bit for Arithmetic. ")
            ->check(CLI::Validator(CLI::Range(0, 32)));

    sc_arith->callback([&]() {
        cs::StdInputStream<> inp;

        if (arith_is_encode) {
            cs::ArithmeticEncode<cs::StdInputStream<>> s1(inp, arith_symbol_bit);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); }, true);
        } else {
            cs::ArithmeticDecode<cs::StdInputStream<>> s1(inp);
            cu::read_while_eof(s1, [&](auto ch) { std::cout.put(ch); }, true);
        }

    });

    CLI11_PARSE(app, argc, argv);
    return 0;
}
