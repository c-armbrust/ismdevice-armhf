//
// Created by root on 2/1/17.
//

#ifndef ISMDEVICE_FIRMWAREUPDATEHANDLER_H
#define ISMDEVICE_FIRMWAREUPDATEHANDLER_H

#include "was/storage_account.h"
#include "was/blob.h"

class FirmwareUpdateHandler {
public:
    FirmwareUpdateHandler(utility::string_t, utility::string_t, std::string);
    static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);

private:
    void InitBlobStorage(utility::string_t storageConnectionString);
    // Blob Storage
    const utility::string_t containerName;
    const std::string storageAccountName;
    azure::storage::cloud_blob_container container;

};

#endif //ISMDEVICE_FIRMWAREUPDATEHANDLER_H
