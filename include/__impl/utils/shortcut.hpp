#ifndef CPPARMC_SHORTCUT_HPP
#define CPPARMC_SHORTCUT_HPP

#include <bitset>
#include <limits>


namespace cpparmc::utils {

    template<typename Device, typename Callback, std::uint8_t output_width = 8>
    void read_while_eof(Device& device, Callback callback, bool greedy = false) {

        while (true) {
            const auto frame = device.next(output_width);

            if (device.eof()) {
                if (greedy && frame) {
                    const auto r = frame.value();
                    if (std::get<0>(r) > 0) callback(std::get<1>(r));
                }
                break;
            }

            callback(std::get<1>(frame.value()));
        }
    }
}

#endif //CPPARMC_SHORTCUT_HPP
