#include "config.h"
#include "methods_map.hpp"
#include "emulate_service.hpp"

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <tuple>

namespace local
{

boost::asio::io_service io;

const std::string const_service   {"service:"};
const std::string const_path      {"path:"};
const std::string const_interface {"interface:"};
const std::string const_method    {"method:"};

std::string service_name{};
std::string path{};
std::string interface{};
std::string method{};

std::shared_ptr<sdbusplus::asio::dbus_interface> iface  = nullptr;
std::shared_ptr<sdbusplus::asio::connection> systemBus = nullptr;

unsigned int path_counter{0};
unsigned int interface_counter{0};
unsigned int properties_counter{0};
unsigned int method_counter{0};
unsigned int association_counter{0};

std::vector<std::string> method_list;


static int  serviceReadMethod = PROPERTY_READWRITE; // defaul allow changes

template <typename PropertyType>
bool createProperty(const std::string& propName, PropertyType& value)
{
    auto flagsMethod = sdbusplus::asio::PropertyPermission::readWrite;
    if (serviceReadMethod == PROPERTY_READONLY)
    {
        flagsMethod = sdbusplus::asio::PropertyPermission::readOnly;
    }
    bool ok = iface->register_property(propName, value, flagsMethod);
    return ok;
}


struct AssociationInfo
{
    std::vector<std::tuple<std::string, std::string, std::string>> association;
    bool active;
    std::string property;

    AssociationInfo() : active(false)
    {

    }
    void store(std::vector<std::string>& array)
    {
        while (array.size() < 3)
        {
            array.push_back(std::string{""});
        }
        association.push_back(
                    std::make_tuple(array.at(0),
                                    array.at(1),
                                    array.at(2)));
    }
    void clear()
    {
        association.erase(association.begin(), association.end());
        active = false;
    }
    bool setProperty(const std::string& prop)
    {
        clear();
        property = prop;
        active = true;
        return active;
    }
    bool create()
    {
        bool ok = active;

        if (ok == true)
        {
            std::cout << "\t\tassociation [" << ++association_counter << "] "
                      << path << " " << interface << " Associations" << std::endl;
            // even empty it is necessary to create Association
            ok = createProperty(property, association);
        }
        clear();
        return ok;
    }
};


AssociationInfo association;


//=======================================================================
/**
* @brief parse_line parses a single line and perform the service populate
* @param
* @return true OK, false some error
*/
//=======================================================================
bool parse_line(const std::string& line,
                sdbusplus::asio::object_server& objServer)
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

    if (association.active == true)
    {
        auto pos = line.find("(sss):");
        if (pos != std::string::npos)
        {
            std::vector<std::string> sessions;
            boost::split(sessions, line, boost::is_any_of(":"));
            auto value =  sessions.at(1);
            boost::algorithm::trim(value);
            std::vector<std::string> arrayStrings;
            boost::split(arrayStrings, value, boost::is_any_of("\t"));
            association.store(arrayStrings);
            return true;
        }
        else
        {
            association.create();
        }
    }

