#include "Camera.h"
#include <iostream>

Camera::Camera(utility::string_t stoconnstr, utility::string_t containername, std::string storageaccname) : storageConnectionString{stoconnstr}, containerName{containername}, storageAccountName{storageaccname}
{
	InitBlobStorage();

	InitPRU();

//	fps = 2.0; // fps setzen macht nur im freilaufenden Modus Sinn
	exposure = 200.0;
	gain = 30;

	InitCamera();
}

Camera::~Camera()
{
	// Disable PRU0 and close memory mappings
	prussdrv_pru_disable(0);
	prussdrv_exit();

	// IDS camera
	// Cleanup
	if(is_FreeImageMem(hCam, imageMem, imageMemID) != IS_SUCCESS){
		std::cout<<"Free image memory error\n";
		terminate_on_error(hCam);
	}
	free(imageMem);

	if(is_ExitCamera(hCam) != IS_SUCCESS){
		std::cout<<"Exit camera error\n";
		terminate_on_error(hCam);
	}
}

void Camera::terminate_on_error(HIDS hCam)
{
	INT pErr; // Error code
	IS_CHAR* ppcErr; // Error text
	is_GetError(hCam, &pErr, &ppcErr);
	std::cout<<"Error with ids camera occured:\n";
	std::cout<<"Code: "<<pErr<<std::endl;
	std::cout<<"Text: "<<ppcErr<<std::endl;
	is_ExitCamera(hCam);
	exit(1);
}


void Camera::UploadCaptureToBlobStorage(std::string filename)
{
	try
	{
		// Retrieve reference to a blob named <filename>.
		azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(filename);

		// Create or overwrite the <filename> blob with contents from a local file.
		blockBlob.upload_from_file(utility::string_t{filename});

		// Delete local file
		std::remove(filename.c_str());

		std::string uri = "https://" + storageAccountName + ".blob.core.windows.net/" + containerName + "/" + filename;

		// Set CaptureNotification
		CaptureNotification notification(uri);
		NewCaptureUploaded.SetNotification(&notification);
		// Notify all subscribers
		NewCaptureUploaded.Notify();
	}
	catch(const std::exception& e)
	{
		std::wcout << U("Error: ") << e.what() << std::endl;
	}
}



void Camera::InitBlobStorage()
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



