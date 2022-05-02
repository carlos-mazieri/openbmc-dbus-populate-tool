#pragma once

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <string>

namespace local
{

bool registerMethod(std::shared_ptr<sdbusplus::asio::dbus_interface> iface,
                       const std::string& service,
                       const std::string& path,
                       const std::string& interface,
                       const std::string& method);

}
