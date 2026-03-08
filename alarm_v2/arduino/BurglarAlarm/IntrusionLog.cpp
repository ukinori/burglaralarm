#include "IntrusionLog.h"
#include <string.h>
#include <stdio.h>

IntrusionLog::IntrusionLog() : head(0), count(0) {
    memset(entries, 0, sizeof(entries));
}

void IntrusionLog::logEvent(const char* event) {
    entries[head].timestampMs = millis();
    strncpy(entries[head].event, event, LOG_EVENT_LEN - 1);
    entries[head].event[LOG_EVENT_LEN - 1] = '\0';

    // Echo to Serial → Python writes to CSV
    char buf[LOG_EVENT_LEN + 20];
    snprintf(buf, sizeof(buf), "LOG:%lu:%s", entries[head].timestampMs, event);
    Serial.println(buf);

    head = (head + 1) % LOG_MAX_ENTRIES;   // ring-buffer wrap
    if (count < LOG_MAX_ENTRIES) count++;
}

void IntrusionLog::printLog() const {
    char buf[LOG_EVENT_LEN + 20];
    for (int i = 0; i < count; i++) {
        int idx = (head - count + i + LOG_MAX_ENTRIES) % LOG_MAX_ENTRIES;
        snprintf(buf, sizeof(buf), "LOG:%lu:%s",
                 entries[idx].timestampMs, entries[idx].event);
        Serial.println(buf);
    }
}

void IntrusionLog::clearLog() {
    memset(entries, 0, sizeof(entries));
    head  = 0;
    count = 0;
}

int IntrusionLog::getCount() const { return count; }
