#pragma once

#include <string>
#include <cstdint>
#include <map>
#include <variant>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

class address final {
public:
    address(uint32_t address, uint16_t port,
            std::map<std::string, std::variant<uint32_t, std::string>> txt) :
        _address(address),
        _port(port),
        _txt(txt)
    {}
    address(uint32_t address, uint16_t port) :
        _address(address),
        _port(port)
    {}
    address(std::string address, uint16_t port) :
        _address(parse_address(address)),
        _port(port)
    {}
    address(std::string address, uint16_t port,
            std::map<std::string, std::variant<uint32_t, std::string>> txt) :
        _address(parse_address(address)),
        _port(port),
        _txt(txt)
    {}
    virtual ~address() = default;

    static address make_from_network(const sockaddr_in& sock_addr);
    static address make_from_host(const sockaddr_in& sock_addr);

    uint32_t get_address() const;
    uint16_t get_port() const;
    std::map<std::string, std::variant<uint32_t, std::string>> get_txt() const;
    std::string get_printable_address() const;

    sockaddr_in to_sockaddr_in() const;

private:
    const uint32_t _address;
    const uint16_t _port;
    const std::map<std::string, std::variant<uint32_t, std::string>> _txt;

    static uint32_t parse_address(std::string address);
};
