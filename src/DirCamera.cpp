#include "DirCamera.h"

DirCamera::DirCamera(utility::string_t stoconnstr, utility::string_t containername, std::string storageaccname, std::string dir) : storageConnectionString{stoconnstr}, containerName{containername}, storageAccountName{storageaccname}, directory{dir}, running{false}, capturePeriod{10}
{
	// init random seed
	srand(time(nullptr));	

	InitBlobStorage();
	InitCamera();
	
	std::cout << "storage account: " << storageAccountName << std::endl;
	std::cout << "containername: " << containerName << std::endl;
	std::cout << "directory: " << directory << std::endl;
}

DirCamera::~DirCamera()
{

}

void DirCamera::Start()
{
	this->running = true;
	std::thread t([&]{
		HandleCaptures();
	});
	t.detach();
}

void DirCamera::Stop()
{
	this->running = false;
}

void DirCamera::setCapturePeriod(int period)
{
	capturePeriod = period;
}

void DirCamera::GetCameraInfo()
{
	std::cout << std::endl;
	std::cout << "Kamera Infos:\n"; 
	std::cout << "--------------------\n";
	std::cout << "Simulated directory camera" << std::endl;
}

void DirCamera::HandleCaptures()
{
	while(this->running)
	{
		// Get a random file from the loaded capture files
		std::string file = captures[rand() % captures.size()];

		// Upload to Blob Storage
		UploadCaptureToBlobStorage(file);
			
			
		std::this_thread::sleep_for(std::chrono::seconds(capturePeriod));
		std::cout << "running: " << this->running << std::endl;
	}
}

void DirCamera::InitCamera()
{
	// Load the capture files paths in a vector<string>
	//

	DIR* dp;
	struct dirent* dirp;
	if((dp = opendir(directory.c_str())) == nullptr)
	{
		std::cout << "error: opendir failed" << std::endl;
	}
	while((dirp = readdir(dp)) != nullptr)
	{
		std::string fn = std::string(dirp->d_name);
		if(fn.find(".jpg") != std::string::npos)
			captures.push_back(directory + "/" + fn);
	}
	closedir(dp);
	
	// Debug print
	std::cout << "Directory camera loaded captures: " << std::endl;
	for(int i=0; i< captures.size(); i++)
	{
		std::cout << captures[i] << std::endl;
	}
	std::cout << std::endl;
}

void DirCamera::InitBlobStorage()
{
	try
	{
		std::cout << "Create Azure Storage container " << containerName << " on Storage Account " << storageAccountName << " if not exists" << std::endl;

		// Retrieve storage account from connection string.
		azure::storage::cloud_storage_account storage_account = azure::storage::cloud_storage_account::parse(storageConnectionString);
	
		// Create the blob client
		azure::storage::cloud_blob_client blob_client = storage_account.create_cloud_blob_client();

		// Retrieve a reference to a container
		this->container = blob_client.get_container_reference(containerName);

		// Create the contaeiner if it doesn't already exist.
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

void DirCamera::UploadCaptureToBlobStorage(std::string filename)
{
	try
	{
		// Retrieve reference to a blob named <filename> without the path.
		std::string blobname = filename.substr(filename.find_last_of("/") + 1, filename.length() - filename.find_last_of("/"));
		azure::storage::cloud_block_blob blockBlob = container.get_block_blob_reference(blobname);

		// Create or overwrite the <filename> blob with contents from a local file.
		blockBlob.upload_from_file(utility::string_t{filename});

		// Set CaptureNotification
		CaptureNotification notification(blobname);
		NewCaptureUploaded.SetNotification(&notification);
		// Notify all subscribers
		NewCaptureUploaded.Notify();
	}
	catch(const std::exception& e)
	{
		std::wcout << U("Error: ") << e.what() << std::endl;
	}
}
