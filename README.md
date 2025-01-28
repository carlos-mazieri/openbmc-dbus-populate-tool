# openbmc-dbus-populate-tool
This is a simple tool that aims to replace a such service having a configuration file with data: paths, interfaces, properties and their values.

The purpose is to improve the openbmc development using emulator providing real DBus data from a real HW machine.
The idea is to clone data from a service on the real machine and then use this data to reproduce that service on the emulator (i.e qemu).
## Syntax
1. dbus-populate-tool copy    **service-name** [object]
2. dbus-populate-tool service **service-configuration-data-file**

The first command has to be invoked on the real machine, it prints information which must be saved in a text file to be used as **service-configuration-data-file**.

Then the second command has to be invoked on the emulator, the text file generated from the first command has to be transferred from the real machine to the emulator.

If the the service is active on the emulator it must be stopped before invoking the **dbus-populate-tool**.

## Build
```
# make sure you have loaded your openbmc SDK setup
$ source <my-sdk-location>/environment-setup-arm1176jzs-openbmc-linux-gnueabi

$ cd openbmc-dbus-populate-tool
$ meson [build-options] ../build
$ ninja -v -C ../build
```

### Build options passed to meson
There are two options:
```
 -Dreadonly=enabled   -> create properties are readonly, (default is readWrite)
 -Dservicefile='"fullpath-filename"'   -> load this file instead of accepting files from the command line
```

## Running Examples
### (1) On Real machine, using 'copy' command
```
$ dbus-populate-tool copy xyz.openbmc_project.EntityManager | tee EntityManager.txt

$ scp EntityManager.txt root@qemuAddress:/tmp   ## supposing it is possible to copy directly to qemu running session
```

### (2) On Qemu, using 'service' command
```
$ systemctl stop xyz.openbmc_project.EntityManager   ## stop the real service and starts the emulated one

$ dbus-populate-tool service /tmp/EntityManager.txt  ## supposing scp command above has stored he file in /tmp

```

## Sample of the output printed by '*dbus-populate-tool copy*' command
```
service: xyz.openbmc_project.GpuMgr

path:/xyz/openbmc_project/GpuMgr
	interface:xyz.openbmc_project.GpuMgr.Server
	 #method:Passthrough
	   #arg: type:i direction:in
	   #arg: type:y direction:in
	   #arg: type:y direction:in
	   #arg: type:y direction:in
	   #arg: type:au direction:in
	   #arg: type:(iau) direction:out
	 #method:PassthroughFpga
	   #arg: type:i direction:in
	   #arg: type:y direction:in
	   #arg: type:y direction:in
	   #arg: type:y direction:in
	   #arg: type:au direction:in
	   #arg: type:(iau) direction:out

path:/xyz/openbmc_project/inventory/system/chassis/Baseboard
	interface:xyz.openbmc_project.Association.Definitions
		Associations                   =
        interface:xyz.openbmc_project.Inventory.Decorator.Asset
                Associations                   =
                CurrentPowerState              = s = xyz.openbmc_project.State.Chassis.PowerState.On
                Manufacturer                   = s = Any
                Name                           = s = 
        interface:xyz.openbmc_project.Inventory.Item.Chassis
                Associations                   =
                CurrentPowerState              = s = xyz.openbmc_project.State.Chassis.PowerState.On
                Manufacturer                   = s = Any
                Name                           = s =
                Type                           = s = xyz.openbmc_project.Inventory.Item.Chassis.ChassisType.Zone
        interface:xyz.openbmc_project.State.Chassis
		Associations                   =
                CurrentPowerState              = s = xyz.openbmc_project.State.Chassis.PowerState.On
```

## dbus-populate-tool copy output format
The **dbus-populate-tool copy {service}** prints an output on such format that should be saved into a file.

This file can edited to add object-paths, interfaces, properties or changing existing property values. Also a file can created and edited from scratch.

The format, where **{TAB}** means one TAB character ('\t') is:

**service:**&lt;service-name&gt;

**path:**&lt;object-path-name&gt; 

**{TAB}interface:**&lt;interface-name&gt;

**{TAB}{TAB}**&lt;property-name&gt;  **=**  &lt;property-type&gt;  **=**  &lt;value&gt;

In the example above almost all properties are **type** 's' meaning strings.

The following table provides an equivalence among C++, Stamdard DBus types and dbus-populate-tool supported types:

| C++ type                                           |  Standard DBus type|   dbus-populate-tool type|
|----------------------------------- |-----            |-----           |
| bool                                               | b                  | b                        |
| uint8_t                                            | y                  | y                        |
| vector&lt;uint8_t&gt;                              | ay                 | Y                        |
| int16_t                                            | n                  | n                        |
| vector&lt;int16_t&gt;                              | an                 | N                        |
| uint16_t                                           | q                  | q                        |
| vector&lt;uint16_t&gt;                             | aq                 | Q                        |
| int32_t                                            | i                  | i                        |
| vector&lt;int32_t&gt;                              | ai                 | I                        |
| uint32_t                                           | u                  | u                        |
| vector&lt;uint32_t&gt;                             | au                 | U                        |
| int64_t                                            | x                  | x                        |
| vector&lt;int64_t&gt;                              | ax                 | X                        |
| uint64_t                                           | t                  | t                        |
| vector&lt;uint64_t&gt;                             | at                 | T                        |
| string                                             | s                  | s                        |
| vector&lt;string&gt;                               | as                 | S                        |
| OpenBMC Associations<br>vector&lt;tuple&lt;string, string, string&gt;&gt;| a{sss}              | A                        |

Array values are also separated by **{TAB}** characters.
Association items are separated by new lines ('\n') and the tuples are also separated by **{TAB}** characters.


