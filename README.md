# ismdevice
azure devices sdk for c in a c++ oo shell

# Prerequisites
Azure IoT devices C SDK: https://github.com/Azure/azure-iot-sdks/tree/master/c
Microsoft Azure Storage Client Library for C++: https://github.com/Azure/azure-storage-cpp
Casablanca C++ REST SDK: https://github.com/Microsoft/cpprestsdk/wiki/How-to-build-for-Linux
IDS Camera Driver and SDK: https://en.ids-imaging.com/download-ueye-emb-hardfloat.html

The following paths may differ on your system. The Makefile assumes that their content is copied to the /inc and /lib older of the ismdevice-armhf project.

# Required INCLUDEs
# IoT SDK
/home/debian/azure-iot-sdks/c/c-utility/inc
/home/debian/azure-iot-sdks/c/iothub_client/inc
# Azure Storage & C++ REST
/home/debian/azure-storage-cpp/Microsoft.WindowsAzure.Storage/includes
/home/debian/casablanca/Release/include

# Required LIBS
# IoT SDK
(libuamqp.a)
/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/uamqp

(libaziotsharedutil.a)
/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/c-utility

(libiothub_client.a, libiothub_client_amqp_transport.a)
/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/iothub_client

# Azure Storage & C++ REST
(libazurestorage.so, libazurestorage.so.2, libazurestorage.so.2.5)
(cp the libazurestorage.so.2.5 and create with ln -s:
libazurestorage.so.2 -> libazurestorage.so.2.5 and libazurestorage.so -> libazurestorage.so.2)
/home/debian/azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.debug/Binaries

(libcpprest.so, libcpprest.so.2.9)
(copy the libcpprest.so.2.9 and create with ln -s:
libcpprest.so -> libcpprest.so.2.9)
/home/debian/casablanca/Release/build.debug/Binaries

# service code
https://github.com/c-armbrust/ismserviceconsole
