#include <variant>
#include <map>
#include <string>

std::map<std::string, std::variant<uint32_t, std::string>>
parse_txt_record(const unsigned char *txt_rec, unsigned short txt_rec_len) {
    // Example: A concatenated string.
    // The first byte is the length.
    // "hello" -> length 5, string "hello"
    // "world" -> length 5, string "world"
    //unsigned char concatenated_string[] = {5, 'h', 'e', 'l', 'l', 'o', 5, 'w', 'o', 'r', 'l', 'd'};
    //int total_length = sizeof(concatenated_string);

    // Create the result map to hold the split strings
    std::map<std::string, std::variant<uint32_t, std::string>> result_map;
    std::string current_pair;

    // Iterate through the txt_rec 
    for (unsigned short i = 0; i < txt_rec_len; ) {
        // Read the length byte
        auto len = txt_rec[i];

        // If the length is 0, it means the end
        if (len == 0) {
            break;
        }

        // Copy the string data (excluding the length byte)
        current_pair = std::string((const char *)txt_rec + i + 1, len);

        auto equal_sign_pos = current_pair.find('=');
        if (equal_sign_pos != std::string::npos) {
            auto key = current_pair.substr(0, equal_sign_pos);
            auto value = current_pair.substr(equal_sign_pos + 1);
            result_map[key] = value;
        }
        i += len + 1;
    }
    return result_map;
}

std::map<std::string, std::variant<uint32_t, std::string>>
process_txt_record(const unsigned char *txt_rec, unsigned short txt_rec_len)
{
std::map<std::string, std::variant<uint32_t, std::string>> txtMap = 
    parse_txt_record(txt_rec, txt_rec_len);

    // Convert the map key.
    // TODO: process other keys as well.
    if (txtMap.find("flags") != txtMap.end()) {
        auto val = std::get<std::string>(txtMap["flags"]);
        txtMap["flags"] = std::stoi(val, nullptr, 16);
    }

    return txtMap;
}