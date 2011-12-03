#ifndef COMMONCLASS_H_
#define COMMONCLASS_H_
#include "../interface/logger.h"

class CommonClass {
	mutable Logger* logger;
public:
	enum Exception {
		LOGGER_NOT_SET
	};
	CommonClass();
	void setLogger(Logger& logger);
	Logger const& getLogger() const;
	Logger& getLogger();
	Logger* getLoggerPtr();
	bool hasLogger() const;
protected:
	void log(std::string const& message, Logger::Priority prio) const;
};

#endif
