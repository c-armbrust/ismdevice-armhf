//
// Created by root on 1/17/17.
//

#ifndef ISMDEVICE_FWUPDATE_H
#define ISMDEVICE_FWUPDATE_H

#ifdef __cplusplus
extern "C" {
#endif

int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);

#ifdef __cplusplus
}
#endif

#endif //ISMDEVICE_FWUPDATE_H
