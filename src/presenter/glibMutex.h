#ifndef GLIBMUTEX_H_
#define GLIBMUTEX_H_
#include "../interface/mutex.h"
#include <glibmm/thread.h>
#include "../presenter/commonClass.h"

class GlibMutex : public Mutex, public CommonClass {
protected:
	Glib::Mutex mutex;
public:
	void lock();
	bool trylock();
	void unlock();
};

#endif
