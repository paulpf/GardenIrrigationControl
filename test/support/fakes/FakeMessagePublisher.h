#pragma once

#include <string>
#include <vector>

#include "imessagepublisher.h"

class FakeMessagePublisher : public IMessagePublisher
{
public:
  struct Entry
  {
    std::string topic;
    std::string payload;
  };

  bool connected = true;
  std::vector<Entry> entries;

  bool isConnected() override
  {
    return connected;
  }

  void publish(const char *topic, const char *payload) override
  {
    entries.push_back({topic ? topic : "", payload ? payload : ""});
  }

  int countTopic(const std::string &topic) const
  {
    int count = 0;
    for (const Entry &entry : entries)
    {
      if (entry.topic == topic)
      {
        count++;
      }
    }
    return count;
  }
};
