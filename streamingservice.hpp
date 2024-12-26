// streamingservice.hpp
// Defines the data types and Service for price streams.

#ifndef STREAMING_SERVICE_HPP
#define STREAMING_SERVICE_HPP

#include "soa.hpp"
#include "marketdataservice.hpp"
#include <map>
#include <vector>
#include <string>
#include <stdexcept>

/**
 * A price stream order with price and quantity (visible and hidden).
 */
class PriceStreamOrder {
public:
  // Constructor for a PriceStreamOrder
  PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side);

  // Getters
  PricingSide GetSide() const;
  double GetPrice() const;
  long GetVisibleQuantity() const;
  long GetHiddenQuantity() const;

private:
  double price;
  long visibleQuantity;
  long hiddenQuantity;
  PricingSide side;
};

/**
 * Price Stream with a two-way market.
 * Type T is the product type.
 */
template<typename T>
class PriceStream {
public:
  // Constructor
  PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder);

  // Getters
  const T& GetProduct() const;
  const PriceStreamOrder& GetBidOrder() const;
  const PriceStreamOrder& GetOfferOrder() const;

private:
  T product;
  PriceStreamOrder bidOrder;
  PriceStreamOrder offerOrder;
};

/**
 * Streaming service to publish two-way prices.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class StreamingService : public Service<std::string, PriceStream<T>> {
public:
  // Publish two-way prices
  void PublishPrice(const PriceStream<T>& priceStream) {
    const std::string& productId = priceStream.GetProduct().GetProductId();
    dataStore[productId] = priceStream;

    // Notify all listeners
    for (auto &listener : listeners) {
      listener->ProcessAdd(priceStream);
    }
  }

  // Get data for a specific product
  PriceStream<T>& GetData(std::string productId) override {
    if (dataStore.find(productId) != dataStore.end()) {
      return dataStore[productId];
    } else {
      throw std::runtime_error("PriceStream not found for product ID: " + productId);
    }
  }

  // Add a listener to the service
  void AddListener(ServiceListener<PriceStream<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<PriceStream<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::map<std::string, PriceStream<T>> dataStore; // Map to store price streams by product ID
  std::vector<ServiceListener<PriceStream<T>>*> listeners; // Listeners to notify on updates
};

// Implementation of PriceStreamOrder
PriceStreamOrder::PriceStreamOrder(double _price, long _visibleQuantity, long _hiddenQuantity, PricingSide _side)
  : price(_price), visibleQuantity(_visibleQuantity), hiddenQuantity(_hiddenQuantity), side(_side) {}

PricingSide PriceStreamOrder::GetSide() const { return side; }

double PriceStreamOrder::GetPrice() const { return price; }

long PriceStreamOrder::GetVisibleQuantity() const { return visibleQuantity; }

long PriceStreamOrder::GetHiddenQuantity() const { return hiddenQuantity; }

// Implementation of PriceStream
template<typename T>
PriceStream<T>::PriceStream(const T &_product, const PriceStreamOrder &_bidOrder, const PriceStreamOrder &_offerOrder)
  : product(_product), bidOrder(_bidOrder), offerOrder(_offerOrder) {}

template<typename T>
const T& PriceStream<T>::GetProduct() const { return product; }

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetBidOrder() const { return bidOrder; }

template<typename T>
const PriceStreamOrder& PriceStream<T>::GetOfferOrder() const { return offerOrder; }

#endif // STREAMING_SERVICE_HPP
