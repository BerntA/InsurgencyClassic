# INSURGENCY: Classic

### Introduction
INSURGENCY: Modern Infantry Combat for SDK 2013!

### Compiling
Use the VPC scripts to generate the necessary project file(s), see
[Source SDK 2013 Compiling Help](https://developer.valvesoftware.com/wiki/Source_SDK_2013) for additional help.
* Windows: You can use VS2010, VS2013, VS2015, VS2017, VS2019, or VS2022 to compile this project.
* Linux: Use GCC 4.8 or GCC 5 compiler.
* OSX: Use XCode 10, run osx-compile.

#### Unable to run VPC to generate solution
Open cmd and run
```
reg add "HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\10.0\Projects\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}" /v DefaultProjectExtension /t REG_SZ /d vcxproj /f
```
