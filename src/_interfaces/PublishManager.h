#ifndef PUBLISH_MANAGER_H
#define PUBLISH_MANAGER_H

#include <vector>
#include "IPublisher.h"
#include "Data.h"

class PublishManager
{
public:
  PublishManager(/* args */);
  ~PublishManager();
  void registerPublishers(IPublisher *publisher);
  void publish(Data &data);
  void readData(Data &data);

  private:
  std::vector<IPublisher*> publishers;
};

#endif // PUBLISH_MANAGER_H