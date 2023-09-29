#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <typeindex>

#define NO_TIMESTAMP 0ULL


class IData {
public:
  IData() {}
  virtual void clear() = 0;
};

template <typename T>
class DataT: public IData {
public:
  DataT(const T& value): IData(), value_(value) {}
  virtual void clear() override {};
  T value_;
};


/**
* @brief TimeSeries it's a circular buffer of Samples
* THIS CLASS IS NOT THREAD SAFE!
*/

typedef std::vector<std::shared_ptr<IData>> data_t;
typedef std::map<uint64_t, data_t> ts_data_map_t;

class TimeSeries {
  static const std::size_t DEFAULT_MAX_SIZE = 10;
public:

  // map[type_k][ts_i] --> vector_data
  std::unordered_map <std::type_index, ts_data_map_t> type_data_map_;
  
  // to limit timeseries size
  std::unordered_map <std::type_index, std::size_t> max_size_map_;


  TimeSeries() { }

  //////  virtual functions
  //virtual void clear() { samples_.clear(); }
  //virtual void updateRemovingOldestValue(const T& old_value) {}
  //virtual void updateAddingNewestValue(const T& new_value) {}
  //virtual T getBestValue() const { return newestValue(); }
  /////////////////////////

  template <typename T>
  void setMaxSize(const std::size_t& max_size) {
    max_size_map_[typeid(T)] = max_size;
    // TODO: delete older samples
  }

  size_t getMaxSize(const std::type_index& data_type) {
    auto it = max_size_map_.find(data_type);
    if (it != max_size_map_.end()) {
      return it->second;
    }
    // not found: use default max_size
    return DEFAULT_MAX_SIZE;
  }


  void pushData(const uint64_t& timestamp, std::shared_ptr<IData> data) {
    std::type_index data_type = typeid(IData);
    // get or create timestamp->data map
    ts_data_map_t& ts_data_map = type_data_map_[data_type];

    // if buffer is full, remove oldest data
    const size_t max_size = getMaxSize(data_type);
    if (ts_data_map.size() > max_size - 1) {
      // remove old data
      // TODO: call remove old data callback
      auto it = ts_data_map.begin();
      ts_data_map.erase(it);
    }

    ts_data_map[timestamp].push_back(data);
  }

  template <typename T>
  data_t getData(const uint64_t& timestamp) {
    std::type_index data_type = typeid(T);
    auto it = type_data_map_.find(data_type);
    if (it != type_data_map_.end()) {
      ts_data_map_t& ts_data_map = it->second;
      auto it_ts = ts_data_map.find(timestamp);
      if (it_ts != ts_data_map.end()) {
        return ts_data_map[timestamp];
      }
    }
    return data_t();
  }

};
