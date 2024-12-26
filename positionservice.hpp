/**
 * positionservice.hpp
 * Defines the data types and Service for positions.
 *
 * @author Breman Thuraisingham
 */
#ifndef POSITION_SERVICE_HPP
#define POSITION_SERVICE_HPP

#include <string>
#include <map>
#include "soa.hpp"
#include "tradebookingservice.hpp"

using namespace std;

/**
 * Position class in a particular book.
 * Type T is the product type.
 */
template<typename T>
class Position
{

public:

  // Constructor for a position
  Position(const T &_product);

  // Get the product
  const T& GetProduct() const;

  // Get the position quantity for a specific book
  long GetPosition(string &book);

  // Get the aggregate position across all books
  long GetAggregatePosition();

  // Update the position for a specific book
  void UpdatePosition(const string &book, long quantity);

private:
  T product;
  map<string, long> positions;
};

/**
 * Position Service to manage positions across multiple books and securities.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PositionService : public Service<string, Position<T>>
{

public:

  // Add a trade to the service
  void AddTrade(const Trade<T> &trade) override {
    string productId = trade.GetProduct().GetProductId();

    // Create a new position if it doesn't exist
    if (dataStore.find(productId) == dataStore.end()) {
      dataStore[productId] = Position<T>(trade.GetProduct());
    }

    // Update the position for the product
    Position<T>& position = dataStore[productId];
    position.UpdatePosition(trade.GetBook(), trade.GetSide() == BUY ? trade.GetQuantity() : -trade.GetQuantity());

    // Notify listeners about the updated position
    for (auto& listener : listeners) {
      listener->ProcessUpdate(position);
    }
  }

  // Get data for a specific product
  Position<T>& GetData(string productId) override {
    if (dataStore.find(productId) != dataStore.end()) {
      return dataStore[productId];
    } else {
      throw runtime_error("Position not found for product ID: " + productId);
    }
  }

  // Add a listener to the service
  void AddListener(ServiceListener<Position<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const vector<ServiceListener<Position<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  map<string, Position<T>> dataStore; // Map to store positions by product ID
  vector<ServiceListener<Position<T>>*> listeners; // Listeners to notify on updates
};

// Implementation of Position class methods
template<typename T>
Position<T>::Position(const T &_product) : product(_product) {}

template<typename T>
const T& Position<T>::GetProduct() const {
  return product;
}

template<typename T>
long Position<T>::GetPosition(string &book) {
  return positions[book];
}

template<typename T>
long Position<T>::GetAggregatePosition() {
  long aggregate = 0;
  for (const auto &entry : positions) {
    aggregate += entry.second;
  }
  return aggregate;
}

template<typename T>
void Position<T>::UpdatePosition(const string &book, long quantity) {
  positions[book] += quantity;
}

#endif // POSITION_SERVICE_HPP
