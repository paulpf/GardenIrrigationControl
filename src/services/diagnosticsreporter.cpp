#include "diagnosticsreporter.h"

#include <cstring>

DiagnosticsReporter::DiagnosticsReporter(IMessagePublisher &publisher)
    : _publisher(publisher)
{
}

void DiagnosticsReporter::setup(const char *clientName)
{
  _topic = String(clientName) + "/system/diagnostics";
}

void DiagnosticsReporter::record(TraceLevel level, const String &message,
                                 unsigned long uptimeMs)
{
  DiagnosticEvent &event = _events[_nextIndex];
  event.level = level;
  event.uptimeMs = uptimeMs;
  strncpy(event.message, message.c_str(), MAX_MESSAGE_LENGTH - 1);
  event.message[MAX_MESSAGE_LENGTH - 1] = '\0';

  _nextIndex = (_nextIndex + 1) % MAX_EVENTS;
  if (_count < MAX_EVENTS)
  {
    _count++;
  }
}

void DiagnosticsReporter::publishReport()
{
  if (!_publisher.isConnected() || _topic.length() == 0)
  {
    return;
  }

  String report = buildReportJson();
  _publisher.publish(_topic.c_str(), report.c_str());
}

int DiagnosticsReporter::count() const
{
  return _count;
}

const char *DiagnosticsReporter::levelToString(TraceLevel level)
{
  switch (level)
  {
  case TraceLevel::WARNING:
    return "warning";
  case TraceLevel::ERROR:
    return "error";
  case TraceLevel::DEBUG:
    return "debug";
  case TraceLevel::TRACE:
    return "trace";
  case TraceLevel::INFO:
    return "info";
  default:
    return "log";
  }
}

String DiagnosticsReporter::escapeJson(const char *value)
{
  String escaped;
  if (value == nullptr)
  {
    return escaped;
  }

  for (const char *cursor = value; *cursor != '\0'; cursor++)
  {
    switch (*cursor)
    {
    case '"':
      escaped += "\\\"";
      break;
    case '\\':
      escaped += "\\\\";
      break;
    case '\n':
      escaped += "\\n";
      break;
    case '\r':
      escaped += "\\r";
      break;
    case '\t':
      escaped += "\\t";
      break;
    default:
      char character[2] = {*cursor, '\0'};
      escaped += character;
      break;
    }
  }

  return escaped;
}

int DiagnosticsReporter::eventIndexAt(int orderedIndex) const
{
  if (orderedIndex < 0 || orderedIndex >= _count)
  {
    return -1;
  }

  const int oldestIndex =
      _count == MAX_EVENTS ? _nextIndex : (_nextIndex - _count + MAX_EVENTS) %
                                             MAX_EVENTS;
  return (oldestIndex + orderedIndex) % MAX_EVENTS;
}

String DiagnosticsReporter::buildReportJson() const
{
  String report = "{\"count\":";
  report += String(_count);
  report += ",\"events\":[";

  for (int i = 0; i < _count; i++)
  {
    const int eventIndex = eventIndexAt(i);
    if (eventIndex < 0)
    {
      continue;
    }

    if (i > 0)
    {
      report += ",";
    }

    const DiagnosticEvent &event = _events[eventIndex];
    report += "{\"level\":\"";
    report += levelToString(event.level);
    report += "\",\"uptimeMs\":";
    report += String(event.uptimeMs);
    report += ",\"message\":\"";
    report += escapeJson(event.message);
    report += "\"}";
  }

  report += "]}";
  return report;
}
