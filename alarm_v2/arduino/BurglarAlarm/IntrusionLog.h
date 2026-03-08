#ifndef INTRUSION_LOG_H
#define INTRUSION_LOG_H
#include <Arduino.h>

#define LOG_MAX_ENTRIES  10
#define LOG_EVENT_LEN    32

/**
 * IntrusionLog
 * Ring-buffer of intrusion events stored in SRAM.
 * Each entry is echoed over Serial so Python can persist it to CSV.
 * Buffer size is kept small (10 × 32 bytes = 320 bytes) to protect SRAM.
 */
struct LogEntry {
    unsigned long timestampMs;
    char          event[LOG_EVENT_LEN];
};

class IntrusionLog {
private:
    LogEntry entries[LOG_MAX_ENTRIES];
    int      head;
    int      count;

public:
    IntrusionLog();

    void logEvent(const char* event);   // stores + echoes LOG:<ms>:<event>
    void printLog() const;              // re-sends all entries over Serial
    void clearLog();
    int  getCount() const;
};

#endif