void Camera::InitCamera()
{
	hCam = 0; // 0: open the first free camera
	if(is_InitCamera(&hCam, NULL) != IS_SUCCESS){
		std::cout<<"Init camera error\n";
		terminate_on_error(hCam);
	}

	// Set Bitmap-Mode (store digital image in system memory)
	if(is_SetDisplayMode(hCam, IS_SET_DM_DIB) != IS_SUCCESS){
		std::cout<<"Set display mode error\n";
		terminate_on_error(hCam);
	}

	// is_AllocImageMem
	char* ppcImgMem; // Pointer to image memory
	int pid; // ID for image memory
	if(is_AllocImageMem(hCam, sizeX, sizeY, bitsPerPixel, &ppcImgMem, &pid) != IS_SUCCESS){
		std::cout<<"Alloc image mem error\n";
		terminate_on_error(hCam);
	}

	// Calculate image buffer size
	// Calculate line increment
	const int line = sizeX * int((bitsPerPixel + 7) / 8);
	const int adjust = line % 4 == 0 ? 0 : 4 - line % 4;
	// adjust is 0 if line mod 4 eq 0, rest(line / 4) else.
	const int lineinc = line + adjust;
	const int BufferSize = ( sizeX * int((bitsPerPixel + 7) / 8) + adjust) * sizeY;
	std::cout << "BufferSize = " << BufferSize << " Byte\n";

	// is_SetAllocatedImageMem
	imageMem = new char[BufferSize];
    mlock(imageMem, BufferSize);
	// set imageMemID and imageMem member variables
	if(is_SetAllocatedImageMem(hCam, sizeX, sizeY, bitsPerPixel, imageMem, &imageMemID) != IS_SUCCESS){
		std::cout<<"Set allocated image mem error\n";
		terminate_on_error(hCam);
	}

	// Set image memory as aktive memory
	if(is_SetImageMem(hCam, imageMem, imageMemID) != IS_SUCCESS){
		std::cout<<"Activate image memory error\n";
		terminate_on_error(hCam);
	}

	// Set trigger mode
	if(is_SetExternalTrigger(hCam, IS_SET_TRIGGER_LO_HI) != IS_SUCCESS){
		std::cout<<"Set trigger mode error\n";
		terminate_on_error(hCam);
	}

	// Get pixel clock range
	INT nRet = is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET_RANGE, (void*)PixelclockRange, sizeof(PixelclockRange));
	if(nRet == IS_SUCCESS)
	{
		std::cout<<"Pixelclock infos:\n";
		std::cout << PixelclockRange[0] << " MHz - " << PixelclockRange[1] << " MHz\n";
		std::cout << "Increment: " << PixelclockRange[2] << " MHz\n";
	}
	else
	{
		std::cout<<"Determine pixel clock range error\n";
		terminate_on_error(hCam);
	}

	// Set pixel clock to the min pixel clock
    if(is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&PixelclockRange[0], sizeof(PixelclockRange[0])) != IS_SUCCESS) {
		std::cout<<"Set pixel clock error:\n";
		terminate_on_error(hCam);
	}

	// Get current pixel clock
    UINT nPixelClock;
    if(is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&nPixelClock, sizeof(nPixelClock)) != IS_SUCCESS) {
        std::cout<<"Get pixel clock error\n";
        terminate_on_error(hCam);
    }
    else {
        std::cout << "Pixelclock = " << nPixelClock << std::endl;
        pixelclock = nPixelClock; // set the member variable
    }

	// FPS
	// FPS_min = 1/max
	// FPS_max = 1/min
	// FPS_n = 1/(min+n*Interval)
	double min, max, interval;
	if(is_GetFrameTimeRange(hCam, &FpsRange[0], &FpsRange[1], &FpsRange[2]) != IS_SUCCESS){
		std::cout<<"Get fps range error\n";
		terminate_on_error(hCam);
	}
	else
	{
		std::cout << "FPS: " << 1/FpsRange[1] << " - " << 1/FpsRange[0] << std::endl;
		std::cout << "Increment: " << FpsRange[2] << std::endl;
	}

	// Set framerate
	double newFPS;
	if(is_SetFrameRate(hCam, IS_GET_FRAMERATE, &newFPS) != IS_SUCCESS){
		std::cout<<"Set framerate error\n";
		terminate_on_error(hCam);
	}
	else
		std::cout << "Currently set FPS value: " << newFPS << std::endl;

	// TODO English translation
	// Get Exposure capabilities
	UINT nCaps = 0;
	nRet = is_Exposure(hCam, IS_EXPOSURE_CMD_GET_CAPS, (void*)&nCaps, sizeof(nCaps));
	if (nRet == IS_SUCCESS)
	{	std::cout << "Exposure capabilities:\n";
		if(nCaps & IS_EXPOSURE_CAP_EXPOSURE)
			std::cout << "Die Belichtungszeiteinstellung wird unterstuetz\n";
  		if (nCaps & IS_EXPOSURE_CAP_FINE_INCREMENT)
      		std::cout << "Das feine Belichtungszeitraster wird unterstuetz\n";
		if(nCaps & IS_EXPOSURE_CAP_LONG_EXPOSURE)
			std::cout << "Die Langzeitbelichtung wird unterstuetz\n";
		if(nCaps & IS_EXPOSURE_CAP_DUAL_EXPOSURE)
			std::cout << "Der Sensor unterstuetzt die duale Belichtun\n";
	}
	else
	{
		std::cout << "Get exposure caps error\n";
		terminate_on_error(hCam);
	}

	// Get Exposure Infos (values in ms)
	ExposureRange[3];
	if(is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE, (void*)&ExposureRange, sizeof(ExposureRange)) != IS_SUCCESS){
		std::cout << "Get exposure range infos error\n";
		terminate_on_error(hCam);
	}

	std::cout << "Min exposure: " << ExposureRange[0] << " ms" << std::endl;
	std::cout << "Max exposure: " << ExposureRange[1] << " ms" << std::endl;
	std::cout << "Exposure increment: " << ExposureRange[2] << " ms" << std::endl;

	// Set min exposure time
	std::cout << "Set exposure to " << exposure << " ms\n";
	if(is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&exposure, sizeof(exposure)) != IS_SUCCESS){
		std::cout << "Set exposure error\n";
		terminate_on_error(hCam);
	}

	// Currently set exposure
	double nExposure;
	if(is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)&nExposure, sizeof(nExposure)) != IS_SUCCESS){
		std::cout << "Get current exposure error\n";
		terminate_on_error(hCam);
	}
	else
		std::cout << "Current Exposure: " << nExposure << " ms" << std::endl;

	// Activate and set hardware gain (Gain Boost)
	std::cout << "Enable hardware gain boost.\n";
	if(is_SetGainBoost(hCam, IS_SET_GAINBOOST_ON) != IS_SUCCESS){
		std::cout << "is_SetGainBoost error:\n";
		terminate_on_error(hCam);
	}

	// HW Gain
	// Range allowed to set by is_SetHardwareGain is simply between 0% and 100%
	GainRange[0] = 0;
	GainRange[1] = 100;
	GainRange[2] = 1;

	if(is_SetHardwareGain(hCam, gain, 0, 0, 0) != IS_SUCCESS)
	{
		std::cout << "is_SetHardwareGain error:\n";
		terminate_on_error(hCam);
	}

	// Get Master Gain to display it.
	int mastergain;
	mastergain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, 0, 0, 0);
	std::cout << "Set hardware gain to: " << mastergain << std::endl;

	// Enable FRAME-Event
	is_EnableEvent(hCam, IS_SET_EVENT_FRAME);

	std::cout << "Set continious trigger mode\n";
	// Continious trigger mode
	if(is_CaptureVideo(hCam, IS_WAIT) != IS_SUCCESS){
		std::cout << "is_CaptureVideo error:\n";
		terminate_on_error(hCam);
	}
}

