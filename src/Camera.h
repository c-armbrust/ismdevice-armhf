#ifndef CAMERA_H
#define CAMERA_H

#include <thread>
#include <unistd.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <uEye.h>
#include <iostream>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "was/storage_account.h"
#include "was/blob.h"
#include <cstdio>
#include "PubSub.h"


class Camera
{
public:
	Camera(utility::string_t, utility::string_t, std::string);
	~Camera();

	void Start();
	void Stop();
	void GetCameraInfo();

// get/set methods
	unsigned int getTriggerduration();
	unsigned int getPulseduration();
	unsigned int getPauseduration();
	unsigned int getPixelclock();
	double getExposure();
	unsigned int getGain();
	double getFps();
	void setTriggerduration(unsigned int);
	void setPulseduration(unsigned int);
	void setPauseduration(unsigned int);
	bool setPixelclock(unsigned int);
	bool setExposure(double);
	bool setGain(unsigned int);

public: //public exposed  Publishers aka Events
	CaptureNotificationPublisher NewCaptureUploaded;

private:
	void InitBlobStorage(utility::string_t storageConnectionString);
	void InitPRU();
	void InitCamera();
	void HandleCaptures();
	void terminate_on_error(HIDS);
	void UploadCaptureToBlobStorage(std::string);

private:
	// Blob Storage
	const utility::string_t containerName;
	const std::string storageAccountName;
	azure::storage::cloud_blob_container container;

	// PRU
	tpruss_intc_initdata pruss_intc_initdata;
	unsigned int* pruDataMem;
	unsigned int triggerduration;
	unsigned int pulseduration;
	unsigned int pauseduration;

	// IDS cam
	HIDS hCam;
	const int sizeX = 640;
	const int sizeY = 480;
	const int bitsPerPixel = 8;
	double fps;
	double FpsRange[3]; // min, max, interval (fps_min = 1/max; fps_max = 1/min; fps_n = 1/(min+n*interval))

	unsigned int pixelclock;
	unsigned int PixelclockRange[3]; // min, max, increment
	double exposure;
	double ExposureRange[3]; // min, max, increment
	unsigned int gain;
	unsigned int GainRange[3];
	char* imageMem;
	int imageMemID;
};

#endif
