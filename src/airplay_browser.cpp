#include <iostream>
#include <exception>
#include <map>
#include <variant>
#include <netdb.h>
#include <dns_sd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "common.hpp"
#include "airplay_browser.hpp"
#ifdef AVAHI_COMPAT
#include "safe_socket.hpp"
#endif

const std::string airplay_browser::AIRPLAY_REGTYPE = "_airplay._tcp";

std::map<std::string, std::variant<uint32_t, std::string>>
process_txt_record(const unsigned char *txt_rec, unsigned short txt_rec_len);

airplay_browser::airplay_browser() {
    CHECK_AND_THROW(kDNSServiceErr_NoError == DNSServiceBrowse(&_dns_browse_ref,
                                                               static_cast<DNSServiceFlags>(0),
                                                               kDNSServiceInterfaceIndexAny,
                                                               AIRPLAY_REGTYPE.c_str(),
                                                               nullptr,
                                                               reinterpret_cast<DNSServiceBrowseReply>(&browse_callback),
                                                               this),
                    "DNSServiceBrowse failed");
#ifdef AVAHI_COMPAT
    safe_socket::set_nonblocking(DNSServiceRefSockFD(_dns_browse_ref));
#endif
}

airplay_browser::~airplay_browser() {
    try {
        DNSServiceRefDeallocate(_dns_browse_ref);
    }
    catch (...) {
        PRINT_ANY_EXCEPTION();
    }
}

std::map<std::string, address> airplay_browser::get_devices() {
    airplay_browser browser;
    browser.wait_for_devices();
    return browser._devices;
}

void airplay_browser::browse_callback(DNSServiceRef sdRef,
                                      DNSServiceFlags flags,
                                      uint32_t interfaceIndex,
                                      DNSServiceErrorType errorCode,
                                      const char *serviceName,
                                      const char *regtype,
                                      const char *replyDomain,
                                      airplay_browser *context){
    CHECK_AND_THROW(kDNSServiceErr_NoError == errorCode, "browse_callback failed");

    context->_current_resolved_service_name = std::string(serviceName);

    DNSServiceRef dns_resolve_ref = nullptr;

#ifdef AVAHI_COMPAT
    if (flags != kDNSServiceFlagsAdd) // Only support Add for now!
       return;
    flags = 0; // Otherwise the Avahi compatibility layer just chokes on MDNServiceResolve().
#endif

    if(kDNSServiceErr_NoError != DNSServiceResolve(&dns_resolve_ref,
                                                   flags,
                                                   interfaceIndex,
                                                   serviceName,
                                                   regtype,
                                                   replyDomain,
                                                   reinterpret_cast<DNSServiceResolveReply>(&resolve_callback),
                                                   context)){
        std::cerr << "Could not resolve service" << std::endl;
        return;
    }

#ifdef AVAHI_COMPAT
    int socket = DNSServiceRefSockFD(dns_resolve_ref);
    safe_socket::set_nonblocking(socket);
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    while (1) {
        if(select(socket+1, &read_fds, NULL, NULL, NULL) < 0) {
          perror("select");
        }
        DNSServiceProcessResult(dns_resolve_ref);
        if (!context->_devices.empty()) /* FIXME: this works only the first time! */
            break;
    }
#else
    DNSServiceProcessResult(dns_resolve_ref);
#endif
    DNSServiceRefDeallocate(dns_resolve_ref);
}

void airplay_browser::resolve_callback(DNSServiceRef sdRef,
                                       DNSServiceFlags flags,
                                       uint32_t interfaceIndex,
                                       DNSServiceErrorType errorCode,
                                       const char *fullname,
                                       const char *hosttarget,
                                       uint16_t port,
                                       uint16_t txtLen,
                                       const unsigned char* txtRecord,
                                       airplay_browser *context){
    CHECK_AND_THROW(kDNSServiceErr_NoError == errorCode, "resolve_callback failed");

    const struct hostent* host = gethostbyname(hosttarget);
    const auto addr_list = reinterpret_cast<struct in_addr**>(host->h_addr_list);

    auto txtMap = process_txt_record(txtRecord, txtLen);
    const std::pair<std::string, address> device_info(context->_current_resolved_service_name,
                                                      address(ntohl(static_cast<uint32_t>((addr_list[0])->s_addr)),
                                                              ntohs(port), txtMap));
    context->_devices.insert(device_info);

    context->_current_resolved_service_name = "";
}

void airplay_browser::wait_for_devices() {
#ifdef AVAHI_COMPAT
    int socket = DNSServiceRefSockFD(_dns_browse_ref);

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(socket, &read_fds);

    while(1) {
        if(select(socket+1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select");
        }
#endif
    CHECK_AND_THROW(kDNSServiceErr_NoError == DNSServiceProcessResult(_dns_browse_ref),
                    "Error processing mDNS daemon response")
#ifdef AVAHI_COMPAT
        if (!_devices.empty()) // FIXME: this works only for the first time!
            break;
    }
#endif
}
