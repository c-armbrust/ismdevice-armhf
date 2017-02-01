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
    // Convert method name to string
    std::string method = std::string{method_name};
    // If method is firmwareUpdate, initiate firmware update
    if(method.compare("firmwareUpdate") == 0) {
        // Get JSON payload
        std::string fw_data((char*)payload, size);

        std::cout << "\nInitiate Firmware Update\n";

        std::cout << "Payload: " << fw_data << std::endl;
    }
    // Else return a generic response
    else {
        std::cout << "Unknown device method called: " << method << std::endl;
        std::cout << "Payload: " << payload << std::endl;
    }

    // Respond to method
    char* RESPONSE_STRING = (char*)"{ \"Response\": \"This is the response from the device\" }";
    int status = 200;
    std::cout << "Response status:  " << status << std::endl;
    std::cout << "Response payload: " << RESPONSE_STRING << std::endl;

    *resp_size = strlen(RESPONSE_STRING);
    if ((*response = (unsigned char*)malloc(*resp_size)) == NULL)
        status = -1;
    else
        memcpy(*response, RESPONSE_STRING, *resp_size);

    return status;
}

void FirmwareUpdateHandler::DownloadFirmwareUpdate(std::string blobUrl, std::string fileName) {
    // Retrieve reference to blob
    azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(U(fileName));
    blockBlob.download_to_file("~/fwupdate.tar.gz");
    // Save blob contents to a file.
//    concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
//    concurrency::streams::ostream output_stream(buffer);
//    blockBlob.download_to_stream(output_stream);
//
//    std::ofstream outfile(fileName, std::ofstream::binary);
//    std::vector<unsigned char>& data = buffer.collection();
//
//    outfile.write((char *)&data[0], buffer.size());
//    outfile.close();
}

void FirmwareUpdateHandler::InitBlobStorage(utility::string_t storageConnectionString)
{
    try
    {
        // Retrieve storage account from connection string.
        azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storageConnectionString);

        // Create the blob client
        azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

        // Retrieve a reference to a container.
        this->container = blob_client.get_container_reference(containerName);

        // Create the container if it doesn't already exist.
        container.create_if_not_exists();
    }
    catch(const std::exception& e)
    {
        std::wcout << U("Error: ") << e.what() << std::endl;
    }
}