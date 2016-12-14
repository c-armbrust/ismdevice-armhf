#ifndef PUB_SUB_H
#define PUB_SUB_H

#include <list>
#include <string>

class Subscriber;

class Notification
{
public:
	virtual ~Notification() = 0;
};

class Publisher
{
public:
	virtual void Attach(Subscriber*) = 0;
	virtual void Detach(Subscriber*) = 0;
	virtual void Notify() = 0;

	// get/set methods for the Notification* member
	virtual Notification* GetNotification() = 0;
	virtual void SetNotification(Notification*) = 0;
protected:
	std::list<Subscriber*> subscribers;
	Notification* notification;
};

class Subscriber
{
public:
	virtual void OnNotification(Publisher*) = 0;
};



// Concrete notification publishers and concrete notification types
//
class CaptureNotification : public Notification// notification type to notify capture upload to blob storage
{
public:
	CaptureNotification(std::string uri);
	std::string CurrentCaptureUri;
};

class CaptureNotificationPublisher : public Publisher // publishes uploaded to blob storage notifications
{
public:
	void Attach(Subscriber*);
	void Detach(Subscriber*);
	void Notify();
	Notification* GetNotification();
	void SetNotification(Notification*);
};

#endif
