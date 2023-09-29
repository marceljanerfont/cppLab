#pragma once

#include <vector>
#include <queue>
#include <functional>
#include <boost/circular_buffer.hpp>

#define NO_TIMESTAMP 0ULL

//namespace timeseries {
  /**
  * @brief Sample
  * T is the attribute type
  */
  template <typename T>
  struct Sample {
    using ValueType = T;

    Sample() : timestamp_(NO_TIMESTAMP) {}
    Sample(const uint64_t& timestamp, const T& value) : timestamp_(timestamp), value_(value) {}
    uint64_t timestamp_;
    T value_;
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
    virtual void clear() { samples_.clear(); }
    virtual bool isClosed() { return false; }
    // to notify that oldest value is about to remove it
    virtual void onRemovingOldestValue(const T& old_value) {}
    // to notify that a new value is about to be added 
    virtual void onAddingNewestValue(const T& new_value) {}
    // to notify that a new value has been added
    virtual void updateStatistics() {}
    ///////////////////////

    virtual void addSample(const uint64_t& timestamp, const T& value) {
      // check if the TimeSeries is already closed, means it has an sticked value
      if (!isClosed()) {
        if (samples_.full()) {
          onRemovingOldestValue(oldestValue());
        }
        onAddingNewestValue(value);
        // add new value into TimeSeries
        samples_.push_back(Sample<T>(timestamp, value));
        // update statistics (data aggregation)
        updateStatistics();
      }
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
//}
