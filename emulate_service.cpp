#include "config.h"
#include "methods_map.hpp"
#include "emulate_service.hpp"

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <sdbusplus/server/manager.hpp>
#include <boost/algorithm/string.hpp>

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <tuple>

#define HEXA_BASE 16
#define DECIMAL_BASE 10

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
std::vector<std::unique_ptr<sdbusplus::server::manager_t>> dbusManagedObjects;

unsigned int path_counter{0};
unsigned int interface_counter{0};
unsigned int properties_counter{0};
unsigned int method_counter{0};
unsigned int association_counter{0};

std::vector<std::string> method_list;


static int  serviceReadMethod = PROPERTY_READWRITE; // defaul allow changes

int64_t string2Uint64(const std::string& value)
{
    int64_t data = 0;
    if (value.empty() == true || value.find_first_of(' ') != std::string::npos)
    {
        return 0;
    }
    // first check for strings "true" and "false"
    bool strTrue = (value == "true" || value == "True" || value == "TRUE");
    bool strFalse = (value == "false" || value == "False" || value == "FALSE");
    if (strTrue == true || strFalse == true)
    {
        return strTrue == true ? 1 : 0; // 1 if value is one of Trues
    }
    else // any decimal or hexa value
    {
        if (value.at(0) == '0' && value.size() > 2 &&
            (value.at(1) == 'x' || value.at(1) == 'X'))
        {
            // Hexa first
            try
            {
                data = std::stoll(value, nullptr, HEXA_BASE);
            }
            catch (...)
            {
                data = 0;
            }
        }
        else
        {
            try
            {
                data = std::stoll(value, nullptr, DECIMAL_BASE);
            }
            catch (...)
            {
                data = 0;
            }
        }
    }
    return data;
}

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

template <class TYPE>
bool createArrayNumericProperty(const std::string property,
                                const std::string& value)
{
    std::vector<std::string> arrarStrings;
    boost::split(arrarStrings, value, boost::is_any_of("\t"));
    TYPE numericArray;
    for (auto& value : arrarStrings)
    {
        numericArray.push_back(string2Uint64(value));
    }
    return createProperty(property, numericArray);
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
            bool notified = iface->initialize(true);
            if (notified == false)
            {
                std::cerr << "Interface initialize ERROR: "
                          << iface->get_interface_name() << std::endl;
            }
        }
        // special Standard D-Bus interface, it needs Server Manager
        if (interface == "org.freedesktop.DBus.ObjectManager")
        {
            auto dbusManagedObjectInstance = std::make_unique<sdbusplus::server::manager_t>(*local::systemBus, path.c_str());
            local::dbusManagedObjects.push_back(std::move(dbusManagedObjectInstance));
            iface = nullptr;
        }
        else
        {
            iface = objServer.add_interface(path, interface);
        }
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
            std::string value =  sessions.at(2);
            auto type  = sessions.at(1);
            boost::algorithm::trim(property);
            boost::algorithm::trim(value);
            boost::algorithm::trim(type);
            if (value.empty())
            {
                value = std::string{"0"};
            }

            bool ok = false;
            switch (type.at(0))
            {
                case 'y': // uint8_t
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
                    int32_t intV = 0;
                    int tempV = 0;
                    if (sscanf(value.c_str(), "%d", &tempV) == 1)
                    {
                        intV = tempV;
                    }
                    ok = createProperty(property, intV);
                    break;
                }
                case 'u':  // uint32_t
                {
                    uint32_t intV = 0;
                    long unsigned int tempV = 0;
                    if (sscanf(value.c_str(), "%lu", &tempV) == 1)
                    {
                        intV = tempV;
                    }
                    ok = createProperty(property, intV);
                    break;
                }
                case 'x':  // int64_t
                {
                    int64_t intV = 0;
                    long long int tempV = 0;
                    if (sscanf(value.c_str(), "%lld", &tempV) == 1)
                    {
                        intV = tempV;
                    }
                    ok = createProperty(property, intV);
                    break;
                }
                case 't':  // uint64_t
                {
                    uint64_t intV = 0;
                    long long unsigned tempV = 0;
                    if (sscanf(value.c_str(), "%llu", &tempV) == 1)
                    {
                        intV = tempV;
                    }
                    ok = createProperty(property, intV);
                    break;
                }
                case 'd':  // double
                {
                    double intV = static_cast<double>(std::stod(value));
                    ok = createProperty(property, intV);
                    break;
                }
                case 'D': // array of doubles
                {
                    std::vector<std::string> arrayStrings;
                    boost::split(arrayStrings, value, boost::is_any_of("\t"));
                    std::vector<double> arrayDoubles;
                    for(auto& strDouble : arrayStrings)
                    {
                        double doubleValue = 0.0;
                        if (false == strDouble.empty())
                        {
                            doubleValue = static_cast<double>(std::stod(strDouble));
                        }
                        arrayDoubles.push_back(doubleValue);
                    }
                    ok = createProperty(property, arrayDoubles);
                    break;
                }
                case 'S': // array of strings
                {
                   std::vector<std::string> arrayStrings;
                   boost::split(arrayStrings, value, boost::is_any_of("\t"));
                   ok = createProperty(property, arrayStrings);
                   break;
                }
                case 'Y': // array of uint8_t
                {
                   ok = createArrayNumericProperty<std::vector<uint8_t>>
                       (property, value);
                   break;
                }
                case 'N': // array of int16_t
                {
                   ok = createArrayNumericProperty<std::vector<int16_t>>
                       (property, value);
                   break;
                }
                case 'Q': // array of uint16_t
                {
                   ok = createArrayNumericProperty<std::vector<uint16_t>>
                       (property, value);
                   break;
                }
                case 'I': // array of int32_t
                {
                   ok = createArrayNumericProperty<std::vector<int32_t>>
                       (property, value);
                   break;
                }
                case 'U': // array of uint32_t
                {
                   ok = createArrayNumericProperty<std::vector<uint32_t>>
                       (property, value);
                   break;
                }
                case 'X': // array of int64_t
                {
                   ok = createArrayNumericProperty<std::vector<int64_t>>
                       (property, value);
                   break;
                }
                case 'T': // array of uint64_t
                {
                   ok = createArrayNumericProperty<std::vector<uint64_t>>
                       (property, value);
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
        if (local::iface != nullptr)
        {
            local::iface->initialize();
        }
        local::io.run();
        return 0;
    }

    fprintf(stderr, "INFO: service not started, no data or an error\n");
    return 1;
}
