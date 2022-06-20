# openbmc-dbus-populate-tool
This is a simple tool that aims to replace a such service having a file with data: paths, interfaces, properties and values.
The purpose is to improve the openbmc development using emulator providing real DBus data from a real machine.
The idea is to clone data from a service on the real machine and then use this data to reproduce that service on the emulator (i.e qemu).
## Syntax
1. dbus-populate-tool copy    \<service-name\> [object]
2. dbus-populate-tool service \<service-data-file\>

The first command has to be invoked on the real machine, it print information which must be saved in a text file.
Then the second command has to be invoked on the emulator, the text file generated from the first command has to be transferred from the real machine to the emulator.
If the the service is active on the emulator it must be stopped before invoking the *dbus-populate-tool*.

## Build
```
# make sure you have loaded your openbmc SDK setup
$ .  <my-sdk-location>/environment-setup-arm1176jzs-openbmc-linux-gnueabi

$ cd openbmc-dbus-populate-tool
$ meson [build-options] ../build
$ ninja -v -C .../build
```

### Build options passed to meson
There are two options:
```
 -Dreadonly=enabled   -> create properties are readonly, (default is readWrite)
 -Dservicefile='"fullpath-filename"'   -> load this file instead accepting files from command line
```

## Running Examples
### (1) On Real machine
```
$ dbus-populate-tool copy xyz.openbmc_project.EntityManager | tee EntityManager.txt

$ scp EntityManager.txt root@qemuAddress:/tmp   ## supposing it is possible to copy directly to qemu running session
```

### (2) On Qemu
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
 		Manufacturer                   = s = NVIDIA
 		Name                           = s = 
 	interface:xyz.openbmc_project.Inventory.Item.Chassis
		Associations                   =
 		CurrentPowerState              = s = xyz.openbmc_project.State.Chassis.PowerState.On
 		Manufacturer                   = s = NVIDIA
 		Name                           = s =
 		Type                           = s = xyz.openbmc_project.Inventory.Item.Chassis.ChassisType.Zone
 	interface:xyz.openbmc_project.State.Chassis
		Associations                   =
 		CurrentPowerState              = s = xyz.openbmc_project.State.Chassis.PowerState.On
```

