#include "commonClass.h"

CommonClass::CommonClass() {
	this->logger = NULL;
}

void CommonClass::setLogger(Logger& logger) {
	this->logger = &logger;
}
Logger const& CommonClass::getLogger() const {
	if (this->logger == NULL) {
		throw LOGGER_NOT_SET;
	}
	return *this->logger;
}
Logger& CommonClass::getLogger() {
	if (this->logger == NULL) {
		throw LOGGER_NOT_SET;
	}
	return *this->logger;
}
Logger* CommonClass::getLoggerPtr() {
	return this->logger;
}
bool CommonClass::hasLogger() const {
	return this->logger != NULL;
}
void CommonClass::log(std::string const& message, Logger::Priority prio) const {
	if (this->logger) {
		this->logger->log(message, prio);
	}
}
