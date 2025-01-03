/**
 * riskservice.hpp
 * Defines the data types and Service for fixed income risk.
 *
 * @author Breman Thuraisingham
 */
#ifndef RISK_SERVICE_HPP
#define RISK_SERVICE_HPP

#include "soa.hpp"
#include "positionservice.hpp"
#include <unordered_map>
#include <iostream>
#include <stdexcept>

/**
 * PV01 risk.
 * Type T is the product type.
 */
template<typename T>
class PV01
{

public:

  // ctor for a PV01 value
  PV01(const T &_product, double _pv01, long _quantity);

  // Get the product on this PV01 value
  const T& GetProduct() const;

  // Get the PV01 value
  double GetPV01() const;

  // Get the quantity that this risk value is associated with
  long GetQuantity() const;

  // Update the quantity
  void UpdateQuantity(long newQuantity);

private:
  T product;
  double pv01;
  long quantity;

};

/**
 * A bucket sector to bucket a group of securities.
 * We can then aggregate bucketed risk to this bucket.
 * Type T is the product type.
 */
template<typename T>
class BucketedSector
{

public:

  // ctor for a bucket sector
  BucketedSector(const std::vector<T> &_products, std::string _name);

  // Get the products associated with this bucket
  const std::vector<T>& GetProducts() const;

  // Get the name of the bucket
  const std::string& GetName() const;

private:
  std::vector<T> products;
  std::string name;

};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<std::string, PV01<T>>
{

public:

  // Add a position that the service will risk
  void AddPosition(Position<T> &position) override {
    std::string productId = position.GetProduct().GetProductId();
    long aggregatePosition = position.GetAggregatePosition();

    if (data.find(productId) == data.end()) {
      data[productId] = PV01<T>(position.GetProduct(), 0.01, aggregatePosition);
    } else {
      data[productId].UpdateQuantity(aggregatePosition);
    }

    PV01<T> &pv01 = data[productId];

    for (auto &listener : listeners) {
      listener->ProcessUpdate(pv01);
    }
  }

  // Get the bucketed risk for the bucket sector
  PV01<BucketedSector<T>> GetBucketedRisk(const BucketedSector<T> &sector) const {
    double totalPv01 = 0.0;
    long totalQuantity = 0;

    for (const auto &product : sector.GetProducts()) {
      std::string productId = product.GetProductId();
      if (data.find(productId) == data.end()) {
        throw std::runtime_error("Product not found in RiskService: " + productId);
      }
      const PV01<T> &pv01 = data.at(productId);
      totalPv01 += pv01.GetPV01() * pv01.GetQuantity();
      totalQuantity += pv01.GetQuantity();
    }

    return PV01<BucketedSector<T>>(sector, totalPv01 / totalQuantity, totalQuantity);
  }

  // Get data by product ID
  PV01<T>& GetData(const std::string& productId) override {
    if (data.find(productId) != data.end()) {
      return data[productId];
    } else {
      throw std::runtime_error("PV01 not found for product ID: " + productId);
    }
  }

  // Add a listener to the service
  void AddListener(ServiceListener<PV01<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<PV01<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::unordered_map<std::string, PV01<T>> data; // Map to store PV01 values by product ID
  std::vector<ServiceListener<PV01<T>>*> listeners; // Listeners to notify on updates
};

// Implementation of PV01 methods
template<typename T>
PV01<T>::PV01(const T &_product, double _pv01, long _quantity) : product(_product), pv01(_pv01), quantity(_quantity) {}

template<typename T>
const T& PV01<T>::GetProduct() const {
  return product;
}

template<typename T>
double PV01<T>::GetPV01() const {
  return pv01;
}

template<typename T>
long PV01<T>::GetQuantity() const {
  return quantity;
}

template<typename T>
void PV01<T>::UpdateQuantity(long newQuantity) {
  quantity = newQuantity;
}

// Implementation of BucketedSector methods
template<typename T>
BucketedSector<T>::BucketedSector(const std::vector<T>& _products, std::string _name) : products(_products), name(_name) {}

template<typename T>
const std::vector<T>& BucketedSector<T>::GetProducts() const {
  return products;
}

template<typename T>
const std::string& BucketedSector<T>::GetName() const {
  return name;
}

#endif
