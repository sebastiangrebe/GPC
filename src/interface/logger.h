#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED

#include <string>

class Logger {
public:
	enum Priority {
		EVENT,
		IMPORTANT_EVENT,
		INFO,
		WARNING,
		ERROR
	};
	virtual void log(std::string const& str, Priority prio) = 0;
};

#endif
