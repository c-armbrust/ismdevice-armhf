#include "Device.h"
#include "Singleton.h"
#include "Commands.h"
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>
#include "json.hpp"
#include <exception>
#include <iothubtransportmqtt.h>
#include "FirmwareUpdateHandler.h"
#include <sys/stat.h>
extern "C" {
	#include "crypto.h"
}

Device::Device(const std::string& configFile)
{
	// Declare device settings variable
	nlohmann::json devSettings;
	if (configFile == "") {
		// Variables for the decrypted JSON settings
		unsigned char* jsonString;
		int jsonLength;
		// Decrypt device settings using TPM
		DeviceCrypto_Decrypt((char*)"settings", &jsonString, &jsonLength);
		// Parse JSON
		devSettings = nlohmann::json::parse(jsonString);
	} else {
		std::ifstream ifs(configFile);
		devSettings = nlohmann::json::parse(ifs);
	}
	// Fill into string variables
	std::string connectionString = devSettings["ConnectionString"];
	std::string storage_connection_string = devSettings["StorageConnectionString"];
	std::string storage_acc_name = devSettings["StorageAccount"];
	std::string container_name = devSettings["StorageContainer"];
    // Container in which firmware update files are stored
    std::string firmware_update_container = devSettings["FwUpdateContainer"];
    // Url to retrieve public key from portal
    publicKeyUrl = devSettings["PublicKeyUrl"];
	// Start Device initialization
	_state = &Singleton<ReadyState>::Instance();
	platform_init();
	iotHubClientHandle = IoTHubClient_CreateFromConnectionString(connectionString.c_str(), MQTT_Protocol);
//	camera = new Camera(storage_connection_string, container_name, storage_acc_name);
//	camera->GetCameraInfo();
    firmwareUpdateHandler = new FirmwareUpdateHandler(storage_connection_string, "fwupdates", storage_acc_name);
	settings = new DeviceSettings(this->getDeviceId(connectionString), _state->getStateName(), 5000, "", // std::string DeviceId, std::string StateName, int CapturePeriod, std::string CurrentCaptureUri
								  0.0025, 8.5, 3.75, 4, 16, // double VarianceThreshold, double DistanceMapThreshold, double RGThreshold, double RestrictedFillingThreshold, double DilateValue
								  10, 10.0,//camera->getGain(), camera->getExposure(), // int Gain, double Exposure
								  4, 0, 1000); // int PulseWidth, int Current, int Predelay

	// Register direct method callback
	if (IoTHubClient_SetDeviceMethodCallback(iotHubClientHandle, Device::DeviceMethodCallback, this) != IOTHUB_CLIENT_OK) {
		std::cout << "Error! Registering Direct Method callback failed.\n";
	}

    // If folder ~/.fwtmp exists, we just rebooted from a firmware update
	struct stat sb;
	if (stat("/home/debian/.fwtmp", &sb) == 0 && S_ISDIR(sb.st_mode))
	{
        std::string error = "";
        bool hasError = false;
        // Check for errors
        std::ifstream f("/home/debian/.fwtmp/error");
        if (f.good()) {
            hasError = true;
            getline(f, error);
        }

        if (hasError)
            // Send message for completed firmware update with error
            this->SendD2C_FwUpdateStatus(CommandType::FIRMWARE_UPDATE_STATUS, "Error during Firmware Update", error);
        else {
            std::string message = "Success";
            std::string status = "Firmware Update completed";

            // If no error, update device software version
            // Update config file
            int r = system("setsetting softwareversion \"$(cat /home/debian/.fwtmp/sw-version)\"");

            // Send message that firmware update was completed
            this->SendD2C_FwUpdateStatus(CommandType::FIRMWARE_UPDATE_STATUS, status, message);
        }

        // Remove directory ~/.fwmp
        int retval = system("rm -rf /home/debian/.fwtmp");
        if (retval != 0) {
            std::cout << "Error! Couldn't delete temporary firmware update directory\n";
        }
	}

	// Overwrite strings with 0 in memory since we don't know when the RAM is gonna be used by something else
	memset((void*)connectionString.data(), 0, connectionString.size());
	memset((void*)storage_connection_string.data(), 0, storage_connection_string.size());
	memset((void*)storage_acc_name.data(), 0, storage_acc_name.size());
	memset((void*)container_name.data(), 0, container_name.size());
    memset((void*)firmware_update_container.data(), 0, firmware_update_container.size());
	// For each JSON value, get a pointer and overwrite the memory with 0
	auto ptr = devSettings["ConnectionString"].get_ptr<nlohmann::json::string_t*>();
	memset((void*)ptr->data(), 0, ptr->size());
	ptr = devSettings["StorageConnectionString"].get_ptr<nlohmann::json::string_t*>();
	memset((void*)ptr->data(), 0, ptr->size());
	ptr = devSettings["StorageAccount"].get_ptr<nlohmann::json::string_t*>();
	memset((void*)ptr->data(), 0, ptr->size());
    ptr = devSettings["StorageContainer"].get_ptr<nlohmann::json::string_t*>();
    memset((void*)ptr->data(), 0, ptr->size());
    ptr = devSettings["FwUpdateContainer"].get_ptr<nlohmann::json::string_t*>();
    memset((void*)ptr->data(), 0, ptr->size());
    ptr = devSettings["PublicKeyUrl"].get_ptr<nlohmann::json::string_t*>();
    memset((void*)ptr->data(), 0, ptr->size());
	// Reassign each value of the JSON object an empty string because the object still thinks we have full sized strings in memory and won't free our memory
	devSettings["ConnectionString"] = "";
	devSettings["StorageConnectionString"] = "";
    devSettings["StorageAccount"] = "";
    devSettings["StorageContainer"] = "";
    devSettings["FwUpdateContainer"] = "";
    devSettings["PublicKeyUrl"] = "";
}

