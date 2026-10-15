#include "NamingTrace.h"

namespace {
    NamingTraceSink *g_traceSink = nullptr;
}

void setNamingTraceSink(NamingTraceSink *sink) {
    g_traceSink = sink;
}

void traceNaming(const QString &category, const QString &detail) {
    if (!g_traceSink) return;
    NamingTraceEvent event;
    event.category = category;
    event.detail = detail;
    g_traceSink->record(event);
}
