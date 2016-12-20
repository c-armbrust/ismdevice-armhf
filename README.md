# ISM Device

Azure Devices SDK for c in a c++ oo shell

[Service Code](https://github.com/c-armbrust/ismserviceconsole)

# Prerequisites
[Azure IoT devices C SDK](https://github.com/Azure/azure-iot-sdks/tree/master/c)

[Microsoft Azure Storage Client Library for C++](https://github.com/Azure/azure-storage-cpp)

[Casablanca C++ REST SDK](https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux)

[IDS Camera Driver and SDK](https://en.ids-imaging.com/download-ueye-emb-hardfloat.html)

[Device Crypto](https://github.com/LeoReentry/ism-device-crypto)


# Required Libraries

The libraries from the projects mentioned above will have to be moved inside the lib/ subfolder:

```
IoT SDK
azure-iot-sdks/c/cmake/iotsdk_linux/uamqp
            libuamqp.a              
azure-iot-sdks/c/cmake/iotsdk_linux/c-utility
            libaziotsharedutil.a
azure-iot-sdks/c/cmake/iotsdk_linux/iothub_client
            libiothub_client_amqp_transport.a
            libiothub_client.a
                                                    
Casablanca C++ REST SDK
casablanca/Release/build.debug/Binaries
            libcpprest.so -> libcpprest.so.2.9
            libcpprest.so.2.9
            
Microsoft Azure Storage Client Library for C++
azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.debug/Binaries
            libazurestorage.so -> libazurestorage.so.2
            libazurestorage.so.2 -> libazurestorage.so.2.5
            libazurestorage.so.2.5
            
Device Crypto
            libdevicecrypto.so

```


# Compilation

## Compilation process
Clone this repo, cd into it and run
```
cmake ./
make
```

## Notes on cross-compilation

Header files have to be copied in the subfolder inc/ or in your system include folders.
Library files have to be in the subfolder lib/ or in the system library folders. When 
running cmake, it will take the absolute path of those folders and include them in the 
executable. The absolute path will have to be the same on the BeagleBone Black, otherwise 
the program will not work. Example:

Our project is in the folder
```
/home/debian/ismdevice-armhf
```
Then, after compiling and moving the binary to the device, the header files and shared
libraries will have to be in
```
/home/debian/ismdevice-armhf/inc
/home/debian/ismdevice-armhf/lib
```
