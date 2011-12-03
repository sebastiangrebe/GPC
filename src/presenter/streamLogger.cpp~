#include "streamLogger.h"

StreamLogger::StreamLogger(std::ostream& stream) : stream(&stream) {}

void StreamLogger::log(std::string const& message, Logger::Priority prio) {
	if (prio == Logger::IMPORTANT_EVENT) {
		*this->stream << " *** ";
	} else if (prio == Logger::EVENT) {
		*this->stream << "   * ";
	} else {
		*this->stream << "     ";
	}

	if (prio == Logger::INFO) {
		*this->stream << "[";
	}
	*this->stream << message;
	if (prio == Logger::INFO) {
		*this->stream << "]";
	}

	*this->stream << std::endl;
}
