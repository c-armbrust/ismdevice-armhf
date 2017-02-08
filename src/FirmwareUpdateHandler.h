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
    void HandleFirmwareUpdate(std::string blobUrl, std::string fileName, std::string publicKeyUrl);

private:
    void InitBlobStorage(utility::string_t storageConnectionString);
    void DownloadFirmwareUpdate(std::string blobUrl, std::string fileName);
    void HandleFirmwareUpdateError(std::string error);

private:
    // Blob Storage
    const utility::string_t containerName;
    const std::string storageAccountName;
    azure::storage::cloud_blob_container container;

};

#endif //ISMDEVICE_FIRMWAREUPDATEHANDLER_H
