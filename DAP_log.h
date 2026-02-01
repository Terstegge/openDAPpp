
// This is a simple logging class to be used within ...
// The user interface are the two macros defined below:
//
//   DAP_LOG(...)  to print a logging message on stdout, and
//   DAP_LOG_LEVEL(l) to set the logging level (see below).
//
#ifndef DAP_LOG_H
#define DAP_LOG_H

#define MAX_LINE_LENGTH 200

#ifdef NDEBUG
#define DAP_LOG(...)
#define DAP_LOG_LEVEL(level)
#else
#define DAP_LOG(...)         DAP_log::inst.print(__FILE_NAME__, __LINE__, __VA_ARGS__)
#define DAP_LOG_LEVEL(level) DAP_log::inst.setLevel(level)
#endif

class DAP_log {
public:
    // The log levels
    enum log_level { LOG_OFF=0, LOG_ERROR=1, LOG_WARNING=2, LOG_INFO=3, LOG_DEBUG=4 };

    // The logger is a singleton
    static DAP_log inst;
    // Print a single log line. The format specifier
    // is similar to the standard printf function.
    // only %s (string), %d (integer) %x (hex integer)
    // and %b (boolean) are allowed, without a width
    // specification. A "\n" will be added to the output
    // string automatically.
    void print(const char *file, int line, log_level l, const char *fmt, ...);
    // Set the log level
    inline void setLevel(log_level l) { _level = l; }

private:
    // No public access to CTOR
    DAP_log() : _level {log_level::LOG_OFF} {}
    // The current log level
    log_level _level;
    char _buffer[MAX_LINE_LENGTH] {0};
    const char * _level_str[5] = {"[OFF] ", "[ERR] ", "[WAR] ", "[INF] ", "[DBG] "};
};

#endif // DAP_LOG_H
