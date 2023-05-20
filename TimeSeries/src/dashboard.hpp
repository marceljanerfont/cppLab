#pragma once

#include <map>

#include "time_series.hpp"

// to delete
#define MAX_TIMESERIES_HISTORY 10
#define ATTRIBUTES_SIZE 18



/**
* @brief TimeSeriesMap
* 
*/
template <typename T, typename U>
class TimeSeriesMap {
public:

  TimeSeriesMap() {}
  ~TimeSeriesMap() {}

  // replace TimeSeries if it exists
  std::shared_ptr<TimeSeries<T, U>> registerTimeSeries(const std::string& key,
    const size_t& max_size, std::unique_ptr<BestAlgorithm<T, U>> best_algo = std::make_unique<NullAlgorithm<T, U>>()) {
    return (map_[key] = std::make_shared<TimeSeries<T, U>>(max_size, std::move(best_algo)));
  }

  // returns empty std::shared_ptr if doesn't exists
  std::shared_ptr<TimeSeries<T, U>> getTimeSeries(const std::string& key) {
    auto it = map_.find(key);
    if (it == map.enmd()) { //not found
      return std::shared_ptr<TimeSeries<T, U>>(); // return empty ptr
    }
    return it->second;
  }

private:
  std::map<std::string, std::shared_ptr<TimeSeries<T, U>>> map_;
};

/**
* @brief Dashboard
* where all TimeSeries are managed
*/
class Dashboard:
  private TimeSeriesMap<int, int>,
  private TimeSeriesMap<unsigned int, unsigned int>,
  private TimeSeriesMap<float, float>,
  private TimeSeriesMap<Pose, Pose>

{
public:
  Dashboard() {}
  ~Dashboard() {}

  // replace TimeSeries if it exists
  template <typename T, typename U>
  std::shared_ptr<TimeSeries<T, U>> registerTimeSeries(const std::string& key, const size_t& max_size,
    std::unique_ptr<BestAlgorithm<T, U>> best_algo = std::make_unique<NullAlgorithm<T, U>>()) {
    return TimeSeriesMap<T, U>::registerTimeSeries(key, max_size, std::move(best_algo));
  }

  // returns empty std::shared_ptr if doesn't exists
  template <typename T, typename  U>
  std::shared_ptr<TimeSeries<T, U>> getTimeSeries(const std::string& key) {
    return TimeSeriesMap<T, U>::getTimeSeries(key);
  }
};

///////////////////////////////////////////////////////////


/*
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, std::vector<float>, AccumulatedConfidences<ATTRIBUTES_SIZE>>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, std::vector<data::Detection>, DetectionClassIdAll>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, float>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, unsigned int, BitMaskOccurrences<4>>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, unsigned int, BitMaskOccurrences<2>>
  */

/*

namespace data {
  struct Rectangle {
    unsigned int x_tl_; // x coordinate of the top left corner
    unsigned int y_tl_; // y coordinate of the top left corner
    unsigned int x_br_; // x coordinate of the bottom right corner
    unsigned int y_br_; // y coordinate of the bottom right corner
  };
  struct Detection {
    float confidence_;
    unsigned int class_id_;
    float threashold_;
  };
}
*/
/**
* @brief DetectionClassIdAll
* It computes Best Value (CLASS_ID) from ALL of occurrences of class_id_ in Time Series of Detections.
* It does not compute Best Confidence (not needed from the time being)


struct DetectionClassIdAll {
  using DataType = std::vector<data::Detection>;;

  void clear() {
    count_map_.clear();
    best_value_ = 0;
  }

  void removeOldValue(const DataType& value) {
    // remove old detection class_id from the counter map
    for (const auto& d : value) {
      count_map_[d.class_id_]--;
    }
  }

  void addNewValue(const DataType& value) {
    // count the class_id occurrence
    for (const auto& d : value) {
      count_map_[d.class_id_]++;
    }

    // compute best_value_
    best_value_ = 0;
    for (const auto& [class_id, count] : count_map_) {
      if (count > 0) {
        best_value_ |= (1 << class_id);
      }
    }
  }

  std::unordered_map<unsigned int, unsigned int> count_map_;
  unsigned int best_value_ = 0;
}; 
*/