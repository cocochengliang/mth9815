// tradebookingservice.hpp
// Defines the data types and Service for trade booking.

#ifndef TRADE_BOOKING_SERVICE_HPP
#define TRADE_BOOKING_SERVICE_HPP

#include "soa.hpp"

// Trade sides
enum Side { BUY, SELL };

/**
 * Trade object with a price, side, and quantity on a particular book.
 * Type T is the product type.
 */
template<typename T>
class Trade {
public:
  // Constructor for a trade
  Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side);

  // Getters
  const T& GetProduct() const;
  const string& GetTradeId() const;
  double GetPrice() const;
  const string& GetBook() const;
  long GetQuantity() const;
  Side GetSide() const;

private:
  T product;
  string tradeId;
  double price;
  string book;
  long quantity;
  Side side;
};

/**
 * Trade Booking Service to book trades to a particular book.
 * Keyed on trade ID.
 * Type T is the product type.
 */
template<typename T>
class TradeBookingService : public Service<string, Trade<T>> {
public:
  // Book the trade
  virtual void BookTrade(const Trade<T> &trade) = 0;
};

// Implementation of Trade
template<typename T>
Trade<T>::Trade(const T &_product, string _tradeId, double _price, string _book, long _quantity, Side _side)
  : product(_product), tradeId(_tradeId), price(_price), book(_book), quantity(_quantity), side(_side) {}

template<typename T>
const T& Trade<T>::GetProduct() const { return product; }

template<typename T>
const string& Trade<T>::GetTradeId() const { return tradeId; }

template<typename T>
double Trade<T>::GetPrice() const { return price; }

template<typename T>
const string& Trade<T>::GetBook() const { return book; }

template<typename T>
long Trade<T>::GetQuantity() const { return quantity; }

template<typename T>
Side Trade<T>::GetSide() const { return side; }

#endif // TRADE_BOOKING_SERVICE_HPP
