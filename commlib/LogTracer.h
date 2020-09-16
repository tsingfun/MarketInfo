#ifndef LOGTRACER_H
#define LOGTRACER_H

#include "Logger.h"

#ifndef	NO_DEBUG_LOG
#define LOG_TRACER() CLogTracer logTracer(__FUNCTION__)
#else // NO_DEBUG_LOG
#define LOG_TRACER() 
#endif // NO_DEBUG_LOG

class CLogTracer
{
private:

    const char* const functionName;

public:

    CLogTracer(const char* functionName) : functionName(functionName) {
        LOG_DEBUG(_T("--> %s"),  functionName);
    }
    
    ~CLogTracer() {
        if (std::uncaught_exception()) {
			LOG_DEBUG(_T("<-- %s (uncaught exception)"), functionName);
        } else {
			LOG_DEBUG(_T("<-- %s"), functionName);
        }
    }    
};

#endif //LOGTRACER_H