//
// Created by root on 2/1/17.
//

#include <stdlib.h>
#include "iothub_client.h"
#include "FirmwareUpdateHandler.h"


FirmwareUpdateHandler::FirmwareUpdateHandler(utility::string_t stoconnstr, utility::string_t containername, std::string storageaccname) : containerName{containername}, storageAccountName{storageaccname}
{
    InitBlobStorage(stoconnstr);
    memset((void*)stoconnstr.data(), 0, stoconnstr.size());
}

int FirmwareUpdateHandler::DeviceMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *resp_size, void *userContextCallback) {
    (void)userContextCallback;

    printf("\r\nDevice Method called\r\n");
    printf("Device Method name:    %s\r\n", method_name);
    printf("Device Method payload: %.*s\r\n", (int)size, (const char*)payload);

    int status = 200;
    char* RESPONSE_STRING = (char*)"{ \"Response\": \"This is the response from the device\" }";
    printf("\r\nResponse status: %d\r\n", status);
    printf("Response payload: %s\r\n\r\n", RESPONSE_STRING);

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = (unsigned char*)malloc(*resp_size)) == NULL)
    {
        status = -1;
    }
    else
    {
        memcpy(*response, RESPONSE_STRING, *resp_size);
    }
    return status;

}

void FirmwareUpdateHandler::InitBlobStorage(utility::string_t storageConnectionString)
{
    try
    {
        std::cout << "Create Azure Storage container " << containerName << " on Storage Account " << storageAccountName << " if not exists" << std::endl;

        // Retrieve storage account from connection string.
        azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storageConnectionString);

        // Create the blob client
        azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

        // Retrieve a reference to a container.
        this->container = blob_client.get_container_reference(containerName);

        // Create the container if it doesn't already exist.
        container.create_if_not_exists();

        // Make the blob container publicly accessible.
        azure::storage::blob_container_permissions permissions;
        permissions.set_public_access(azure::storage::blob_container_public_access_type::blob);
        container.upload_permissions(permissions);
    }
    catch(const std::exception& e)
    {
        std::wcout << U("Error: ") << e.what() << std::endl;
    }
}