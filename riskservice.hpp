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
  BucketedSector(const vector<T> &_products, string _name);

  // Get the products associated with this bucket
  const vector<T>& GetProducts() const;

  // Get the name of the bucket
  const string& GetName() const;

private:
  vector<T> products;
  string name;

};

/**
 * Risk Service to vend out risk for a particular security and across a risk bucketed sector.
 * Keyed on product identifier.
 * Type T is the product type.
 */
template<typename T>
class RiskService : public Service<string, PV01<T>>
{

public:

  // Add a position that the service will risk
  void AddPosition(Position<T> &position) override {
    string productId = position.GetProduct().GetProductId();
    long aggregatePosition = position.GetAggregatePosition();

    if (this->data.find(productId) == this->data.end()) {
      this->data[productId] = PV01<T>(position.GetProduct(), 0.01, aggregatePosition);
    }

    PV01<T> &pv01 = this->data[productId];
    pv01 = PV01<T>(position.GetProduct(), pv01.GetPV01(), aggregatePosition);

    for (auto &listener : this->listeners) {
      listener->ProcessUpdate(pv01);
    }
  }

  // Get the bucketed risk for the bucket sector
  const PV01<BucketedSector<T>>& GetBucketedRisk(const BucketedSector<T> &sector) const override {
    double totalPv01 = 0.0;
    long totalQuantity = 0;

    for (const auto &product : sector.GetProducts()) {
      string productId = product.GetProductId();
      const PV01<T> &pv01 = this->data.at(productId);
      totalPv01 += pv01.GetPV01() * pv01.GetQuantity();
      totalQuantity += pv01.GetQuantity();
    }

    static PV01<BucketedSector<T>> bucketedRisk(sector, totalPv01, totalQuantity);
    return bucketedRisk;
  }

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

// Implementation of BucketedSector methods
template<typename T>
BucketedSector<T>::BucketedSector(const vector<T>& _products, string _name) : products(_products), name(_name) {}

template<typename T>
const vector<T>& BucketedSector<T>::GetProducts() const {
  return products;
}

template<typename T>
const string& BucketedSector<T>::GetName() const {
  return name;
}

#endif