Device::~Device()
{
	// Pub Sub detach
//	camera->NewCaptureUploaded.Detach(this);
}

int Device::DeviceMethodCallback(const char *method_name, const unsigned char *payload, size_t size, unsigned char **response, size_t *resp_size, void *userContextCallback) {
    // Convert method name to string
    std::string method = std::string{method_name};
    // If method is firmwareUpdate, initiate firmware update
    if(method.compare("firmwareUpdate") == 0) {
        // Get JSON payload
        nlohmann::json fw_data = nlohmann::json::parse(std::string((char*)payload, size));
        std::string blob = fw_data["blobUrl"];
        std::string fileName = fw_data["fileName"];
        std::string version = fw_data["version"];
        std::cout << "\nInitiate Firmware Update\n";

		// Respond to method
		char* RESPONSE_STRING = (char*)"{ \"Response\": \"Initiating Firmware Update\" }";
		int status = 200;

		*resp_size = strlen(RESPONSE_STRING);
		if ((*response = (unsigned char*)malloc(*resp_size)) == NULL)
			status = -1;
		else
			memcpy(*response, RESPONSE_STRING, *resp_size);

		// Run firmware update in new Thread
		Device* device = (Device*)userContextCallback;
		std::thread t([&](std::string blob_t, std::string fileName_t, std::string version_t, Device* d){
			d->FirmwareUpdate(blob_t, fileName_t, version_t);
		}, blob, fileName, version, device);
		t.detach();
		// Return status
		return status;
    }
	// Respond to method
	char* RESPONSE_STRING = (char*)"{ \"Response\": \"Unknown function called\" }";
	int status = 404;
	*resp_size = strlen(RESPONSE_STRING);
	if ((*response = (unsigned char*)malloc(*resp_size)) == NULL)
		status = -1;
	else
		memcpy(*response, RESPONSE_STRING, *resp_size);
	return status;


}

void Device::FirmwareUpdate(std::string blobUrl, std::string fileName, std::string version) {
    this->firmwareUpdateHandler->HandleFirmwareUpdate(blobUrl, fileName, publicKeyUrl, version);
}
// Pub Sub interface
void Device::OnNotification(Publisher* context)
{
	// Use RTTI (run-time type information) mechanism of C++ to cast in the correct concrete notification type
	// http://stackoverflow.com/questions/351845/finding-the-type-of-an-object-in-c

	Notification* notification = context->GetNotification();

	if(dynamic_cast<CaptureNotification*>(notification) != nullptr)
	{
		std::string uri = dynamic_cast<CaptureNotification*>(notification)->CurrentCaptureUri;
		std::cout << "Camera notification: " << uri << std::endl;

		// Update settings with new current capture uri and send them as D2C message
		settings->setCurrentCaptureUri(uri);
		SendD2C_DeviceSettings(CommandType::CAPTURE_UPLOADED);
	}
	/* ...
	if(dynamic_cast<[an other concrete notification type ptr]>(notification) != nullptr)
	{
		// access fields of that concrete notification type
		// ...
	}
	*/
}

void Device::SubscribeNotifications()
{
//	camera->NewCaptureUploaded.Attach(this);
}



