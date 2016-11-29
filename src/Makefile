statetest:
	g++ -std=c++14 statetest.cpp  PubSub.cpp Device.cpp Commands.cpp Camera.cpp -I "/home/debian/azure-iot-sdks/c/c-utility/inc" -I "/home/debian/azure-iot-sdks/c/iothub_client/inc" -I /home/debian/azure-storage-cpp/Microsoft.WindowsAzure.Storage/includes -I /home/debian/casablanca/Release/include -L "/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/uamqp" -L "/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/c-utility" -L "/home/debian/azure-iot-sdks/c/cmake/iotsdk_linux/iothub_client" -L /home/debian/azure-storage-cpp/Microsoft.WindowsAzure.Storage/build.debug/Binaries -L /home/debian/casablanca/Release/build.debug/Binaries -lboost_system -lcpprest -lazurestorage -liothub_client -liothub_client_amqp_transport -laziotsharedutil -luamqp -lm -lcurl -lssl -lcrypto -lpthread -lprussdrv -lueye_api -o statetest

clean:
	rm statetest
