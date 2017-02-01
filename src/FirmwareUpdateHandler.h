//
// Created by root on 2/1/17.
//

#ifndef ISMDEVICE_FIRMWAREUPDATEHANDLER_H
#define ISMDEVICE_FIRMWAREUPDATEHANDLER_H


class FirmwareUpdateHandler {
public:
    static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);
private:
    FirmwareUpdateHandler() { };
};


#endif //ISMDEVICE_FIRMWAREUPDATEHANDLER_H