void Device::ChangeState(DeviceState* s)
{
	_state = s;
	settings->setStateName(_state->getStateName());
}

bool Device::UpdateSettings(std::string msgbody)
{
	std::cout << "\nold settings:" << std::endl;
	settings->Report();
	settings->Deserialize(msgbody);

//	// Update camera settings
//	//
//	SetCameraPruValues(); // Assertion: settings is updated before this call
//
//	if(camera->setGain(settings->getGain()) == false)
//		return false;
//	if(camera->setExposure(settings->getExposure()) == false)
//		return false;

	std::cout << "\nnew settings:" << std::endl;
	settings->Report();

	// No errors occured
	return true;
}

// DeviceId is part of the connection string
std::string Device::getDeviceId(const std::string& connectionString)
{
	std::string connStr = connectionString;
	std::string searchPattern = "DeviceId=";
	std::size_t pos_begin = connStr.find(searchPattern) + searchPattern.length();
	std::size_t pos_end = connStr.find(";", pos_begin+1);
	std::string id = connStr.substr(pos_begin, pos_end - pos_begin);
	memset((void*)connStr.data(), 0, connStr.size());
	return id;
}

void Device::StartCamera()
{
//	SetCameraPruValues();
//	camera->Start();
}

void Device::SetCameraPruValues()
{
    // 1. Translate  DeviceSettings --> Camera settings (trigger-, pulse-, pause-duration)
    // 2. Set the camera settings
    // 3. Start camera
    //
    // 1 instruction = 5ns
    // 200 instructions = 1µs
    // !!! 2 instructions per loop (divide your number by 2)
    //

    // DeviceSettings.PulseWidth -> Camera.pulseduration
    unsigned int pulseduration = (unsigned int)settings->getPulseWidth();
    pulseduration *= 200;
    pulseduration /= 2;
    camera->setPulseduration(pulseduration);

    // DeviceSettings.Predelay -> Camera.triggerduration
    unsigned int triggerduration = (unsigned int)settings->getPredelay();
    triggerduration *= 200;
    triggerduration /= 2;
    camera->setTriggerduration(triggerduration);

    // DeviceSettings.CapturePeriod -> Camera.pauseduration
    unsigned int pauseduration = (unsigned int)settings->getCapturePeriod();
    pauseduration *= 1000; // ms to µso
    pauseduration *= 200;
    pauseduration /= 2;
    pauseduration -= camera->getTriggerduration();
    pauseduration -= camera->getPulseduration();
    camera->setPauseduration(pauseduration);

    // print the values in hex (include <iomanip>)
    std::stringstream stream;
    stream << std::hex << camera->getPulseduration();
    std::cout << "Set camera pulseduration to: " << stream.str() << " (hex)" << std::endl;
    stream.str("");
    stream.clear();
    stream << std::hex << camera->getTriggerduration();
    std::cout << "Set camera triggerduration to: " << stream.str() << " (hex)" << std::endl;
    stream.str("");
    stream.clear();
    stream << std::hex << camera->getPauseduration();
    std::cout << "Set camera pauseduration to: " << stream.str() << " (hex)" << std::endl;
}

