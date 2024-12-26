/**
 * marketdataservice.hpp
 * Defines the data types and Service for order book market data.
 *
 * @author Breman Thuraisingham
 */
#ifndef MARKET_DATA_SERVICE_HPP
#define MARKET_DATA_SERVICE_HPP

#include <string>
#include <vector>
#include "soa.hpp"

using namespace std;

// Side for market data
enum PricingSide { BID, OFFER };

/**
 * A market data order with price, quantity, and side.
 */
class Order
{

public:

  // Constructor for an order
  Order(double _price, long _quantity, PricingSide _side) : price(_price), quantity(_quantity), side(_side) {}

  // Get the price on the order
  double GetPrice() const { return price; }

  // Get the quantity on the order
  long GetQuantity() const { return quantity; }

  // Get the side on the order
  PricingSide GetSide() const { return side; }

private:
  double price;
  long quantity;
  PricingSide side;
};

/**
 * Class representing a bid and offer order.
 */
class BidOffer
{

public:

  // Constructor for bid/offer
  BidOffer(const Order &_bidOrder, const Order &_offerOrder) : bidOrder(_bidOrder), offerOrder(_offerOrder) {}

  // Get the bid order
  const Order& GetBidOrder() const { return bidOrder; }

  // Get the offer order
  const Order& GetOfferOrder() const { return offerOrder; }

private:
  Order bidOrder;
  Order offerOrder;
};

/**
 * Order book with a bid and offer stack.
 * Type T is the product type.
 */
template<typename T>
class OrderBook
{

public:

  // Constructor for the order book
  OrderBook(const T &_product, const vector<Order> &_bidStack, const vector<Order> &_offerStack) : product(_product), bidStack(_bidStack), offerStack(_offerStack) {}

  // Get the product
  const T& GetProduct() const { return product; }

  // Get the bid stack
  const vector<Order>& GetBidStack() const { return bidStack; }

  // Get the offer stack
  const vector<Order>& GetOfferStack() const { return offerStack; }

private:
  T product;
  vector<Order> bidStack;
  vector<Order> offerStack;
};

/**
 * Market Data Service which distributes market data.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class MarketDataService : public Service<string, OrderBook<T>>
{

public:

  // Get the best bid/offer order
  const BidOffer& GetBestBidOffer(const string &productId) {
    auto& orderBook = dataStore[productId];
    const Order& bestBid = orderBook.GetBidStack().front();
    const Order& bestOffer = orderBook.GetOfferStack().front();
    bestBidOffer = BidOffer(bestBid, bestOffer);
    return bestBidOffer;
  }

  // Aggregate the order book
  const OrderBook<T>& AggregateDepth(const string &productId) {
    return dataStore[productId];
  }

  // Add a listener to the service
  void AddListener(ServiceListener<OrderBook<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const vector<ServiceListener<OrderBook<T>>*>& GetListeners() const override {
    return listeners;
  }

  // OnMessage callback for receiving market data updates
  void OnMessage(OrderBook<T>& data) override {
    string productId = data.GetProduct().GetProductId();
    dataStore[productId] = data;

    // Notify all listeners
    for (auto& listener : listeners) {
      listener->ProcessAdd(data);
    }
  }

  // Get data by product ID
  OrderBook<T>& GetData(string productId) override {
    if (dataStore.find(productId) != dataStore.end()) {
      return dataStore[productId];
    } else {
      throw runtime_error("OrderBook not found for product ID: " + productId);
    }
  }

private:
  map<string, OrderBook<T>> dataStore; // Map to store order books by product ID
  vector<ServiceListener<OrderBook<T>>*> listeners; // Listeners to notify on updates
  BidOffer bestBidOffer;
};

#endif // MARKET_DATA_SERVICE_HPP
