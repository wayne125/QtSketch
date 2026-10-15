#pragma once
#include <QString>

struct NamingTraceEvent {
    QString category;  // e.g. "orientation", "peripheral_candidate", "tie_retained"
    QString detail;     // free-form human-readable detail string
};

struct NamingTraceSink {
    virtual ~NamingTraceSink() = default;
    virtual void record(const NamingTraceEvent &event) = 0;
};

// Global sink pointer -- null by default (zero overhead, no behavior change,
// normal naming runs never touch this). Set by a driver/test that wants tracing.
void setNamingTraceSink(NamingTraceSink *sink);
void traceNaming(const QString &category, const QString &detail);
