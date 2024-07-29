#include "config.h"
#include "methods_map.hpp"
#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>

#include <map>
#include <unordered_map>
#include <string>

using DbusObjectPath = std::string;
using DbusService = std::string;
using DbusInterface = std::string;
using DbusObjectInfo = std::pair<DbusObjectPath, DbusService>;
using DbusProperty = std::string;

using Association = std::tuple<std::string, std::string, std::string>;

using Value = std::variant<bool, uint8_t, int16_t, uint16_t, int32_t, uint32_t,
                           int64_t, uint64_t, double, std::string,
                           std::vector<std::string>, std::vector<Association>,
                           std::vector<uint8_t>, std::vector<double>,
                           std::vector<int16_t>, std::vector<uint16_t>,
                           std::vector<int32_t>, std::vector<uint32_t>,
                           std::vector<int64_t>, std::vector<uint64_t>>;

using PropertyMap = std::unordered_map<DbusProperty, Value>;

using ObjectTree =
    std::unordered_map<DbusObjectPath, std::unordered_map<DbusService, std::vector<DbusInterface>>>;

using InterfaceList = std::vector<std::string>;

using DbusInterfaceMap = std::unordered_map<DbusInterface, PropertyMap>;

using ObjectValueTree =
    std::unordered_map<sdbusplus::message::object_path, DbusInterfaceMap>;

// property   value
using PropertyStringMap   = std::unordered_map<DbusProperty, std::string>;
using InterfaceProperties = std::unordered_map<DbusInterface, PropertyStringMap>;
using ObjectPropertyStringValue = std::unordered_map<DbusObjectPath, InterfaceProperties>;


struct  MethoArgs
{
    std::string arg;
    std::string type;
    std::string direction;
};

using MethodArgsList = std::vector<MethoArgs>;
using MethodArgsMap = std::unordered_map<std::string, MethodArgsList>;
                            /*interface*/
using InterfaceMethodMap =  std::unordered_map<std::string, MethodArgsMap>;


InterfaceMethodMap interfaceMethodMap;

namespace
{

/** it will have only objects which have Associations */
ObjectPropertyStringValue objectsAssociationMap;

