
#include "methods_map.hpp"

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <map>
#include <vector>
#include <tuple>
#include <array>
#include <iostream>

#include <stdlib.h>

namespace local
{
constexpr auto serviceGpuMgr = "xyz.openbmc_project.GpuMgr";
constexpr auto mapperService = "xyz.openbmc_project.ObjectMapper";
constexpr auto gpioStatusService = "xyz.openbmc_project.GpioStatusHandler";

constexpr auto pathGrpuMgr   = "/xyz/openbmc_project/GpuMgr";
constexpr auto intGpuMggServer = "xyz.openbmc_project.GpuMgr.Server";



constexpr auto methodDeviceGetData  = "DeviceGetData";
constexpr auto paramOverTemperatureInfo  =
        "gpu.thermal.temperature.overTemperatureInfo";

std::tuple<int, std::string, std::vector<uint32_t>>
DeviceGetData(int index, const std::string& callParam, int access)
{
    std::string ret{"none"};
    std::vector<uint32_t> array;
    (void) access;
    (void) index;
    if (callParam == paramOverTemperatureInfo)
    {
        ret = "Baseboard GPU over temperature info : ";
        unsigned int randomBit = random() % 8;
        unsigned int bitmask = 1 << index | 1 << randomBit;
        char hexValue[32];
        ::snprintf(hexValue, sizeof(hexValue) -1, "%04x", bitmask);
        ret += hexValue;
        array.push_back((uint32_t)bitmask);
        array.push_back((uint32_t)0);
        std::cout << "DeviceGetData()" << " bitmask:" << bitmask
                  << " hexValue:" << hexValue
                  << " ret:" << ret
                  << " " << array.at(0) << " " << array.at(1)
                  << std::endl;
    }
    return std::make_tuple(0, ret, array);
}

std::map<std::string,  std::vector<std::string>>
MapperGetObject(const std::string& objectPath,
                std::vector<std::string>& /*interfaces*/)
{
    const std::string& service =
         objectPath.find("GpioStatusHandler") != std::string::npos ?
             gpioStatusService :  serviceGpuMgr;

    std::vector<std::string> services{service};
    std::map<std::string,  std::vector<std::string>> ret;
    ret[service] = services;
    return ret;
}


bool registerMethod(std::shared_ptr<sdbusplus::asio::dbus_interface> iface,
                    const std::string& service,
                    const std::string& path,
                    const std::string& interface,
                    const std::string& method)
{
    bool ret = false;
    if (service == serviceGpuMgr)
    {
        if (path == pathGrpuMgr && interface == intGpuMggServer)
        {
            if (method == methodDeviceGetData)
            {
                ret = iface->register_method(method, DeviceGetData);
            }
        }
    }
    else if (service == mapperService)
    {
        ret = iface->register_method(method, MapperGetObject);
    }
    if (ret == false)
    {
        std::cerr << "Not able to create the method: "
                  << method << std::endl;
    }
    return ret;
}

}
