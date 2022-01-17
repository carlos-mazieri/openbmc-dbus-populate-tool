
#include "config.h"
#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>

namespace local
{

boost::asio::io_service io;

const std::string const_service   {"service:"};
const std::string const_path      {"path:"};
const std::string const_interface {"interface:"};

std::string service_name{};
std::string path{};
std::string interface{};

std::shared_ptr<sdbusplus::asio::dbus_interface> iface  = nullptr;
std::shared_ptr<sdbusplus::asio::connection> systemBus = nullptr;

unsigned int path_counter{0};
unsigned int interface_counter{0};
unsigned int properties_counter{0};

//=======================================================================
/**
* @brief parse_line parses a single line and perform the service populate
* @param
* @return true OK, false some error
*/
//=======================================================================
template <typename CallbackTypeSet, typename CallbackTypeGet>
bool parse_line(const std::string& line,
                sdbusplus::asio::object_server& objServer,
                CallbackTypeSet && setFunction,
                CallbackTypeGet && getFunction)
{
    if (service_name.empty() == true && line.find(const_service) == 0)
    {
        service_name = line.substr(const_service.length());
        boost::algorithm::trim(service_name);
        try
        {
             systemBus->request_name(service_name.c_str());
        }
        catch (...)
        {
            fprintf(stderr, "ERROR: Could not create the service %s\n",
                       service_name.c_str());
            return false;
        }

    }
    else if (line.find(const_path) == 0)
    {
        path = line.substr(const_path.length());
        boost::algorithm::trim(path);
        path_counter++;
    }
    else if (line.find(const_interface) == 0)
    {
        interface = line.substr(const_interface.length());
        boost::algorithm::trim(interface);
        interface_counter++;
        if (iface != nullptr)
        {
            iface->initialize();
        }
        iface = objServer.add_interface(path, interface);
    }
    else if (service_name.empty() == false && path.empty() == false)
    {
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            auto property = line.substr(0, pos);
            auto value = line.substr(pos+1);
            boost::algorithm::trim(property);
            boost::algorithm::trim(value);

            bool ok = iface->register_property_rw(property,
                                  value,
                                  sdbusplus::vtable::property_::emits_change,
                                  setFunction,
                                  getFunction);

            if (ok == false)
            {
                fprintf(stderr, "ERROR: property=%s value=%s \n",
                           property.c_str(),value.c_str());
                return false;
            }
            else
            {
                properties_counter++;
            }
        }
    }
    return true;
}


} // local namespace


//=======================================================================
/**
 * @brief emulate_service creates a service, objects and properties
 * @param filename the file contains all the information
 * @return 0 OK, not zero some problem
 */
//=======================================================================
int emulate_service(char *filename)
{
    std::ifstream file(filename);
    if (file.is_open() == false)
    {
        fprintf(stderr, "Error: could not open the file '%s'\n", filename);
        return -ENOENT;
    }

    local::systemBus = std::make_shared<sdbusplus::asio::connection>(local::io);
    sdbusplus::asio::object_server objServer(local::systemBus);

    auto lambdaR =  [](const std::string & propvalue) -> std::string
                     {
                            return propvalue;
                     };
    auto lambdaRW =  [](const std::string & newValue,
                        const std::string & oldValue)
                     {
                         std::string & mz = const_cast<std::string&>(oldValue);
                         mz = newValue;
                         return 1;
                     };

    bool ok = true;
    std::string line;
    while (std::getline(file, line) && ok == true)
    {
        boost::algorithm::trim(line);
        if (line.empty() == true || line.at(0) == '#') {continue;}
        ok = local::parse_line(line, objServer, *lambdaRW, *lambdaR);
    }
    file.close();


    if (ok == true && local::iface != nullptr && local::interface_counter > 0)
    {
        printf("Service          : %s\n", local::service_name.c_str());
        printf("Object(s) Path(s): %u\n", local::path_counter);
        printf("Interfaces(s)    : %u\n", local::interface_counter);
        printf("Properties       : %u\n", local::properties_counter);
        local::iface->initialize();
        local::io.run();
        return 0;
    }

    fprintf(stderr, "INFO: service not started, no data or an error\n");
    return 1;
}
