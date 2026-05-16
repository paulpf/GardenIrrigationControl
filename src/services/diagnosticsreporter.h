#ifndef DIAGNOSTICSREPORTER_H
#define DIAGNOSTICSREPORTER_H

#include "imessagepublisher.h"
#include "trace.h"

class DiagnosticsReporter
{
public:
  explicit DiagnosticsReporter(IMessagePublisher &publisher);

  void setup(const char *clientName);
  void record(TraceLevel level, const String &message, unsigned long uptimeMs);
  void publishReport();
  int count() const;

private:
  static constexpr int MAX_EVENTS = 8;
  static constexpr int MAX_MESSAGE_LENGTH = 96;

  struct DiagnosticEvent
  {
    TraceLevel level = TraceLevel::INFO;
    unsigned long uptimeMs = 0;
    char message[MAX_MESSAGE_LENGTH] = {0};
  };

  IMessagePublisher &_publisher;
  String _topic;
  DiagnosticEvent _events[MAX_EVENTS];
  int _nextIndex = 0;
  int _count = 0;

  static const char *levelToString(TraceLevel level);
  static String escapeJson(const char *value);
  int eventIndexAt(int orderedIndex) const;
  String buildReportJson() const;
};

#endif // DIAGNOSTICSREPORTER_H
