cmake_minimum_required(VERSION 3.0)
project(ismdevice)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wl,-rpath=/home/debian/ismdevice-armhf/lib -I /home/debian/ismdevice-armhf/inc -L /home/debian/ismdevice-armhf/lib")

add_executable(statetest statetest.cpp PubSub.cpp Device.cpp Commands.cpp Camera.cpp)
target_link_libraries(statetest boost_system cpprest azurestorage iothub_client iothub_client_amqp_transport aziotsharedutil uamqp m curl ssl crypto pthread prussdrv ueye_api devicecrypto)


