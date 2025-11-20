#include <cstdint>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <variant>

#include "airplay_device.hpp"
#include "airplay_browser.hpp"

__attribute__((weak)) int main() {
#ifdef AVAHI_COMPAT
    setenv("AVAHI_COMPAT_NOWARN", "y", 0); // disable the annoying warnign!
#endif
    std::cout << "[+] Fetching AirPlay devices:" << std::endl;
    const auto devices = airplay_browser::get_devices();
    for (auto device : devices) {
        std::cout << "     > " << device.first << std::endl;
        std::cout << "     > " << device.second.get_printable_address() << std::endl;
        auto requires_pairing = std::get<uint32_t>(device.second.get_txt()["flags"]) & 0x200;
        std::cout << "     > " << (requires_pairing ? "requires pairing" : "open") << std::endl;
    }

    std::cout << "[+] Connecting to device:" << std::endl;
    airplay_device appletv(devices.begin()->second);
//    airplay_device appletv(devices.at("Samsung Q50 Series LR"));
    std::cout << appletv.send_message(MessageType::GetServices) << std::endl;

    return 0;
}
