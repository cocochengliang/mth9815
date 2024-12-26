/**
 * historicaldataservice.hpp
 * Defines the data types and Service for historical data.
 *
 * @author Breman Thuraisingham
 */
#ifndef HISTORICAL_DATA_SERVICE_HPP
#define HISTORICAL_DATA_SERVICE_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include "soa.hpp"

/**
 * Service for processing and persisting historical data to a persistent store.
 * Keyed on some persistent key.
 * Type T is the data type to persist.
 */
template<typename T>
class HistoricalDataService : public Service<std::string, T>
{

public:

  // Constructor
  HistoricalDataService() {}

  // Persist data to a store
  void PersistData(std::string persistKey, const T& data) {
    // Store the data
    dataStore[persistKey] = data;

    // Notify all listeners
    for (auto& listener : listeners) {
      listener->ProcessAdd(dataStore[persistKey]);
    }

    // Log persistence
    std::cout << "Persisted data for key: " << persistKey << std::endl;
  }

  // Get data by key
  T& GetData(std::string key) override {
    if (dataStore.find(key) != dataStore.end()) {
      return dataStore[key];
    } else {
      throw std::runtime_error("Data not found for key: " + key);
    }
  }

  // OnMessage callback (not typically used for historical data but can be overridden)
  void OnMessage(T& data) override {
    // Example: Could directly persist incoming messages
    std::string key = "DefaultKey"; // Replace with real key logic
    PersistData(key, data);
  }

  // Add a listener to the service
  void AddListener(ServiceListener<T>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<T>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::map<std::string, T> dataStore; // Map to store data by key
  std::vector<ServiceListener<T>*> listeners; // Listeners to notify on persistence
};

#endif // HISTORICAL_DATA_SERVICE_HPP
