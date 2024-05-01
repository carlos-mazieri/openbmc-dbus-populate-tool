#pragma once

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <string>
#include <vector>
#include <tuple>

#define log_func()  std::cout << __func__ << "()" << std::endl

namespace local
{


using MethodMapSupported = std::unordered_map<std::string, int>;
using MethodsMap         = std::unordered_map<std::string, MethodMapSupported>;

/**
 * @brief isServiceSupported
 * @param service
 * @param path
 * @param interface
 * @param method
 *
 * @return true if that method is defined
 */
bool isMethodSupported(const std::string& service,
                        const std::string& path,
                        const std::string& interface,
                        const std::string& method);

bool registerMethod(std::shared_ptr<sdbusplus::asio::dbus_interface> iface,
                       const std::string& service,
                       const std::string& path,
                       const std::string& interface,
                       const std::string& method);

namespace methods
{
   // add methods here
} // namespace methods
} // namespace local
