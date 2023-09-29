#pragma once

#include <vector>

#include "timeseries.hpp"

/**
* @brief AccumulatedConfidences
* It does not compute Best Value nor Best Confidence from TimeSeries.
* It updated the accumulated confidences of all Time Series
*/
class TsAccumulatedConfidences : public TimeSeries<std::vector<float>> {
public:
  TsAccumulatedConfidences(const std::size_t& max_size, const std::size_t& max_attr) :
    TimeSeries<std::vector<float>>(max_size), max_attr_(max_attr) {
    acc_confidences_ = std::vector<float>(max_attr_, 0.f);
  }

  virtual void clear() override {
    TimeSeries::clear();
    acc_confidences_ = std::vector<float>(max_attr_, 0.f);
  }

  virtual void updateRemovingOldestValue(const std::vector<float>& old_value) override {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), old_value.cbegin(), acc_confidences_.begin(), std::minus<float>());
  }

  virtual void updateAddingNewestValue(const std::vector<float>& new_value) override {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), new_value.cbegin(), acc_confidences_.begin(), std::plus<float>());
  }

  virtual std::vector<float> getBestValue() const override {
    return acc_confidences_;
  };

protected:
  std::vector<float> acc_confidences_;

private:
  std::size_t max_attr_;
};
