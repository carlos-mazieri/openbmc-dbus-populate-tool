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
$ meson ../build
$ ninja -v -C .../build
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

service: xyz.openbmc_project.EntityManager

path:/xyz/openbmc_project/inventory/system/board/Yosemite_V2_Baseboard
	interface:xyz.openbmc_project.Inventory.Decorator.Asset
		Manufacturer                   = Wiwynn
 		Model                          = Yosemite V2 MP
 		PartNumber                     = B91.01501.0055
 		SerialNumber                   = WTL202224FMA1
 	interface:xyz.openbmc_project.Inventory.Item.Board
		Manufacturer                   = Wiwynn
 		Model                          = Yosemite V2 MP
 		Name                           = Yosemite V2 Baseboard
 		PartNumber                     = B91.01501.0055
 		Probe                          = xyz.openbmc_project.FruDevice({'PRODUCT_PRODUCT_NAME': 'Yosemite V2 .*'})
 		SerialNumber                   = WTL202224FMA1
 		Type                           = Board
path:/xyz/openbmc_project/inventory/system/board/Yosemite_V2_Baseboard/HSC
	interface:xyz.openbmc_project.Configuration.pmbus
		Address                        = 64
 		Bus                            = 10
 		Direction                      = greater than
 		Label                          = vin
 		Labels                         = vin iout1 pin temp1 maxvin maxiout1 maxpin maxtemp1
 		Name                           = HSC
 		Severity                       = 1.000000
 		Type                           = pmbus
 		Value                          = 13.750000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds0
		Direction                      = greater than
 		Label                          = vin
 		Name                           = upper critical
 		Severity                       = 1.000000
 		Value                          = 13.750000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds1
		Direction                      = less than
 		Label                          = vin
 		Name                           = lower critical
 		Severity                       = 0.000000
 		Value                          = 11.250000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds2
		Direction                      = greater than
 		Label                          = iout1
 		Name                           = upper critical
 		Severity                       = 1.000000
 		Value                          = 52.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds3
		Direction                      = greater than
 		Label                          = pin
 		Name                           = upper critical
 		Severity                       = 1.000000
 		Value                          = 625.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds4
		Direction                      = less than
 		Label                          = temp1
 		Name                           = upper critical
 		Severity                       = 1.000000
 		Value                          = 105.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds5
		Direction                      = less than
 		Label                          = peakvin
 		Name                           = lower critical
 		Severity                       = 1.000000
 		Value                          = 0.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds6
		Direction                      = less than
 		Label                          = maxiout1
 		Name                           = lower critical
 		Severity                       = 1.000000
 		Value                          = 0.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds7
		Direction                      = less than
 		Label                          = maxpin
 		Name                           = lower critical
 		Severity                       = 1.000000
 		Value                          = 0.000000
 	interface:xyz.openbmc_project.Configuration.pmbus.Thresholds8
		Direction                      = less than
 		Label                          = maxtemp1
 		Name                           = lower critical
 		Severity                       = 1.000000
 		Value                          = 0.000000

path:/xyz/openbmc_project/inventory/system/board/Yosemite_V2_Baseboard/IpmbBus1
	interface:xyz.openbmc_project.Configuration.IPMIChannels
		Bus                            = ipmb
 		Index                          = 0
 		Name                           = IpmbBus1
 		Type                           = IPMIChannels

path:/xyz/openbmc_project/inventory/system/board/Yosemite_V2_Baseboard/IpmbBus2
	interface:xyz.openbmc_project.Configuration.IPMIChannels
		Bus                            = ipmb
 		Index                          = 1
 		Name                           = IpmbBus2
 		Type                           = IPMIChannels


```