// TODO: English translation
void Camera::GetCameraInfo()
{
	CAMINFO pInfo;
	if(is_GetCameraInfo(hCam, &pInfo) != IS_SUCCESS){
		std::cout<<"Get camera info error\n";
		terminate_on_error(hCam);
	}
	std::cout << std::endl;
	std::cout << "Kamera Infos:\n";
	std::cout << "--------------------\n";
	std::cout << "Seriennummer: " << pInfo.SerNo << std::endl;
	std::cout << "Hersteller: " << pInfo.ID << std::endl;
	std::cout << "HW Version USB Board: " << pInfo.Version << std::endl;
	std::cout << "Systemdatum des Endtests: " << pInfo.Date << std::endl;
	std::cout << "Kamera-ID: " << (int)pInfo.Select << std::endl; // Ab Werk steht die Kamera-ID auf 1. Kann mit is_SetCameraID geaendert werden
	if(pInfo.Type == IS_CAMERA_TYPE_UEYE_USB_SE)
		std::cout << "Kameratyp: USB uEye SE" << std::endl;
	std::cout << std::endl;

	std::cout << "ueye API Info\n";
	std::cout << "--------------------\n";
	int version = is_GetDLLVersion();
	int build = version & 0xFFFF;
	version = version >> 16;
	int minor = version & 0xFF;
	version = version >> 8;
	int major = version & 0xFF;
	std::cout << "API version " << major << "." << minor << "." << build << std::endl;
	std::cout << std::endl;
}



void Camera::InitPRU()
{
    // Initialize structure used by prussdrv_pruintc_intc
    // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
    pruss_intc_initdata = PRUSS_INTC_INITDATA;

	// Allocate and initialize memory
	prussdrv_init();
	prussdrv_open(PRU_EVTOUT_0);

	// Map PRU0 Data RAM0
	prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pruDataMem);

	// Default Values
    triggerduration = 0x00000190; // 1ms - 100000 (dec) - 0x00000190 (hex)
    pulseduration = 0x00002710; // 4us - 400 (dec) - 0x00002710 (hex)
    pauseduration = 0x05F5E100; // 1s - 100000000 (dec) - 0x05F5E100 (hex)



    // Trigger duration
    // Write a byte into PRU0 Data RAM0. Offset = 0 Bytes, Data Bytes = 4
	pruDataMem[0] = triggerduration;

	// Pulse duration
   	// Write a byte into PRU0 Data RAM0. Offset = 4 Bytes, Data Bytes = 4
   	pruDataMem[1] = pulseduration;

   	// Pause duration
   	// Write a byte into PRU0 Data RAM0. Offset = 8 Bytes, Data Bytes = 4
	pruDataMem[2] = pauseduration;

	// turn loop of PRU program off
    pruDataMem[3] = 0x00000000;

    // Map PRU's interrupts
    prussdrv_pruintc_init(&pruss_intc_initdata);
}



void Camera::Start()
{
	// turn loop of PRU program on
	pruDataMem[3] = 0x00000001;

	// Load and execute the PRU program on PRU0
	prussdrv_exec_program (0, "./pulse.bin");


	std::thread t([&]{
		// Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
    	int n = prussdrv_pru_wait_event(PRU_EVTOUT_0);
		std::cout << "PRU program completed, event number " << n << std::endl;
	});
	t.detach();


	// Handle captures on each trigger
	std::thread u([&]{
		HandleCaptures();
	});
	u.detach();
}



