#ifndef DIR_CAMERA_H
#define DIR_CAMERA_H

#include <thread>
#include <iostream>
#include <string.h>
#include <time.h>
#include <chrono>
#include "PubSub.h"
#include "was/storage_account.h"
#include "was/blob.h"
#include <dirent.h>
#include <vector>
#include <stdlib.h>

class DirCamera
{
public: 
	DirCamera(utility::string_t, utility::string_t, std::string, std::string);
	~DirCamera();
	
	void Start();
	void Stop();
	void GetCameraInfo();
	void setCapturePeriod(int period);

public:	// public exposed Publishers aka Events
	CaptureNotificationPublisher NewCaptureUploaded;

private:
	void HandleCaptures();
	void InitBlobStorage();
	void InitCamera();
	void UploadCaptureToBlobStorage(std::string);

private:
	std::string directory;
	bool running;
	int capturePeriod;
	std::vector<std::string> captures;

	// Blob storage
	const utility::string_t containerName;
	const utility::string_t storageConnectionString;
	const std::string storageAccountName;
	azure::storage::cloud_blob_container container;
};	
#endif
