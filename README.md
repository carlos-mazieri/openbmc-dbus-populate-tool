# openbmc-dbus-populate-tool
This is simple tool that aims to replace a such service having a file with data: paths, interfaces, properties and values.
The purpose is to improve the openbmc development using emulator providing real DBus data from a real machine. 
The idea is to clone data from a service on the real machine and then use this data to reproduce that service on the emulador (i.e qemu).
## Syntax
1. openbmc-dbus-populate-tool clone <service> [object]
2. openbmc-dbus-populate-tool service service-data-file

The first command has to be invoked on the real machine, it generates a text file named "<service>-data.txt".
Then the second command has to be invoked on the emulator, the service-data-file has to be transferred from the real machine to the emulator.
If the the service is active on the emulator it must be stopped before invoking the *openbmc-dbus-populate-tool*.
