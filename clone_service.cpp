#include "config.h"

#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

#include <map>
#include <string>

using DbusObjectPath = std::string;
using DbusService = std::string;
using DbusInterface = std::string;
using DbusObjectInfo = std::pair<DbusObjectPath, DbusService>;
using DbusProperty = std::string;

using Association = std::tuple<std::string, std::string, std::string>;

using Value = std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
                           int64_t, uint64_t, double, std::string,
                           std::vector<std::string>, std::vector<Association>>;

using PropertyMap = std::map<DbusProperty, Value>;

using ObjectTree =
    std::map<DbusObjectPath, std::map<DbusService, std::vector<DbusInterface>>>;

using InterfaceList = std::vector<std::string>;

using DbusInterfaceMap = std::map<DbusInterface, PropertyMap>;

using ObjectValueTree =
    std::map<sdbusplus::message::object_path, DbusInterfaceMap>;

// property   value
using PropertyStringMap   = std::map<DbusProperty, std::string>;
using InterfaceProperties = std::map<DbusInterface, PropertyStringMap>;
using ObjectPropertyStringValue = std::map<DbusObjectPath, InterfaceProperties>;


namespace
{

 sdbusplus::bus::bus _bus = sdbusplus::bus::new_default_system();

/**
 * @brief variantToString
 * @param variantVar
 * @return
 */
std::string variantToString(const Value &variantVar)
{
    std::string value;
    if (std::holds_alternative<std::string>(variantVar))
    {
        value = std::get<std::string>(variantVar);
    }
    else if (std::holds_alternative<bool>(variantVar))
    {
        auto boolean = std::get<bool>(variantVar);
        value = boolean == true ? "true" : "false";
    }
    else if (std::holds_alternative<uint8_t>(variantVar))
    {
        value = std::to_string(std::get<uint8_t>(variantVar));
    }
    else if (std::holds_alternative<int16_t>(variantVar))
    {
        value = std::to_string(std::get<int16_t>(variantVar));
    }
    else if (std::holds_alternative<uint16_t>(variantVar))
    {
        value = std::to_string(std::get<uint16_t>(variantVar));
    }
    else if (std::holds_alternative<int32_t>(variantVar))
    {
        value = std::to_string(std::get<int32_t>(variantVar));
    }
    else if (std::holds_alternative<uint32_t>(variantVar))
    {
        value = std::to_string(std::get<uint32_t>(variantVar));
    }
    else if (std::holds_alternative<int64_t>(variantVar))
    {
        value = std::to_string(std::get<int64_t>(variantVar));
    }
    else if (std::holds_alternative<uint64_t>(variantVar))
    {
        value = std::to_string(std::get<uint64_t>(variantVar));
    }
    else if (std::holds_alternative<double>(variantVar))
    {
        value = std::to_string(std::get<double>(variantVar));
    }
    else if (std::holds_alternative<std::vector<std::string>>(variantVar))
    {
        std::vector<std::string> array =
                std::get<std::vector<std::string>>(variantVar);
        auto counter = array.size();
        if (counter > 0)
        {
            value = array.at(0);
            for (counter = 1; counter < array.size(); ++counter)
            {
                value += ' ' + array.at(counter);
            }
        }
    }
    return value;
}

/**
 * @brief readXml
 * @param is
 * @param interfaceMatch
 * @param ifs
 * @return
 */
std::vector<std::string>
readXml(std::istream& is,
                    const std::string& interfaceMatch,
                    std::vector<std::string> *ifs  /*out*/ )
{
    // populate tree structure pt
    using boost::property_tree::ptree;
    ptree pt;
    read_xml(is, pt);
    // traverse pt
    std::vector<std::string> children;
    const ptree & formats =  pt.get_child("node");
    BOOST_FOREACH(ptree::value_type const& f, formats)
    {
        if (f.first == "node")
        {
            std::string child = f.second.get("<xmlattr>.name", "");
            if (child.empty() == false)
            {
                children.push_back(child);
            }
        }
        else if (ifs != nullptr && f.first == "interface")
        {
           std::string interface = f.second.get("<xmlattr>.name", "");
           if (   (interfaceMatch.empty() == true
                     || interface.find(interfaceMatch) != std::string::npos)
               && (interface.find("org.freedesktop.DBus.", 0) != 0))
           {
               ifs->push_back(interface);
           }

        }
    }
    return children;
}


/**
 * @brief printObjectPropertyStringValue
 * @param tree
 */
void printObjectPropertyStringValue(const ObjectPropertyStringValue &tree)
{
    for (auto & item : tree)
    {
        std::string subpath = item.first;
        printf("\npath:%s\n", subpath.c_str());
        for (auto &intf : item.second)
        {
            std::string interface = intf.first;
            printf("\tinterface:%s\n", interface.c_str());
            for (auto& prop : intf.second)
            {
                std::string property = prop.first;
                std::string value = prop.second;
                printf("\t\t%-30s = %s\n ",
                       property.c_str(), value.c_str());
            }
        }
    }
}


/**
 * @brief copyPropertyMapToPropertyStringMap
 * @param valuesProp
 * @param strProp
 */
void copyPropertyMapToPropertyStringMap(const PropertyMap &valuesProp,
                                        PropertyStringMap *strProp)
{
    for (auto& prop : valuesProp)
    {
        std::string name = prop.first;
        Value val = prop.second;
        std::string value = variantToString(val);
        (*strProp)[name] = value;
    }
}


/**
 * @brief getProperties
 * @param service
 * @param path
 * @param interface
 * @return
 */
PropertyMap
getProperties(const std::string &service,
              const std::string &path,
              const std::string &interface)
{
    PropertyMap properties;
    auto method = _bus.new_method_call(service.c_str(), path.c_str(),
                                         "org.freedesktop.DBus.Properties",
                                         "GetAll");
    method.append(interface);
    try
    {
        auto reply = _bus.call(method);
        reply.read(properties);

    } catch (...)
    {
        // Empty
    }
    return properties;
}


/**
 * @brief Dbus::objectTreeValues
 * @param service
 * @param path
 * @param interfaceMatch
 * @param tree
 */
void objectTreeValues(const std::string &service,
                      const std::string &path,
                      const std::string &interfaceMatch,
                      ObjectPropertyStringValue *tree)
{
    auto msg = _bus.new_method_call(service.c_str(), path.c_str(),
                                    "org.freedesktop.DBus.Introspectable",
                                    "Introspect");
    try
    {
        std::string xml;
        auto response = _bus.call(msg);
        response.read(xml);
        std::istringstream xml_stream(xml);
//        printf("%s\n", xml.c_str());
        std::vector<std::string> interfaceList;
        auto children = readXml(xml_stream, interfaceMatch, &interfaceList);
        PropertyStringMap stringValues;
        InterfaceProperties interfaceData;
        for (const auto & intf_name : interfaceList)
        {
            auto propertiesValues = getProperties(service, path, intf_name);
            if (propertiesValues.empty() == false)
            {
                copyPropertyMapToPropertyStringMap(propertiesValues,
                                                   &stringValues);
                interfaceData[intf_name] = stringValues;
            }
        }
        if (interfaceData.empty() == false)
        {
            (*tree)[path] = interfaceData;
        }
        for (auto const& child:  children)
        {
            std::string child_path(path);
            child_path += '/' + child;
            objectTreeValues(service, child_path, interfaceMatch, tree);
        }
    }
    catch (...)
    {
        // Empty
    }
}


/**
 * @brief Dbus::objectTreeValues
 * @param service
 * @param path
 * @param interface
 * @return
 */
ObjectPropertyStringValue
objectTreeValues(const std::string &service,
                 const std::string &path,
                 const std::string &interface = {})
{
    ObjectPropertyStringValue tree;
    objectTreeValues(service, path, interface, &tree);
    printf("service: %s\n",  service.c_str());
    printObjectPropertyStringValue(tree);

    return tree;
}


} // namespace


int clone_service(int argc, char *argv[])
{
    std::string service{argv[2]};
    std::string path = {"/"};
    if (argc == 4)
    {
        path = argv[3];
    }
    else
    {
        for (decltype(service.size()) counter = 0; counter < service.size(); ++counter)
        {
            path.push_back (service.at(counter) != '.' ?
                                   service.at(counter) : '/') ;
        }
    }
    objectTreeValues(service, path);
    return 0;
}
