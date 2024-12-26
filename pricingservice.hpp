/**
 * pricingservice.hpp
 * Defines the data types and Service for internal prices.
 *
 * @author Breman Thuraisingham
 */
#ifndef PRICING_SERVICE_HPP
#define PRICING_SERVICE_HPP

#include <string>
#include "soa.hpp"

/**
 * A price object consisting of mid and bid/offer spread.
 * Type T is the product type.
 */
template<typename T>
class Price
{

public:

  // Constructor for a price
  Price(const T &_product, double _mid, double _bidOfferSpread);

  // Get the product
  const T& GetProduct() const;

  // Get the mid price
  double GetMid() const;

  // Get the bid/offer spread around the mid
  double GetBidOfferSpread() const;

private:
  const T& product;
  double mid;
  double bidOfferSpread;
};

/**
 * Pricing Service managing mid prices and bid/offers.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class PricingService : public Service<string, Price<T>>
{

public:

  // Publish a price to the service
  void PublishPrice(const Price<T> &price) {
    string productId = price.GetProduct().GetProductId();
    dataStore[productId] = price;

    // Notify all listeners
    for (auto &listener : listeners) {
      listener->ProcessAdd(price);
    }
  }

  // Get data for a specific product
  Price<T>& GetData(string productId) override {
    if (dataStore.find(productId) != dataStore.end()) {
      return dataStore[productId];
    } else {
      throw runtime_error("Price not found for product ID: " + productId);
    }
  }

  // Add a listener to the service
  void AddListener(ServiceListener<Price<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const vector<ServiceListener<Price<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  map<string, Price<T>> dataStore; // Map to store prices by product ID
  vector<ServiceListener<Price<T>>*> listeners; // Listeners to notify on updates
};

// Implementation of Price class methods
template<typename T>
Price<T>::Price(const T &_product, double _mid, double _bidOfferSpread) :
  product(_product), mid(_mid), bidOfferSpread(_bidOfferSpread) {}

template<typename T>
const T& Price<T>::GetProduct() const {
  return product;
}

template<typename T>
double Price<T>::GetMid() const {
  return mid;
}

template<typename T>
double Price<T>::GetBidOfferSpread() const {
  return bidOfferSpread;
}

#endif // PRICING_SERVICE_HPP
