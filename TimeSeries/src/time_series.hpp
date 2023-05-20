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
  Sample() : timestamp_(NO_TIMESTAMP) {}
  Sample(const T& value, const uint64_t& timestamp) : value_(value), timestamp_(timestamp) {}
  T value_;
  uint64_t timestamp_;
};

// todelete
struct Joint {
  int x_;
  int y_;
};

struct Pose {
  static const unsigned int JOINT_TOTAL{ 14 };
  Joint joints_[JOINT_TOTAL];
};


/**
* @brief BestAlgorithm is an abstract base class that traits TimeSeries 
* in order to find best value of the time series
*/
template<typename T, typename U>
class BestAlgorithm {
public:
  virtual void clear() = 0;
  virtual void removeOldValue(const T& value) = 0;
  virtual void addNewValue(const T& value) = 0;
  virtual U getBestValue() const = 0;
};

/**
* @brief NullAlgorithm
* It does not compute Best Value nor Best Confidence from TimeSeries. It does nothing.
*/
template <typename T, typename U>
class NullAlgorithm: public BestAlgorithm<T, U> {
public:
  void clear() override {}
  void removeOldValue(const T& value) override {}
  void addNewValue(const T& value) override { }
  U getBestValue() const override {
    return 0;
  }
};

/**
* @brief NewestValue
* The best value, is the last added (the newest)
*/
template <typename T, typename U>
class NewestValue : public BestAlgorithm<T, U> {
public:
  void clear() override { best_value_ = 0; }
  void removeOldValue(const T& value) override {}
  void addNewValue(const T& value) override { best_value_ = value; }
  U getBestValue() const override {
    return best_value_;
  }
  U best_value_;
};

/**
* @brief TopFrequency
* Best Value is the most frequently occurring value in the TimeSeries.
*/
template <typename T, typename U>
class TopFrequency : public BestAlgorithm<T, U> {
public:
  void clear() override { count_map_.clear(); }
  void removeOldValue(const T& value) override { count_map_[value]--; }
  void addNewValue(const T& value) override { count_map_[value]++; }
  U getBestValue() const override {
    U best_value = 0;
    std::size_t max_count = 0;
    for (const auto& pair : count_map_) {
      if (pair.second > max_count) {
        max_count = pair.second;
        best_value = pair.first;
      }
    }
    return best_value;
  }
private:
  std::unordered_map<T, U> count_map_;
};

/**
* @brief TopFrequencyBitmask
* It computes Best Value from number of occurrences of bits position in a bitmask values.
*/
class TopFrequencyBitmask : public BestAlgorithm<unsigned int, unsigned int> {
public:
  TopFrequencyBitmask(const unsigned int& max_flags = 1) : BestAlgorithm<unsigned int, unsigned int>(), max_flags_(max_flags) {}
  void clear() override { 
    count_map_.clear();
  }
  void removeOldValue(const unsigned int& value) override { 
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }
  void addNewValue(const unsigned int& value) override { 
    // count the bit position
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
  }
  unsigned int getBestValue() const override {
    // compute best_value_
    unsigned int best_value = 0;
    using Pair = std::pair<unsigned int, unsigned int>;
    struct CompareCount {
      bool operator() (const Pair& l, const Pair& r) const { return r.second > l.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, CompareCount> queue(count_map_.begin(), count_map_.end());

    for (unsigned int i = 0; (i < max_flags_ && !queue.empty()); ++i) {
      best_value |= (1 << queue.top().first);
      queue.pop();
    }

    return best_value;
  }

private:
  const unsigned int MAX_BITS_{ 32 };
  std::unordered_map<unsigned int, unsigned int> count_map_;
  unsigned int max_flags_;  
};


/**
* @brief BitMaskOccurrences
* It computes Best Value from number of occurrences of bits position in a bitmask values.
* It does not compute Best Confidence (not needed from the time being)
*/
template <unsigned int SIZE>
struct BitMaskOccurrences {
  using DataType = unsigned int;
  using BestType = unsigned int;

  void clear() {
    count_map_.clear(); 
    best_value_ = 0;
  }

  void removeOldValue(const DataType& value) {
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < 32; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }

  void addNewValue(const DataType& value) {
    // count the bit position
    for (unsigned int k = 0; k < 32; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
    // compute best_value_
    best_value_ = 0;
    using Pair = std::pair<unsigned int, unsigned int>;
    struct CompareCount{
      bool operator() (const Pair& l, const Pair& r) const { return r.second > l.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, CompareCount> queue(count_map_.begin(), count_map_.end());

    for (unsigned int i = 0; (i < SIZE && !queue.empty()); ++i) {
      best_value_ |= (1 << queue.top().first);
      queue.pop();
    }
  }

  std::unordered_map<unsigned int, unsigned int> count_map_;
  BestType best_value_;
};

/**
* @brief AccumulatedConfidences
* It does not compute Best Value nor Best Confidence from TimeSeries.
* It updated the accumulated confidences of all Time Series
*/
template <std::size_t SIZE>
struct AccumulatedConfidences {
  using DataType = std::vector<float>;

  void clear() {
    acc_confidences_ = std::vector<float>(SIZE, 0.f);
  }

  void removeOldValue(const DataType& value) {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), value.cbegin(), acc_confidences_.begin(), std::minus<float>());
  }

  void addNewValue(const DataType& value) {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), value.cbegin(), acc_confidences_.begin(), std::plus<float>());
  }
  
  std::vector<float> acc_confidences_ = std::vector<float>(SIZE, 0.f);
};

/**
* @brief TimeSeries it's a circular buffer of Samples
* THIS CLASS IS NOT THREAD SAFE!
*/
template <typename T, typename U>
class TimeSeries {
public:

  TimeSeries(const size_t& max_size,
    std::unique_ptr<BestAlgorithm<T, U>> best_algo = std::make_unique<NullAlgorithm<T, U>>()):
    samples_(max_size), best_algo_(std::move(best_algo)) { }

  void clear() {
    samples_.clear();
    best_algo_->clear();
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

  void addSample(const T& value, const uint64_t& timestamp) {
    // Algorithm takes into account the oldest sample being removed
    if (samples_.full()) {
      best_algo_->removeOldValue(samples_.front().value_);
    }

    // add new value into Time Series
    samples_.push_back(Sample<T>(value, timestamp));

    // Algorithm takes into account the new added value
    best_algo_->addNewValue(value);
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

  U getBestValue() const {
    return best_algo_->getBestValue();
  }

  BestAlgorithm<T, U>* getBestValueAlgorithm() {
    return best_algo_.get();
  }

private:
  boost::circular_buffer<Sample<T>> samples_;
  std::unique_ptr<BestAlgorithm<T, U>> best_algo_;
};


/**
* @brief TimeSeries it's a circular buffer of Samples
* THIS CLASS IS NOT THREAD SAFE!

template <std::size_t SIZE, typename T, typename Algo = NullAlgorithm<T>>
class TimeSeries {
public:

  TimeSeries() {}

  void clear() {
    samples_.clear();
    algo_.clear();
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

  void addSample(const T& value, const uint64_t& timestamp) {
    // Algorithm takes into account the oldest sample being removed
    if (samples_.full()) {
      algo_.removeOldValue(samples_.front().value_);
    }

    // add new value into Time Series
    samples_.push_back(Sample<T>(value, timestamp));

    // Algorithm takes into account the new added value
    algo_.addNewValue(value);
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

  Algo algo_;
  

private:
  boost::circular_buffer<Sample<T>> samples_{ SIZE };
};
*/