void Device::StopCamera()
{
//	camera->Stop();
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::Start()
{
	return _state->Start(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::Stop()
{
	return _state->Stop(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::StartPreview()
{
	return _state->StartPreview(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::StopPreview()
{
	return _state->StopPreview(this);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::SetDeviceSettings(std::string msgbody)
{
	return _state->SetDeviceSettings(this, msgbody);
}

IOTHUBMESSAGE_DISPOSITION_RESULT Device::GetDeviceSettings()
{
	return _state->GetDeviceSettings(this);
}



IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::Start(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::Stop(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::StartPreview(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::StopPreview(Device*){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::SetDeviceSettings(Device*, std::string){}
IOTHUBMESSAGE_DISPOSITION_RESULT DeviceState::GetDeviceSettings(Device*){}



// DeviceState -> Device interface
//
void DeviceState::ChangeState(Device* d, DeviceState* s)
{
	d->ChangeState(s);
}

bool DeviceState::UpdateSettings(Device* d, std::string msgbody)
{
	return d->UpdateSettings(msgbody);
}

void DeviceState::SendD2C_DeviceSettings(Device* d, std::string cmdType)
{
	d->SendD2C_DeviceSettings(cmdType);
}

void DeviceState::StartCamera(Device* d)
{
	d->StartCamera();
}

void DeviceState::StopCamera(Device* d)
{
	d->StopCamera();
}



void Device::ReceiveC2D()
{
	IoTHubClient_SetMessageCallback(iotHubClientHandle, ReceiveMessageCallback, this);

	while(1)
    {
        ThreadAPI_Sleep(1000);
    }
}

// Look how to pass parameter to the threads lambda:
// http://stackoverflow.com/questions/25536956/how-to-write-lambda-function-with-arguments-c
void Device::SendD2C_DeviceSettings(std::string cmdType)
{
    std::thread t([&](std::string commandType){
		try
		{
			IOTHUB_MESSAGE_HANDLE messageHandle;
			char sendBuffer[MAX_SEND_BUFFER_SIZE];

			// fill send buffer
			sprintf_s(sendBuffer, MAX_SEND_BUFFER_SIZE, settings->Serialize().c_str());
			messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)sendBuffer, strlen(sendBuffer));

			// set event properties
			MAP_HANDLE propMap = IoTHubMessage_Properties(messageHandle);
			Map_AddOrUpdate(propMap, EventType::D2C_COMMAND.c_str(), commandType.c_str());

			std::cout << "send message, size=" << strlen(sendBuffer) << std::endl;
			std::cout << "CommandType: " << commandType << std::endl;

			// send the message
			IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, SendConfirmationCallback, this);
		}
		catch(std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}, cmdType);

	t.detach();
}
// Look how to pass parameter to the threads lambda:
// http://stackoverflow.com/questions/25536956/how-to-write-lambda-function-with-arguments-c
void Device::SendD2C_FwUpdateStatus(std::string cmdType, std::string status, std::string message)
{
	std::thread t([&](std::string commandType, std::string stat, std::string msg){
		try
		{
            // Get current firmware version from config file
            std::ifstream configFile("/home/debian/.ismdata/config.json");
            nlohmann::json config = nlohmann::json::parse(configFile);
            configFile.close();
            // Get logfile
			std::ifstream logFile("/home/debian/.ismdata/fw-update-log");
			std::string logData((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());
            logFile.close();
            // Response object
			nlohmann::json obj = {
					{"DeviceId", this->settings->getDeviceId()},
					{"FwUpdateStatus", stat},
                    {"Message", msg},
					{"Log", logData},
                    {"Version", config["softwareversion"]}
			};
			IOTHUB_MESSAGE_HANDLE messageHandle;
			char sendBuffer[MAX_SEND_BUFFER_SIZE];

			// fill send buffer
			sprintf_s(sendBuffer, MAX_SEND_BUFFER_SIZE, obj.dump().c_str());
			messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char*)sendBuffer, strlen(sendBuffer));

			// set event properties
			MAP_HANDLE propMap = IoTHubMessage_Properties(messageHandle);
			Map_AddOrUpdate(propMap, EventType::D2C_COMMAND.c_str(), commandType.c_str());

			std::cout << "send message, size=" << strlen(sendBuffer) << std::endl;
			std::cout << "CommandType: " << commandType << std::endl;

			// send the message
			IoTHubClient_SendEventAsync(iotHubClientHandle, messageHandle, SendConfirmationCallback, this);
		}
		catch(std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}, cmdType, status, message);

	t.detach();
}

void Device::SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback)
{
	//Device* d = (Device*)userContextCallback;

	std::cout << "send confirmed." << std::endl;
}



// TODO: free memory of e.g buffer?
IOTHUBMESSAGE_DISPOSITION_RESULT Device::ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void* userContextCallback)
{
	Device* d = (Device*)userContextCallback;
    // Message Properties
    //
    const char* buffer;
    MAP_HANDLE mapProperties = IoTHubMessage_Properties(message);
    if(mapProperties != nullptr)
    {
        const char*const* keys;
        const char*const* values;
        size_t propertyCount = 0;
        if (Map_GetInternals(mapProperties, &keys, &values, &propertyCount) == MAP_OK)
        {
            if (propertyCount > 0)
            {
                for(size_t index = 0; index < propertyCount; index++)
                {
					// Filter all iot hub events on a high level.
					// Handle only events with key EventType::C2D_COMMAND
                    if(std::string{keys[index]} == EventType::C2D_COMMAND)
                    {
						// Message Data
						//
    					size_t size;
    					if (IoTHubMessage_GetByteArray(message, (const unsigned char**)&buffer, &size) == IOTHUB_MESSAGE_OK)
    					{
							// !!! provide the exact size to std::string ctor
        					std::string msgbody{buffer, 0, size};

							// Switched by CommandType::<command> delegate to _state how to handle the command
                        	std::string cmd{values[index]};
                        	if(cmd == CommandType::START)
                        	{
								return d->Start();
                        	}
                       		else if(cmd == CommandType::STOP)
                        	{
								return d->Stop();
                        	}
                        	else if(cmd == CommandType::START_PREVIEW)
                        	{
								return d->StartPreview();
                        	}
                        	else if(cmd == CommandType::STOP_PREVIEW)
                        	{
								return d->StopPreview();
                        	}
                        	else if(cmd == CommandType::SET_DEVICE_SETTINGS)
                        	{
								return d->SetDeviceSettings(msgbody);
                        	}
                        	else if(cmd == CommandType::GET_DEVICE_SETTINGS)
                    	    {
              	              	return d->GetDeviceSettings();
            	            }
        	                else
    	                    {
								std::cout << "- Unknown CommandType" << std::endl;
                        	}
						}
                    }
                }
            }
        }
    }

	std::cout << "ReceiveMessageCallback returns with default IOTHUBMESSAGE_REJECTED." << std::endl;
	return IOTHUBMESSAGE_REJECTED;
}



// ReadyState
//
ReadyState::ReadyState(){}
ReadyState::~ReadyState(){}
ReadyState::ReadyState(ReadyState const& s){}
ReadyState& ReadyState::operator=(ReadyState const& s){}

std::string ReadyState::getStateName() {return "ReadyState";}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::Start(Device* d)
{
    // ACK msg
    // start timer
	StartCamera(d);
    std::cout << "+ Starting to run device!" << std::endl;
	ChangeState(d, &Singleton<RunState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::StartPreview(Device* d)
{
    // ACK msg
    // start timer
	StartCamera(d);
    std::cout << "+ Starting to run device in preview mode!" << std::endl;
	ChangeState(d, &Singleton<PreviewState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "+ READY_STATE: Set new DeviceSettings values!" << std::endl;

	if(UpdateSettings(d, msgbody) == true)
		return IOTHUBMESSAGE_ACCEPTED;
	else
		return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT ReadyState::GetDeviceSettings(Device* d)
{
	// ACK msg
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	SendD2C_DeviceSettings(d, CommandType::UPDATE_DASHBOARD_CONTROLS);
	return IOTHUBMESSAGE_ACCEPTED;
}



// RunState
//
RunState::RunState(){}
RunState::~RunState(){}
RunState::RunState(RunState const& s){}
RunState& RunState::operator=(RunState const&){}

std::string RunState::getStateName() {return "RunState";}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::Stop(Device* d)
{
    // ACK msg
    // stop timer
	StopCamera(d);
    std::cout << "+ Stop running the device and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to preview mode from running device, stop it first!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::StopPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in preview mode!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // NAK msg
    std::cout << "- RUN_STATE: Reject setting DeviceSettings values in RunState!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT RunState::GetDeviceSettings(Device* d)
{
	// ACK msg
	SendD2C_DeviceSettings(d, CommandType::UPDATE_DASHBOARD_CONTROLS);
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	return IOTHUBMESSAGE_ACCEPTED;
}



// PreviewState
//
PreviewState::PreviewState(){}
PreviewState::~PreviewState(){}
PreviewState::PreviewState(PreviewState const& s){}
PreviewState& PreviewState::operator=(PreviewState const& s){}

std::string PreviewState::getStateName() {return "PreviewState";}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::Start(Device* d)
{
    // NAK msg
    std::cout << "- Can't go to run mode from preview mode device, stop preview first!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::Stop(Device* d)
{
    // NAK msg
    std::cout << "- Device is not running in run mode!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::StartPreview(Device* d)
{
    // NAK msg
    std::cout << "- Device is already running in preview mode!" << std::endl;

	return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::StopPreview(Device* d)
{
    // ACK msg
    // stop timer
	StopCamera(d);
    std::cout << "+ Stop running the preview and go back to ready!" << std::endl;
	ChangeState(d, &Singleton<ReadyState>::Instance());

	return IOTHUBMESSAGE_ACCEPTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::SetDeviceSettings(Device* d, std::string msgbody)
{
    // deserialize msg
    // set new DeviceSettings values
    // ACK msg
    std::cout << "+ PREVIEW_STATE: Set new DeviceSettings values!" << std::endl;

	if(UpdateSettings(d, msgbody) == true)
		return IOTHUBMESSAGE_ACCEPTED;
	else
		return IOTHUBMESSAGE_REJECTED;
}

IOTHUBMESSAGE_DISPOSITION_RESULT PreviewState::GetDeviceSettings(Device* d)
{
	// ACK msg
	SendD2C_DeviceSettings(d, CommandType::UPDATE_DASHBOARD_CONTROLS);
	std::cout << "+ Send DeviceSettings D2C message" << std::endl;

	return IOTHUBMESSAGE_ACCEPTED;
}



// DeviceSettings
//
DeviceSettings::DeviceSettings()
{

}

DeviceSettings::DeviceSettings(std::string DeviceId, std::string StateName, int CapturePeriod, std::string CurrentCaptureUri, // general settings
							   double VarianceThreshold, double DistanceMapThreshold, double RGThreshold, double RestrictedFillingThreshold, double DilateValue,     // Matlab Algorithm Settings
							   int Gain, double Exposure, 								   // camera settings
							   int PulseWidth, int Current, int Predelay)        				   // pulse settings
{
	this->DeviceId = DeviceId;
	this->StateName = StateName;
	this->CapturePeriod = CapturePeriod;
	this->CurrentCaptureUri = CurrentCaptureUri;
	this->VarianceThreshold = VarianceThreshold;
	this->DistanceMapThreshold = DistanceMapThreshold;
	this->RGThreshold = RGThreshold;
	this->RestrictedFillingThreshold = RestrictedFillingThreshold;
	this->DilateValue = DilateValue;
	this->Gain = Gain;
	this->Exposure = Exposure;
	this->PulseWidth = PulseWidth;
	this->Current = Current;
	this->Predelay = Predelay;
}

DeviceSettings::~DeviceSettings()
{
}

std::string DeviceSettings::Serialize()
{
	nlohmann::json obj = {
		{"DeviceId", DeviceId},
		{"StateName", StateName},
		{"CapturePeriod", CapturePeriod},
		{"CurrentCaptureUri", CurrentCaptureUri},
		{"VarianceThreshold", VarianceThreshold},
		{"DistanceMapThreshold", DistanceMapThreshold},
		{"RGThreshold", RGThreshold},
		{"RestrictedFillingThreshold", RestrictedFillingThreshold},
		{"DilateValue", DilateValue},
		{"Gain", Gain},
		{"Exposure", Exposure},
		{"PulseWidth", PulseWidth},
		{"Current", Current},
		{"Predelay", Predelay}
	};

	return obj.dump();
}

void DeviceSettings::Deserialize(std::string jsonStr)
{
	try
	{
		nlohmann::json values  = nlohmann::json::parse(jsonStr);

		DeviceId = values["DeviceId"];
    	StateName = values["StateName"];
		CapturePeriod = values["CapturePeriod"];
		CurrentCaptureUri = values["CurrentCaptureUri"];
		VarianceThreshold = values["VarianceThreshold"];
		DistanceMapThreshold = values["DistanceMapThreshold"];
		RGThreshold = values["RGThreshold"];
		RestrictedFillingThreshold = values["RestrictedFillingThreshold"];
		DilateValue = values["DilateValue"];
		Gain = values["Gain"];
		Exposure = values["Exposure"];
		PulseWidth = values["PulseWidth"];
		Current = values["Current"];
		Predelay = values["Predelay"];
	}
	catch(std::exception& e)
	{
		std::cout << "Error on DeviceSettings deserialization" << std::endl;
		std::cout << e.what() << std::endl;
	}
}


void DeviceSettings::Report()
{
	std::cout << "DeviceId: " <<  DeviceId << std::endl;
	std::cout << "StateName: " << StateName << std::endl;
	std::cout << "CapturePeriod: " << CapturePeriod << std::endl;
	std::cout << "CurrentCaptureUri: " << CurrentCaptureUri << std::endl;
	std::cout << "VarianceThreshold: " << VarianceThreshold << std::endl;
	std::cout << "DistanceMapThreshold: " << DistanceMapThreshold << std::endl;
	std::cout << "RGThreshold: " << RGThreshold << std::endl;
	std::cout << "RestrictedFillingThreshold: " << RestrictedFillingThreshold << std::endl;
	std::cout << "DilateValue: " << DilateValue << std::endl;
	std::cout << "Gain: " << Gain << std::endl;
	std::cout << "Exposure: " << Exposure << std::endl;
	std::cout << "PulseWidth: " << PulseWidth << std::endl;
	std::cout << "Current: " << Current << std::endl;
	std::cout << "Predelay: " << Predelay << std::endl;
}
