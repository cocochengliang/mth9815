/**
 * inquiryservice.hpp
 * Defines the data types and Service for customer inquiries.
 *
 * @author Breman Thuraisingham
 */
#ifndef INQUIRY_SERVICE_HPP
#define INQUIRY_SERVICE_HPP

#include "soa.hpp"
#include "tradebookingservice.hpp"

// Various inquiry states
enum InquiryState { RECEIVED, QUOTED, DONE, REJECTED, CUSTOMER_REJECTED };

/**
 * Inquiry object modeling a customer inquiry from a client.
 * Type T is the product type.
 */
template<typename T>
class Inquiry
{

public:

  // ctor for an inquiry
  Inquiry(std::string _inquiryId, const T &_product, Side _side, long _quantity, double _price, InquiryState _state) :
    inquiryId(_inquiryId), product(_product), side(_side), quantity(_quantity), price(_price), state(_state) {}

  // Get the inquiry ID
  const std::string& GetInquiryId() const { return inquiryId; }

  // Get the product
  const T& GetProduct() const { return product; }

  // Get the side on the inquiry
  Side GetSide() const { return side; }

  // Get the quantity that the client is inquiring for
  long GetQuantity() const { return quantity; }

  // Get the price that we have responded back with
  double GetPrice() const { return price; }

  // Get the current state on the inquiry
  InquiryState GetState() const { return state; }

  // Set the state
  void SetState(InquiryState newState) { state = newState; }

  // Set the price
  void SetPrice(double newPrice) { price = newPrice; }

private:
  std::string inquiryId;
  T product;
  Side side;
  long quantity;
  double price;
  InquiryState state;

};

/**
 * Service for customer inquiry objects.
 * Keyed on inquiry identifier (NOTE: this is NOT a product identifier since each inquiry must be unique).
 * Type T is the product type.
 */
template<typename T>
class InquiryService : public Service<std::string, Inquiry<T>>
{

public:

  // Send a quote back to the client
  void SendQuote(const std::string &inquiryId, double price) {
    if (dataStore.find(inquiryId) == dataStore.end()) {
      throw std::runtime_error("Inquiry not found for ID: " + inquiryId);
    }

    auto& inquiry = dataStore[inquiryId];
    inquiry.SetPrice(price);
    inquiry.SetState(QUOTED);
    for (auto& listener : listeners) {
      listener->ProcessUpdate(inquiry);
    }
  }

  // Reject an inquiry from the client
  void RejectInquiry(const std::string &inquiryId) {
    if (dataStore.find(inquiryId) == dataStore.end()) {
      throw std::runtime_error("Inquiry not found for ID: " + inquiryId);
    }

    auto& inquiry = dataStore[inquiryId];
    inquiry.SetState(REJECTED);
    for (auto& listener : listeners) {
      listener->ProcessUpdate(inquiry);
    }
  }

  // Add an inquiry to the service
  void OnMessage(Inquiry<T>& inquiry) override {
    dataStore[inquiry.GetInquiryId()] = inquiry;
    for (auto& listener : listeners) {
      listener->ProcessAdd(inquiry);
    }
  }

  // Get data by inquiry ID
  Inquiry<T>& GetData(std::string inquiryId) override {
    if (dataStore.find(inquiryId) == dataStore.end()) {
      throw std::runtime_error("Inquiry not found for ID: " + inquiryId);
    }
    return dataStore[inquiryId];
  }

  // Add a listener to the service
  void AddListener(ServiceListener<Inquiry<T>>* listener) override {
    listeners.push_back(listener);
  }

  // Get all listeners
  const std::vector<ServiceListener<Inquiry<T>>*>& GetListeners() const override {
    return listeners;
  }

private:
  std::map<std::string, Inquiry<T>> dataStore; // Map to store inquiries by ID
  std::vector<ServiceListener<Inquiry<T>>*> listeners; // Listeners to notify
};

#endif // INQUIRY_SERVICE_HPP
