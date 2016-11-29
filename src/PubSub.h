#ifndef PUB_SUB_H
#define PUB_SUB_H

#include <list>

class Subscriber;

class Publisher
{
public:
	virtual void Attach(Subscriber*) = 0;
	virtual void Detach(Subscriber*) = 0;
	virtual void Notify() = 0;		
protected:
	std::list<Subscriber*>* subscribers;
};



class Subscriber
{
public:
	virtual void OnNotification(void*) = 0;
};

#endif