void Camera::Stop()
{
	// turn loop of PRU program off
	pruDataMem[3] = 0x00000000;
}



void Camera::HandleCaptures()
{
	INT nRet;
	while(true)
	{
		nRet = IS_NO_SUCCESS;
		nRet = is_WaitEvent(hCam, IS_SET_EVENT_FRAME, INFINITE);
		if(nRet == IS_SUCCESS)
		{
			// Save image to file
			// Save jpeg from active memory with quality 80 (without file open dialog)
			IMAGE_FILE_PARAMS ImageFileParams;

			// pnImageID and ppcImageMem equals NULL -> use the image in the active image memory
			ImageFileParams.pnImageID = NULL;
			ImageFileParams.ppcImageMem = NULL;

			// Captures are named "capture" + <timestamp> + ".jpg"
			std::wstring filename = L"capture" + std::to_wstring(time(0)) + L".jpg";
			ImageFileParams.pwchFileName = (wchar_t*)filename.c_str();
			ImageFileParams.nFileType = IS_IMG_JPG;
			ImageFileParams.nQuality = 80;

			if(is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams, sizeof(ImageFileParams)) != IS_SUCCESS){
				std::cout<<"Save image to file error\n";
				terminate_on_error(hCam);
			}
			std::wcout << L"Saved image to " << filename << std::endl;

			// Upload to Blob Storage
			UploadCaptureToBlobStorage(std::string(filename.begin(), filename.end()));
		}
	}
}



// get/set methods
unsigned int Camera::getTriggerduration() { return triggerduration; }
unsigned int Camera::getPulseduration() { return pulseduration; }
unsigned int Camera::getPauseduration() { return pauseduration; }

void Camera::setTriggerduration(unsigned int v)
{
	triggerduration = v;

	// Write a byte into PRU0 Data RAM0. Offset = 0 Bytes, Data Bytes = 4
	pruDataMem[0] = triggerduration;
}

void Camera::setPulseduration(unsigned int v)
{
	pulseduration = v;

    // Write a byte into PRU0 Data RAM0. Offset = 4 Bytes, Data Bytes = 4
	pruDataMem[1] = pulseduration;

}

void Camera::setPauseduration(unsigned int v)
{
	pauseduration = v;

	// Write a byte into PRU0 Data RAM0. Offset = 8 Bytes, Data Bytes = 4
	pruDataMem[2] = pauseduration;
}



unsigned int Camera::getPixelclock()
{
	if(is_PixelClock(hCam, IS_PIXELCLOCK_CMD_GET, (void*)&pixelclock, sizeof(pixelclock)) != IS_SUCCESS) {
		std::cout<<"Get pixel clock error\n";
		terminate_on_error(hCam);
	}
	return pixelclock;
}

double Camera::getExposure()
{
	if(is_Exposure(hCam, IS_EXPOSURE_CMD_GET_EXPOSURE, (void*)&exposure, sizeof(exposure)) != IS_SUCCESS){
		std::cout << "Get current exposure error\n";
		terminate_on_error(hCam);
	}
	return exposure;
}

unsigned int Camera::getGain()
{
	gain = is_SetHardwareGain(hCam, IS_GET_MASTER_GAIN, 0, 0, 0);
	return gain;
}

double Camera::getFps()
{
	// Get frames per second
	if(is_GetFramesPerSecond(hCam, &fps) != IS_SUCCESS){
		std::cout<<"Get FPS error\n";
		terminate_on_error(hCam);
	}
	return fps;
}

bool Camera::setPixelclock(unsigned int v)
{
    if(is_PixelClock(hCam, IS_PIXELCLOCK_CMD_SET, (void*)&v, sizeof(v)) != IS_SUCCESS) {
		std::cout<<"Set pixel clock error:\n";
		//terminate_on_error(hCam);
		return false;
	}
	return true;
}

bool Camera::setExposure(double v)
{
	if(is_Exposure(hCam, IS_EXPOSURE_CMD_SET_EXPOSURE, (void*)&v, sizeof(v)) != IS_SUCCESS){
		std::cout << "Set exposure error\n";
		//terminate_on_error(hCam);
		return false;
	}
	return true;
}

bool Camera::setGain(unsigned int v)
{
	if(is_SetHardwareGain(hCam, v, 0, 0, 0) != IS_SUCCESS)
	{
		std::cout << "is_SetHardwareGain error:\n";
		//terminate_on_error(hCam);
		return false;
	}
	return true;
}
