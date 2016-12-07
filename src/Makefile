statetest:
	g++ -std=c++14 statetest.cpp  PubSub.cpp Device.cpp Commands.cpp Camera.cpp -Wl,-rpath=/home/debian/ismdevice-armhf/lib -I "/home/debian/ismdevice-armhf/inc" -L "/home/debian/ismdevice-armhf/lib" -lboost_system -lcpprest -lazurestorage -liothub_client -liothub_client_amqp_transport -laziotsharedutil -luamqp -lm -lcurl -lssl -lcrypto -lpthread -lprussdrv -lueye_api -o statetest

clean:
	rm statetest
