
#include "methods_map.hpp"

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <array>
#include <iostream>

#include <stdlib.h>
#include <time.h>

#define log_func()  std::cout << __func__ << "()" << std::endl

namespace local
{

using MethodMapSupported = std::unordered_map<std::string, int>;
using MethodsMap         = std::unordered_map<std::string, MethodMapSupported>;

/**
 *  These keys here are just examples
 */
const std::string exampleKey = "xyz.openbmc_project.Example"
                              ":/xyz/openbmc_project/Eample"
                              ":xyz.openbmc_project.Example.Server";
const std::string objectMapperKey = "xyz.openbmc_project.ObjectMapper"
                                    ":/xyz/openbmc_project/object_mapper"
                                    ":xyz.openbmc_project.ObjectMapper";

MethodMapSupported exampleMethods =
  {
    {"ExampleMethod", 1},
    {"GetObject", 1}
  };


MethodsMap methodsMap =
{
 // uncomment the below line to have a dummy GetObject() method for ObjecMapper
 //   {objectMapperKey, objectMapperMethods}
     {exampleKey, exampleMethods}
};



namespace methods
{
 /** Just an example of implementation using xyz.openbmc_project.ObjectMapper service and method GetObject

   # busctl introspect xyz.openbmc_project.ObjectMapper /xyz/openbmc_project/object_mapper
    NAME                                TYPE      SIGNATURE RESULT/VALUE FLAGS
    org.freedesktop.DBus.Introspectable interface -         -            -
    .Introspect                         method    -         s            -
    org.freedesktop.DBus.Peer           interface -         -            -
    .GetMachineId                       method    -         s            -
    .Ping                               method    -         -            -
    org.freedesktop.DBus.Properties     interface -         -            -
    .Get                                method    ss        v            -
    .GetAll                             method    s         a{sv}        -
    .Set                                method    ssv       -            -
    .PropertiesChanged                  signal    sa{sv}as  -            -
    xyz.openbmc_project.ObjectMapper    interface -         -            -
    .GetAncestors                       method    sas       a{sa{sas}}   -
    .GetObject                          method    sas       a{sas}       -
    .GetSubTree                         method    sias      a{sa{sas}}   -
    .GetSubTreePaths                    method    sias      as           -
 */
std::map<std::string,  std::vector<std::string>>
MapperGetObject(const std::string& objectPath,
                std::vector<std::string>& /*interfaces*/)
{
    // suppose there are few services involved in my environment:
    //  xyz.openbmc_project.GpioDeviceService which object-paths have a pattern
    //  if that pattern does not match then uses a default service
    const std::string& service =
         objectPath.find("GpioDeviceItem") != std::string::npos
            ?  "xyz.openbmc_project.GpioDeviceService" \
            :  "xyz.openbmc_project.MyDefaultService";

    std::vector<std::string> services{service};
    std::map<std::string,  std::vector<std::string>> ret;
    ret[service] = services;
    return ret;
}

} // namespace methodsMap



bool isMethodSupported(const std::string& service,
                       const std::string& path,
                       const std::string& interface,
                       const std::string& method)
{
  constexpr auto colon = ":";
  std::string key = service + colon + path + colon + interface;
  bool ret = methodsMap.count(key) != 0 ;
  if (ret == true)
  {
      auto& mMap = methodsMap[key];
      ret = mMap.count(method) != 0;
  }
  return ret;
}

bool registerMethod(std::shared_ptr<sdbusplus::asio::dbus_interface> iface,
                    const std::string& service,
                    const std::string& path,
                    const std::string& interface,
                    const std::string& method)
{
    bool ret = isMethodSupported(service, path, interface, method);

    if (ret == true)
    {
        // just examples how to define and register methods
        if (method == "GetObject")
        {
            ret = iface->register_method(method, &methods::MapperGetObject);
            std::cout << "iface->register_method(" << method  <<") ret="
                      << ret << std::endl;
        }
        else if  (method == "ExampleMethod")
        {
            // register here
        }
        else
        {
            std::cerr << "Not able to create the method: "
                      << method << std::endl;
        }
    }

    return ret;
}

}
