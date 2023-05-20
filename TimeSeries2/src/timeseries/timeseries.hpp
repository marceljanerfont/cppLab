#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <boost/circular_buffer.hpp>

#define NO_TIMESTAMP 0ULL
/**
* @brief Sample
* T is the attribute type
*/
template <typename T>
struct Sample {
  using ValueType = T;

  Sample() : timestamp_(NO_TIMESTAMP) {}
  Sample(const T& value, const uint64_t& timestamp) : value_(value), timestamp_(timestamp) {}
  T value_;
  uint64_t timestamp_;
};

/**
* @brief TimeSeries it's a circular buffer of Samples
* THIS CLASS IS NOT THREAD SAFE!
*/
template <typename T>
class TimeSeries {
public:
  using ValueType = T;

  TimeSeries(const size_t& max_size) : samples_(max_size) { }

  ////  virtual functions
  virtual void clear() {
    samples_.clear();
  }

  virtual void updateRemovingOldestValue(const T& old_value) {}
  
  virtual void updateAddingNewestValue(const T& new_value) {}
  ////
  
  void addSample(const uint64_t& timestamp, const T& value) {
    // Algorithm takes into account the oldest sample being removed
    if (samples_.full()) {
      updateRemovingOldestValue(oldestValue());
    }
    // add new value into Time Series
    samples_.push_back(Sample<T>(value, timestamp));
    // Algorithm takes into account the new added value
    updateAddingNewestValue(value);
  }

  // the oldest sample
  const Sample<T> oldestSample() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range: the TimeSeries is empty");
    }
    return samples_.front();
  }

  const T oldestValue() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range: the TimeSeries is empty");
    }
    return samples_.front().value_;
  }

  // the newest sample
  const Sample<T> newestSample() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range: the TimeSeries is empty");
    }
    return samples_.back();
  }

  const T newestValue() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range: the TimeSeries is empty");
    }
    return samples_.back().value_;
  }

  // postition zero is the oldest
  const Sample<T> at(const std::size_t& pos) const {
    if (pos >= samples_.size()) {
      throw std::out_of_range("Out of range: the TimeSeries is empty");
    }
    return samples_[pos];
  }

  std::size_t size() const {
    return samples_.size();
  }

  bool empty() const {
    return samples_.empty();
  }

  std::size_t capacity() const {
    return samples_.capacity();
  }

  // oldest first
  const boost::circular_buffer<Sample<T>>& samples() const {
    return samples_;
  }

  // it does a copy, oldest first
  const std::vector<T> valuesCopy() const {
    std::vector<T> values;
    std::transform(samples_.begin(), samples_.end(), std::back_inserter(values), std::mem_fn(&Sample<T>::value_));
    return values;
  }

protected:
  boost::circular_buffer<Sample<T>> samples_;
};
