/**
 * executionservice.hpp
 * Defines the data types and Service for executions.
 *
 * @author Breman Thuraisingham
 */
#ifndef EXECUTION_SERVICE_HPP
#define EXECUTION_SERVICE_HPP

#include <string>
#include <map>
#include <vector>
#include "soa.hpp"
#include "marketdataservice.hpp"

enum OrderType { FOK, IOC, MARKET, LIMIT, STOP };

enum Market { BROKERTEC, ESPEED, CME };

/**
 * An execution order that can be placed on an exchange.
 * Type T is the product type.
 */
template<typename T>
class ExecutionOrder
{

public:
  // Constructor for an order
  ExecutionOrder(const T &_product, PricingSide _side, std::string _orderId, OrderType _orderType, double _price, double _visibleQuantity, double _hiddenQuantity, std::string _parentOrderId, bool _isChildOrder) :
    product(_product), side(_side), orderId(_orderId), orderType(_orderType), price(_price), visibleQuantity(_visibleQuantity), hiddenQuantity(_hiddenQuantity), parentOrderId(_parentOrderId), isChildOrder(_isChildOrder) {}

  // Get the product
  const T& GetProduct() const { return product; }

  // Get the side
  PricingSide GetSide() const { return side; }

  // Get the order ID
  const std::string& GetOrderId() const { return orderId; }

  // Get the order type
  OrderType GetOrderType() const { return orderType; }

  // Get the price
  double GetPrice() const { return price; }

  // Get the visible quantity
  double GetVisibleQuantity() const { return visibleQuantity; }

  // Get the hidden quantity
  double GetHiddenQuantity() const { return hiddenQuantity; }

  // Get the parent order ID
  const std::string& GetParentOrderId() const { return parentOrderId; }

  // Check if this is a child order
  bool IsChildOrder() const { return isChildOrder; }

private:
  T product;
  PricingSide side;
  std::string orderId;
  OrderType orderType;
  double price;
  double visibleQuantity;
  double hiddenQuantity;
  std::string parentOrderId;
  bool isChildOrder;
};

/**
 * Service for executing orders on an exchange.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class ExecutionService : public Service<std::string, ExecutionOrder<T>>
{
public:
  // Execute an order on a market
  void ExecuteOrder(const ExecutionOrder<T>& order, Market market) {
    data[order.GetOrderId()] = order;

    // Notify all listeners about the new execution order
    for (auto& listener : listeners) {
      listener->ProcessAdd(data[order.GetOrderId()]);
    }

    // Log the execution order
    std::cout << "Executed order: " << order.GetOrderId()
              << " on market: " << MarketToString(market)
              << " at price: " << order.GetPrice()
              << " with quantity: " << order.GetVisibleQuantity() << std::endl;
  }

  // Get data on an order by ID
  ExecutionOrder<T>& GetData(std::string key) override {
    return data.at(key);
  }

  // Handle incoming messages (data updates)
  void OnMessage(ExecutionOrder<T>& data) override {
    // Add or update the execution order
    ExecuteOrder(data, BROKERTEC); // Default market
  }

  // Add a listener to the service
  void AddListener(ServiceListener<ExecutionOrder<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<ExecutionOrder<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::map<std::string, ExecutionOrder<T>> data; // Storage for execution orders
  std::vector<ServiceListener<ExecutionOrder<T>>*> listeners; // List of listeners

  // Utility function to convert Market enum to string
  std::string MarketToString(Market market) const {
    switch (market) {
      case BROKERTEC: return "BROKERTEC";
      case ESPEED: return "ESPEED";
      case CME: return "CME";
      default: return "UNKNOWN";
    }
  }
};

#endif // EXECUTION_SERVICE_HPP
