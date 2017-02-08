#include <iostream>
#include "Device.h"

int main(int argc, char** argv)
{
	int opt;
    std::string configFile;
    bool useFile = false;
	while((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
			case 'f': {
				configFile = optarg;
                useFile = true;
				std::cout << "Reading from file " << configFile << std::endl;
				break;
			}
			case '?': {
				if (optopt == 'f')
					std::cerr << "Option -f requires an argument.\n";
				else if (isprint (optopt))
					std::cerr << "Unknown option '-" << (char)optopt << "'\n";
				else
					fprintf (stderr,
							 "Unknown option character `\\x%x'.\n",
							 optopt);
				return 1;
			}
			default:
				break;
		}

	}
	Device* d = new Device();
	d->SubscribeNotifications();

	// Full state test
	/*
	d->Stop();
	d->StopPreview();
	std::cout << std::endl;

	d->Start();
	d->Stop();
	d->StartPreview();
	d->StopPreview();
	d->Stop();
	std::cout << std::endl;

	d->StartPreview();
	d->StartPreview();
	d->Start();
	d->Stop();
	d->StopPreview();
	std::cout << std::endl;
	*/

	// IoT Hub
	std::thread receivec2d([&]{ d->ReceiveC2D(); });
	//receivec2d.join();
	receivec2d.detach();

	while(1)
	{
		ThreadAPI_Sleep(1000);
	}
    return 0;
}
