#ifndef STREAM_LOGGER_H_
#define STREAM_LOGGER_H_
#include "../interface/logger.h"
#include <ostream>

class StreamLogger : public Logger {
	std::ostream* stream;
public:
	StreamLogger(std::ostream& stream);
	void log(std::string const& message, Logger::Priority prio);
};

#endif
