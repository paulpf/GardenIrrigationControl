#ifndef PUBLISH_MANAGER_H
#define PUBLISH_MANAGER_H

#include <vector>
#include "IPublisher.h"
#include "Data.h"

class PublishManager
{
    private:
      std::vector<IPublisher*> publishers;
    public:
      PublishManager(/* args */);
      ~PublishManager();
      void registerPublishers(IPublisher *publisher);
      void publish(Data &data);
};

#endif // PUBLISH_MANAGER_H