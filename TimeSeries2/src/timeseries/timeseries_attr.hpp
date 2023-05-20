#pragma once

#include <vector>


/**
* @brief AccumulatedConfidences
* It does not compute Best Value nor Best Confidence from TimeSeries.
* It updated the accumulated confidences of all Time Series
*/
template <std::size_t SIZE>
struct AccumulatedConfidences {
  void clear() {
    acc_confidences_ = std::vector<float>(SIZE, 0.f);
  }

  void removeOldValue(const std::vector<float>& value) {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), value.cbegin(), acc_confidences_.begin(), std::minus<float>());
  }

  void addNewValue(const std::vector<float>& value) {
    std::transform(acc_confidences_.begin(), acc_confidences_.end(), value.cbegin(), acc_confidences_.begin(), std::plus<float>());
  }

  std::vector<float> acc_confidences_ = std::vector<float>(SIZE, 0.f);
};