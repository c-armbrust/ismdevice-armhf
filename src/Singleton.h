#ifndef SINGLETON_H
#define SINGLETON_H

template<class T>
class Singleton
{
public:
	static T& Instance();

protected:
	Singleton(); // ctor hidden
	~Singleton(); // dtor hidden
	Singleton(Singleton const&); // cp ctor hidden
	Singleton& operator=(Singleton const&); // assign op hidden
};



template<class T>
T& Singleton<T>::Instance()
{
	static T _instance;
	return _instance;
}

#endif
