//
// Created by root on 2/1/17.
//

#include <stdlib.h>
#include "iothub_client.h"
#include "FirmwareUpdateHandler.h"

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