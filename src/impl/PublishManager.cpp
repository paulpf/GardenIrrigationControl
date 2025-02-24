#include "../_interfaces/PublishManager.h"
#include "../_interfaces/IPublisher.h"
#include "../_interfaces/Data.h"

PublishManager::PublishManager(/* args */)
{
}

PublishManager::~PublishManager()
{
}

void PublishManager::registerPublishers(IPublisher *publisher)
{
    publishers.push_back(publisher);
}

void PublishManager::readData(Data &data)
{
    for (IPublisher* publisher : publishers)
    {
        publisher->readData(data);
    }
}

void PublishManager::publish(Data &data)
{
    for (IPublisher* publisher : publishers)
    {
        publisher->publish(data);
    }
}