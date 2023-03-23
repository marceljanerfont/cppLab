#pragma once

#include <vector>
#include <functional>
#include <boost/circular_buffer.hpp>

#define NO_TIMESTAMP 0ULL
/**
* @brief Sample
* T is the attribute type
*/
template <typename T>
struct Sample {
  Sample() : timestamp_(NO_TIMESTAMP) {}
  Sample(const T& value, const uint64_t& timestamp) : value_(value), timestamp_(timestamp) {}
  T value_;
  uint64_t timestamp_;
};

/**
* @brief TimeSeries it's a circular buffer of Samples
* THIS CLASS IS NOT THREAD SAFE!
*/
template <typename T, std::size_t SIZE>
class TimeSeries {
public:
  TimeSeries() {}

  void clear() {
    samples_.clear();
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

  void addSample(const Sample<T>& sample) {
    samples_.push_back(sample);
  }

  void addSample(const T& value, const uint64_t& timestamp) {
    samples_.push_back(Sample<T>(value, timestamp));
  }

  // the oldest sample
  const Sample<T> oldestSample() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range");
    }
    return samples_.front();
  }

  // the newest sample
  const Sample<T> newestSample() const {
    if (samples_.empty()) {
      throw std::out_of_range("Out of range");
    }
    return samples_.back();
  }

  // postition zero is the oldest
  const Sample<T> at(const std::size_t& pos) const {
    if (pos >= samples_.size()) {
      throw std::out_of_range("Out of range");
    }
    return samples_[pos];
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

private:
  boost::circular_buffer<Sample<T>> samples_{ SIZE };
};