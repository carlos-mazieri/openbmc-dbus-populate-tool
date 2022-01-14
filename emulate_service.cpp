
#include "config.h"
#include <boost/asio/io_service.hpp>
#include <sdbusplus/asio/object_server.hpp>

#include <map>
#include <string>

boost::asio::io_service io;
std::map<std::string, std::string>  propMz;


int emulate_service(char *argv[])
{
    (void) argv;
    return 0;

    propMz["idade"] = "59";
    propMz["nome"] = "carlos";
    propMz["sobrenome"] = "mazieri";
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);
    sdbusplus::asio::object_server objServer(systemBus);
    systemBus->request_name(argv[1]);

    std::shared_ptr<sdbusplus::asio::dbus_interface> iface =
        objServer.add_interface("/xyz/openbmc_project/FruDevice",
                                "xyz.openbmc_project.MzInterface");

    auto lambdaR =  []  (const std::string & propvalue) -> std::string
                        {
                            printf("lambdaR[] value=%s\n", propvalue.c_str());
                            return propvalue;
                        };

    auto lambdaRW =  []  (const std::string & newValue,
                          const std::string & oldValue)
                        {
                            std::string & mz = const_cast<std::string&>(oldValue);
                            mz = newValue;
                            printf("lambdaRW[] newValue=%s oldValue=%s\n",
                                   newValue.c_str(), oldValue.c_str());
                            return 1;
                        };

    for (const auto& item: propMz)
    {
        std::string key   = item.first;
        const std::string& value = propMz.at(key);

#if 0

        bool ok = iface->register_property_r(key,
                                             value,
                                             sdbusplus::vtable::property_::const_,
                                             *lambdaR);
#else

        bool ok = iface->register_property_rw(key,
                                             value,
                                             sdbusplus::vtable::property_::emits_change,
                                             *lambdaRW,
                                             *lambdaR);
#endif

        printf("item.first=%s item.second=%s ok=%d\n", key.c_str(),
                                                        value.c_str(), ok);
    }


    iface->initialize();

    io.run();

}
