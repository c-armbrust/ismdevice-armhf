#include "PubSub.h"

Notification::~Notification(){}
//CaptureNotification::~CaptureNotification(){}

CaptureNotification::CaptureNotification(std::string uri)
{
	CurrentCaptureUri = uri;
}

void CaptureNotificationPublisher::Attach(Subscriber* sub)
{
	subscribers.push_back(sub);
}

void CaptureNotificationPublisher::Detach(Subscriber* sub)
{
	subscribers.remove(sub);
}

void CaptureNotificationPublisher::Notify()
{
	auto it = subscribers.begin();
	for(; it != subscribers.end(); it++)
	(*it)->OnNotification(this);
}

Notification* CaptureNotificationPublisher::GetNotification()
{
	return notification;
}

void CaptureNotificationPublisher::SetNotification(Notification* n)
{
	notification = n;
}
