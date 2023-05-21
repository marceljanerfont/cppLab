#pragma once

#include <vector>

/**
* @brief AccumulatedConfidences
* It does not compute Best Value nor Best Confidence from TimeSeries.
* It updated the accumulated confidences of all Time Series
*/
struct TsAccumulatedConfidences : public TimeSeries<std::vector<float>> {
  TsAccumulatedConfidences(const std::size_t& max_size, const std::size_t& max_attr) :
    TimeSeries<std::vector<float>>(max_size), max_attr_(max_attr) {
    acc_confidences_ = std::vector<float>(max_attr_, 0.f);
  }

  void clear() override {
    TimeSeries::clear();
    acc_confidences_ = std::vector<float>(max_attr_, 0.f);
  }

  void updateRemovingOldestValue(const std::vector<float>& old_value) override {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), old_value.cbegin(), acc_confidences_.begin(), std::minus<float>());
  }

  void updateAddingNewestValue(const std::vector<float>& new_value) override {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), new_value.cbegin(), acc_confidences_.begin(), std::plus<float>());
  }

  std::vector<float> getBestValue() const override {
    return acc_confidences_;
  };

  std::size_t max_attr_;
  std::vector<float> acc_confidences_;
};
////////////////////////////////////////

class TsHumanAttr:
  public TsAccumulatedConfidences {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_ATTR = 42;
  TsHumanAttr() : TsAccumulatedConfidences(MAX_SIZE, MAX_ATTR) {}
};

////////////////////////////////////////

class TsFaceAttr :
  public TsAccumulatedConfidences {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_ATTR = 18;
  TsFaceAttr() : TsAccumulatedConfidences(MAX_SIZE, MAX_ATTR) {}
};

