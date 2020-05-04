#ifndef CPPARMC_SHORTCUT_HPP
#define CPPARMC_SHORTCUT_HPP

namespace cpparmc::utils {

    template<typename Device, typename Callback>
    void read_while_eof(Device& device, Callback callback) {
        while (true) {
            const auto ch = device.get();
            if (device.eof()) break;
            callback(ch);
        }
    }

}

#endif //CPPARMC_SHORTCUT_HPP