 sdbusplus::bus::bus _bus = sdbusplus::bus::new_default();

template <class TYPE>
std::string declareArray(const Value& variantVar, char declarType)
{
    TYPE array = std::get<TYPE>(variantVar);
    std::string value{declarType};
    value += " = ";
    auto counter = array.size();
    if (counter > 0)
    {
        value += std::to_string(array.at(0));
        for (counter = 1; counter < array.size(); ++counter)
        {
            value += '\t' + std::to_string(array.at(counter));
        }
    }
    return value;
}

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
        value = "s = " + std::get<std::string>(variantVar);
    }
    else if (std::holds_alternative<bool>(variantVar))
    {
        auto boolean = std::get<bool>(variantVar);
        value = "b = ";
        value += boolean == true ? "true" : "false";
    }
    else if (std::holds_alternative<uint8_t>(variantVar))
    {
        value = value = "y = " +std::to_string(std::get<uint8_t>(variantVar));
    }
    else if (std::holds_alternative<int16_t>(variantVar))
    {
        value = value = "n = " +std::to_string(std::get<int16_t>(variantVar));
    }
    else if (std::holds_alternative<uint16_t>(variantVar))
    {
        value = value = "q = " +std::to_string(std::get<uint16_t>(variantVar));
    }
    else if (std::holds_alternative<int32_t>(variantVar))
    {
        value = value = "i = " +std::to_string(std::get<int32_t>(variantVar));
    }
    else if (std::holds_alternative<uint32_t>(variantVar))
    {
        value = value = "u = " + std::to_string(std::get<uint32_t>(variantVar));
    }
    else if (std::holds_alternative<int64_t>(variantVar))
    {
        value = value = "x = " +std::to_string(std::get<int64_t>(variantVar));
    }
    else if (std::holds_alternative<uint64_t>(variantVar))
    {
        value = value = "t = " +std::to_string(std::get<uint64_t>(variantVar));
    }
    else if (std::holds_alternative<double>(variantVar))
    {
        value = value = "d = " +std::to_string(std::get<double>(variantVar));
    }
    else if (std::holds_alternative<std::vector<std::string>>(variantVar))
    {
        std::vector<std::string> array =
                std::get<std::vector<std::string>>(variantVar);
        value = "S = ";
        auto counter = array.size();
        if (counter > 0)
        {
            value += array.at(0);
            for (counter = 1; counter < array.size(); ++counter)
            {
                value += '\t' + array.at(counter);
            }
        }
    }
    else if (std::holds_alternative<std::vector<uint8_t>>(variantVar))
    {
        value = declareArray<std::vector<uint8_t>>(variantVar, 'Y');
    }
    else if (std::holds_alternative<std::vector<int16_t>>(variantVar))
    {
        value = declareArray<std::vector<int16_t>>(variantVar, 'N');
    }
    else if (std::holds_alternative<std::vector<uint16_t>>(variantVar))
    {
        value = declareArray<std::vector<uint16_t>>(variantVar, 'Q');
    }
    else if (std::holds_alternative<std::vector<int32_t>>(variantVar))
    {
        value = declareArray<std::vector<int32_t>>(variantVar, 'I');
    }
    else if (std::holds_alternative<std::vector<uint32_t>>(variantVar))
    {
        value = declareArray<std::vector<uint32_t>>(variantVar, 'U');
    }
    else if (std::holds_alternative<std::vector<int64_t>>(variantVar))
    {
        value = declareArray<std::vector<int64_t>>(variantVar, 'X');
    }
    else if (std::holds_alternative<std::vector<uint64_t>>(variantVar))
    {
        value = declareArray<std::vector<uint64_t>>(variantVar, 'T');
    }
    else if (std::holds_alternative<std::vector<double>>(variantVar))
    {
        value = declareArray<std::vector<double>>(variantVar, 'D');
    }
    else if(std::holds_alternative<std::vector<Association>>(variantVar))
    {
        std::vector<Association> associations =
                std::get<std::vector<Association>>(variantVar);
        value = "A = ";
        for (auto& tuple_strings : associations)
        {
           auto str0 = std::get<0>(tuple_strings);
           auto str1 = std::get<1>(tuple_strings);
           auto str2 = std::get<2>(tuple_strings);
           value += "\n\t\t\t(sss): " + str0 + '\t' + str1 + '\t' + str2;
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
            // ignore "org.freedesktop.DBus." because it is the default
            if (interface.find("org.freedesktop.DBus.", 0) == 0)
            {
                if (interface != "org.freedesktop.DBus.ObjectManager")
                {
                    // Standard D-Bus interface, it needs special handling
                    continue;
                }
            }
            /**
             *  default is to ignore Associations but
             *     meson -Dkeep-association-definition can used
             */

#ifdef IGNORE_ASSOCIATIONS
            if (interface == "xyz.openbmc_project.Association.Definitions")
            {
                continue;
            }
#endif
           if (interfaceMatch.empty() == true ||
                    interface.find(interfaceMatch) != std::string::npos)
           {
               MethodArgsMap methodArgsMap;
               ifs->push_back(interface);
               boost::property_tree::ptree intf =
                       (boost::property_tree::ptree) f.second ;
               BOOST_FOREACH(ptree::value_type const& fint, intf)
               {
                   if (fint.first == "method")
                   {
                       auto mname = fint.second.get("<xmlattr>.name", "");
                       boost::property_tree::ptree args =
                               (boost::property_tree::ptree) fint.second ;
                       MethodArgsList methodList;
                       BOOST_FOREACH(ptree::value_type const& arg, args)
                       {
                           if (arg.first == "arg")
                           {
                               MethoArgs mArgs;
                               mArgs.arg = arg.second.get("<xmlattr>.name", "");
                               mArgs.type =
                                       arg.second.get("<xmlattr>.type", "");
                               mArgs.direction =
                                       arg.second.get("<xmlattr>.direction", "");
                               methodList.push_back(mArgs);
                           }
                       }
                       methodArgsMap[mname] = methodList;
                   } // end method getting data
               }
               interfaceMethodMap[interface] = methodArgsMap;
           } // got a wanted interface
        }
    }
    return children;
}


