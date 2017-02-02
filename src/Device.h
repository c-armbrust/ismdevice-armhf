#ifndef DEVICE_H
#define DEVICE_H

#include "azure_c_shared_utility/platform.h"
#include "azure_c_shared_utility/threadapi.h"
#include "azure_c_shared_utility/crt_abstractions.h"
#include "iothub_client.h"
#include "iothub_message.h"
#include "iothubtransportamqp.h"
#include <thread>
#include "Camera.h"
#include "FirmwareUpdateHandler.h"

// max event message size on IoT Hub = max brokered message size for servicebus = 256kB
#define MAX_SEND_BUFFER_SIZE 262144



class DeviceState;
template<class T> class Singleton;
class DeviceSettings;

class Device : public Subscriber
{
public:
	Device();
	~Device();
	IOTHUBMESSAGE_DISPOSITION_RESULT Start();
	IOTHUBMESSAGE_DISPOSITION_RESULT Stop();
	IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview();
	IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview();
	IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(std::string);
	IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings();

	void ReceiveC2D();

	// Pub Sub attach (not possible in ctor because this ptr is required)
	void SubscribeNotifications();

    // Device direct method callback function
    static int DeviceMethodCallback(const char* method_name, const unsigned char* payload, size_t size, unsigned char** response, size_t* resp_size, void* userContextCallback);
    void FirmwareUpdate(std::string blobUrl, std::string fileName);


private:
	friend class DeviceState;

	void ChangeState(DeviceState*);
	bool UpdateSettings(std::string);
	void OnNotification(Publisher*); // Pub Sub interface
	std::string getDeviceId(const std::string& connectionString);
	static IOTHUBMESSAGE_DISPOSITION_RESULT ReceiveMessageCallback(IOTHUB_MESSAGE_HANDLE, void*);
	static void SendConfirmationCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT, void*);
	void SendD2C_DeviceSettings(std::string);
	void StartCamera();
	void StopCamera();
	void SetCameraPruValues();

private:
	DeviceState* _state;
	IOTHUB_CLIENT_HANDLE iotHubClientHandle;
	DeviceSettings* settings;
	Camera* camera;

public:
    FirmwareUpdateHandler* firmwareUpdateHandler;
    std::string publicKeyUrl;
};



class DeviceState
{
public:
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string)=0;
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*)=0;
	virtual std::string getStateName()=0;

protected:
	void ChangeState(Device*, DeviceState*);
	bool UpdateSettings(Device*, std::string);
	void SendD2C_DeviceSettings(Device*, std::string);
	void StartCamera(Device* d);
	void StopCamera(Device* d);
};



class ReadyState : public DeviceState
{
public:
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);
	virtual std::string getStateName();

private:
	template<class T> friend class Singleton;
	ReadyState(); // ctor hidden
	~ReadyState(); // dtor hidden
	ReadyState(ReadyState const&); // cp ctor hidden
	ReadyState& operator=(ReadyState const&); // assign op hidden
};

class RunState : public DeviceState
{
public:
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);
	virtual std::string getStateName();

private:
	template<class T> friend class Singleton;
	RunState(); // ctor hidden
	~RunState(); // dtor hidden
	RunState(RunState const&); // cp ctor hidden
	RunState& operator=(RunState const&); // assign op hidden
};

class PreviewState : public DeviceState
{
public:
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Start(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT Stop(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StartPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT StopPreview(Device*);
    virtual IOTHUBMESSAGE_DISPOSITION_RESULT SetDeviceSettings(Device*, std::string);
	virtual IOTHUBMESSAGE_DISPOSITION_RESULT GetDeviceSettings(Device*);
	virtual std::string getStateName();

private:
	template<class T> friend class Singleton;
	PreviewState(); // ctor hidden
	~PreviewState(); // dtor hidden
	PreviewState(PreviewState const&); // cp ctor hidden
	PreviewState& operator=(PreviewState const&); // assign op hidden
};



class  DeviceSettings
{
public:
	DeviceSettings();
	DeviceSettings(std::string, std::string, int, std::string, // general settings
				   double, double, double, double, double,	   // Matlab Algorithm settings
				   int, double,								   // camera settings
				   int, int, int);	   // pulse settings
	DeviceSettings(DeviceSettings const&);
	DeviceSettings& operator=(DeviceSettings const&);
	~DeviceSettings();

// Serialization / Deserialization
	std::string Serialize();
	void Deserialize(std::string);
	void Report();

// get/set methods
public:
	inline std::string getDeviceId() {return DeviceId;}
	inline std::string getStateName() {return StateName;}
	inline int getCapturePeriod() {return CapturePeriod;}
	inline std::string getCurrentCaptureUri() {return CurrentCaptureUri;}
	inline double getVarianceThreshold() {return VarianceThreshold;}
	inline double getDistanceMapThreshold() {return DistanceMapThreshold;}
	inline double getRGThreshold() {return RGThreshold;}
	inline double getRestrictedFillingThreshold() {return RestrictedFillingThreshold;}
	inline double getDilateValue() {return DilateValue;}
	inline int getGain() {return Gain;}
	inline double getExposure() {return Exposure;}
	inline int getPulseWidth() {return PulseWidth;}
	inline int getCurrent() {return Current;}
	inline int getPredelay() {return Predelay;}

	inline void setDeviceId(std::string v) {DeviceId = v;}
	inline void setStateName(std::string v) {StateName = v;}
	inline void setCapturePeriod(int v) {CapturePeriod = v;}
	inline void setCurrentCaptureUri(std::string v) {CurrentCaptureUri = v;}
	inline void setVarianceThreshold(double v) {VarianceThreshold = v;}
	inline void setDistanceMapThreshold(double v) {DistanceMapThreshold = v;}
	inline void setRGThreshold(double v) {RGThreshold = v;}
	inline void setRestrictedFillingThreshold(double v) {RestrictedFillingThreshold = v;}
	inline void setDilateValue(double v) {DilateValue = v;}
	inline void setGain(int v) {Gain = v;}
	inline void setExposure(double v) {Exposure = v;}
	inline void setPulseWidth(int v) {PulseWidth = v;}
	inline void setCurrent(int v) {Current = v;}
	inline void setPredelay(int v) {Predelay = v;}

private:
	std::string DeviceId;
	std::string StateName;
	int CapturePeriod;
	std::string CurrentCaptureUri;

	// Matlab Filament-Algorithm params
	double VarianceThreshold;
	double DistanceMapThreshold;
	double RGThreshold;
	double RestrictedFillingThreshold;
	double DilateValue;

	// Camera Settings
	int Gain;
	double Exposure;

	// Pulse Settings
	int PulseWidth;
	int Current;
	int Predelay;
};

#endif