    if (line.find(const_path) == 0)
    {
        path = line.substr(const_path.length());
        boost::algorithm::trim(path);
        path_counter++;
#if !defined(DEFAULT_SERVICE_FILE)
        std::cout << "creating object path [" << path_counter << "] " <<
                     path << std::endl;
#endif
    }
    else if (line.find(const_interface) == 0)
    {
        interface = line.substr(const_interface.length());
        boost::algorithm::trim(interface);
        interface_counter++;
#if !defined(DEFAULT_SERVICE_FILE)
        std::cout << "\tinterface[" << interface_counter << "] "
                  << path << " " << interface << std::endl;
#endif
        if (iface != nullptr)
        {
            iface->initialize();
        }
        iface = objServer.add_interface(path, interface);
    }
    else if (line.find(const_method) == 0)
    {
        method = line.substr(const_method.length());
        boost::algorithm::trim(method);
        if (registerMethod(iface, service_name, path, interface, method))
        {
            method_counter++;
            method_list.push_back(path + ":" + interface + ":" + method);
        }
    }
    else if (service_name.empty() == false && path.empty() == false)
    {
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::vector<std::string> sessions;
            boost::split(sessions, line, boost::is_any_of("="));
            if (sessions.size() < 3)
            {
                return true;
            }
            auto property = sessions.at(0);
            auto value =  sessions.at(2);
            auto type  = sessions.at(1);
            boost::algorithm::trim(property);
            boost::algorithm::trim(value);
            boost::algorithm::trim(type);

            bool ok = false;
            switch (type.at(0))
            {
                case 'y': // int8_t
                {
                    uint8_t byte = static_cast<uint8_t>(std::stoi(value));
                    ok = createProperty(property, byte);
                    break;
                }
                case 'b': // bool
                {
                    bool boolean =
                        (value == "yes"|| value == "Yes" || value == "YES") ?
                                            true : false;
                    ok = createProperty(property, boolean);
                    break;
                }
                case 'n': // int16_t
                {
                    int16_t intV = static_cast<int16_t>(std::stol(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'q':  // uint16_t
                {
                    uint16_t intV = static_cast<uint16_t>(std::stol(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'i':  // int32_t
                {
                    int32_t intV = static_cast<int32_t>(std::stoll(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'u':  // uint32_t
                {
                    uint32_t intV = static_cast<uint32_t>(std::stoll(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'x':  // int64_t
                {
                    int64_t intV = static_cast<int64_t>(std::stoll(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 't':  // uint64_t
                {
                    uint64_t intV = static_cast<uint64_t>(std::stoll(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'd':  // double
                {
                    double intV = static_cast<double>(std::stod(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'S': // array if strings
                {
                   std::vector<std::string> arrarStrings;
                   boost::split(arrarStrings, value, boost::is_any_of("\t"));
                   ok = createProperty(property, arrarStrings);
                   break;
                }
                case 'A':
                {
                   ok = association.setProperty(property);
                   break;
                }
                case 's':
                default:
                {
                    ok = createProperty(property, value);
                    break;
                }
            }
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
int emulate_service(const char *filename, int serviceMethod)
{
    local::serviceReadMethod = serviceMethod;

    std::ifstream file(filename);
    if (file.is_open() == false)
    {
        fprintf(stderr, "Error: could not open the file '%s'\n", filename);
        return -ENOENT;
    }

    local::systemBus = std::make_shared<sdbusplus::asio::connection>(local::io);
    sdbusplus::asio::object_server objServer(local::systemBus);

    bool ok = true;
    std::string line;
    //int line_counter = 1;
    while (std::getline(file, line) && ok == true)
    {
        boost::algorithm::trim(line);
      //  std::cout <<  "line:" << line_counter++ << std::endl;
        if (line.empty() == true || line.at(0) == '#') {continue;}
        ok = local::parse_line(line, objServer);
    }
    local::association.create(); // if the Association was the last property
    file.close();

    if (ok == true && local::iface != nullptr && local::interface_counter > 0)
    {
        printf("Service          : %s\n", local::service_name.c_str());
        printf("Object(s) Path(s): %u\n", local::path_counter);
        printf("Interface(s)     : %u\n", local::interface_counter);
        printf("Properties       : %u\n", local::properties_counter);

        std::string display_method_list{"0"};
        if (local::method_counter > 0)
        {
            display_method_list.clear();
            for (auto it = local::method_list.begin();
                 it != local::method_list.end(); ++it)
            {
                display_method_list += *it + " ";
            }
        }
        printf("Methods          : %s\n", display_method_list.c_str());

        fflush(stdout);
        local::iface->initialize();
        local::io.run();
        return 0;
    }

    fprintf(stderr, "INFO: service not started, no data or an error\n");
    return 1;
}