/**
 * @brief printObjectPropertyStringValue
 * @param tree
 */
void printObjectPropertyStringValue(const ObjectPropertyStringValue &tree,
                                    const std::string& service)
{
    for (auto & item : tree)
    {
        std::string subpath = item.first;
        printf("\npath:%s\n", subpath.c_str());
        for (const auto &intf : item.second)
        {
            std::string interface = intf.first;
            printf("\tinterface:%s\n", interface.c_str());

            // print methods
            const auto& mlist = interfaceMethodMap[interface];
            for (const auto& method : mlist)
            {
                if (local::isMethodSupported(service, subpath, interface,
                                             method.first))
                {
                     printf("\t method:%-30s\n", method.first.c_str());
                }
                else
                {
                    printf("\t #method:%-30s\n", method.first.c_str());
                }
                for (const auto& args: method.second)
                {
                    printf("\t   #arg:%s type:%s direction:%s\n",
                           args.arg.c_str(), args.type.c_str(), args.direction.c_str());
                }
            }

            // print properties
            for (const auto& prop : intf.second)
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
#if defined(DEBUG)        
        fprintf(stderr, "%s\n", xml.c_str());
        fflush(sderr);
#endif        
        std::vector<std::string> interfaceList;
        auto children = readXml(xml_stream, interfaceMatch, &interfaceList);

        InterfaceProperties interfaceData;
        for (const auto & intf_name : interfaceList)
        {
            auto propertiesValues = getProperties(service, path, intf_name);
            if (propertiesValues.empty() == false ||
                    interfaceMethodMap.count(intf_name) != 0)
            {
                PropertyStringMap stringValues;
                copyPropertyMapToPropertyStringMap(propertiesValues,
                                                   &stringValues);
                /**
                 *   Association properties will be created later,
                 *
                 *   First the all Objects are created, then
                 *     Association properties are created, so all connections
                 *     already exists.
                 *
                 *   To work it is necessary xyz.openbmc_project.ObjectMapper
                 *     to start first from this service.
                 */
                if (intf_name == "xyz.openbmc_project.Association.Definitions")
                {
                    InterfaceProperties interfaceDataAssociation;
                    interfaceDataAssociation[intf_name] = stringValues;
                    objectsAssociationMap[path] = interfaceDataAssociation;
                    continue; // dont store Association into interfaceData
                }
                // normal interface
                interfaceData[intf_name] = stringValues;
            }
        }
        if ((interfaceData.empty() == false || children.empty() == false)
            && path != "/")
        {
            (*tree)[path] = interfaceData;
        }
        else
        {
          //  fprintf(stderr, "interfaceData.empty() == true\n");
        }
        for (auto const& child:  children)
        {
            std::string child_path(path);
            if (child_path != "/")
            {
               child_path += '/';
            }
            child_path += child;
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
    if (tree.empty() == true)
    {
        fprintf(stderr, "No items found, check 'service name' ");
        if (path != "/")
        {
            fprintf(stderr, "and 'path'");

        }
        fprintf(stderr, "\n");
    }
    else
    {
        printf("service: %s\n",  service.c_str());
        printObjectPropertyStringValue(tree, service);
#ifndef IGNORE_ASSOCIATIONS
        printObjectPropertyStringValue(objectsAssociationMap, service);
#endif
    }

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

    auto tree = objectTreeValues(service, path);
    return tree.empty() == true ?  1 : 0;
}
