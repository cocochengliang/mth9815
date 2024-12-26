// tradebookingservice.hpp
// Defines the data types and Service for trade booking.

#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include "soa.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade {
public:
  // Constructor for a Trade
  Trade(const T &_product, std::string _tradeId, double _price, std::string _book, long _quantity, Side _side)
    : product(_product), tradeId(_tradeId), price(_price), book(_book), quantity(_quantity), side(_side) {}

  // Getters
  const T& GetProduct() const { return product; }
  const std::string& GetTradeId() const { return tradeId; }
  double GetPrice() const { return price; }
  const std::string& GetBook() const { return book; }
  long GetQuantity() const { return quantity; }
  Side GetSide() const { return side; }

private:
  T product;
  std::string tradeId;
  double price;
  std::string book;
  long quantity;
  Side side;
};

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade ID.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<std::string, Trade<T>> {
public:
  // Book the trade
  void BookTrade(const Trade<T> &trade) {
    const std::string& tradeId = trade.GetTradeId();
    dataStore[tradeId] = trade;

    // Notify all listeners
    for (auto &listener : listeners) {
      listener->ProcessAdd(trade);
    }
  }

  // Get data for a specific trade ID
  Trade<T>& GetData(std::string tradeId) override {
    if (dataStore.find(tradeId) != dataStore.end()) {
      return dataStore[tradeId];
    } else {
      throw std::runtime_error("Trade not found for ID: " + tradeId);
    }
  }

  // Add a listener to the service
  void AddListener(ServiceListener<Trade<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<Trade<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::map<std::string, Trade<T>> dataStore; // Map to store trades by trade ID
  std::vector<ServiceListener<Trade<T>>*> listeners; // Listeners to notify on updates
};

#endif // TRADE_BOOKING_SERVICE_HPP
