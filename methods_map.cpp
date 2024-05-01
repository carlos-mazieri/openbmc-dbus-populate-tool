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

namespace local
{


// ===================== EXAMPLE maps DATA =====================================
const std::string gpuMgrKey = "xyz.openbmc_project.GpuMgr"
                              ":/xyz/openbmc_project/GpuMgr"
                              ":xyz.openbmc_project.GpuMgr.Server";


MethodMapSupported gpuMgrMethods = {
   {"DeviceGetData", 1},
   {"DeviceClearData", 1},
   {"DeviceSetData", 1},
   {"PassthroughFpga", 1}
};

//--
const std::string objectMapperKey = "xyz.openbmc_project.ObjectMapper"
                                    ":/xyz/openbmc_project/object_mapper"
                                    ":xyz.openbmc_project.ObjectMapper";
const  MethodMapSupported objectMapperMethods = {
    {"GetObject", 1},
};
//--

MethodsMap methodsMap =
{
  //  {gpuMgrKey, gpuMgrMethods},
  //  {objectMapperKey, objectMapperMethods}
};

// ===================== END maps DATA example =================================
} //   namespace local


bool local::isMethodSupported(const std::string& service,
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

bool
local::registerMethod(std::shared_ptr<sdbusplus::asio::dbus_interface> iface,
                    const std::string& service,
                    const std::string& path,
                    const std::string& interface,
                    const std::string& method)
{
    (void) iface; // use iface->register_method
    bool ret = isMethodSupported(service, path, interface, method);

    if (ret == true)
    {
        // I do not see a way to make this generic way
        if (method == "DeviceGetData")
        {
            // example, methods::DeviceGetData() is not defined
            // ret = iface->register_method(method, &methods::DeviceGetData);
        }
    }

    return ret;
}
