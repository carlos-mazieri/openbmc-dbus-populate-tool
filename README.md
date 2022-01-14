# openbmc-dbus-populate-tool
This is a simple tool that aims to replace a such service having a file with data: paths, interfaces, properties and values.
The purpose is to improve the openbmc development using emulator providing real DBus data from a real machine.
The idea is to clone data from a service on the real machine and then use this data to reproduce that service on the emulador (i.e qemu).
## Syntax
1. dbus-populate-tool copy    \<service-name\> [object]
2. dbus-populate-tool service \<service-data-file\>

The first command has to be invoked on the real machine, it print information which must be saved in a text file.
Then the second command has to be invoked on the emulator, the text file generated from the first command has to be transferred from the real machine to the emulator.
If the the service is active on the emulator it must be stopped before invoking the *openbmc-dbus-populate-tool*.

## Build
```
# make sure you have loaded your openbmc SDK setup
$ .  <my-sdk-location>/environment-setup-arm1176jzs-openbmc-linux-gnueabi

$ cd openbmc-dbus-populate-tool
$ meson ../build
$ ninja -v -C .../build
```

## Running Examples
### (1) On Real machine
```
$ dbus-populate-tool copy xyz.openbmc_project.EntityManager /xyz/openbmc_project/inventory | tee EntityManager.txt

$ scp EntityManager.txt root@qemuAddress:/tmp   ## supposing it is possible to copy directly to qemu running session
```

### (2) On Qemu
```
$ systemctl stop xyz.openbmc_project.EntityManager   ## stop the real service and starts the emulated one

$ dbus-populate-tool service /tmp/EntityManager.txt  ## supposing scp command above has stored he file in /tmp
```
