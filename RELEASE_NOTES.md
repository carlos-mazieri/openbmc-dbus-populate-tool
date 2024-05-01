# RELEASE NOTES

## Version 0.97 Mar 15 2024
#### Populating properly the Standard D-Bus interface 'org.freedesktop.DBus.ObjectManager' as following:
```
org.freedesktop.DBus.ObjectManager       interface -          -             -
.GetManagedObjects                       method    -          a{oa{sa{sv}}} -
.InterfacesAdded                         signal    oa{sa{sv}} -             -
.InterfacesRemoved                       signal    oas        -             -
```
#### Added support to array of integers, current supported types

The following table provides an equivalence among C++, Stamdard DBus types and dbus-populate-tool supported types:

| C++ type                                           |  Stamdard DBus type|   dbus-populate-tool type|
|-----           ----------------------------------- |-----            -- |-----             ------- |
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


## Version 0.95 Jun 12 2022
Arrays and Associations are implemented.

Added compiler directive and command line options to create Read-Only properties.

Added compiler directive to emulate service for a specific external service file.

Both have meson features.

## Version 0.90 (first version) Jan 17 2022
Both '*dbus-populate-tool copy*' and '*dbus-populate-tool service*' commands working.

Properties with basic types are supposed to work.

Arrays and Associations are not implemented so far.

