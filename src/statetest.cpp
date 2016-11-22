#include <iostream>
#include "Device.h"

int main()
{
	Device* d = new Device(); 

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
