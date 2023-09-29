#pragma once

#include <typeindex>
#include <map>
#include <cassert>
#include <unordered_map>


class Dashboard {
public:
  static const std::size_t MAX_SIZE = 10;
  typedef std::vector<std::size_t> index_vector_t;

  Dashboard() {}

  ~Dashboard() {}

  template <typename T>
  void addSample(const uint64_t& timestamp, T& value) {
    // find all indices
    index_vector_t indices = getTimeSeriesIndices<T>();

    // timeseries doesn't exist yet
    if (indices.empty()) { 
      // create timeseries
      std::shared_ptr<TimeSeries<T>> timeseries = std::make_shared<TimeSeries<T>>(MAX_SIZE);
      addTimeSeries<T>(timeseries);
      // update statistics
      value.updateStatistics(timeseries);
      // add sample
      timeseries->addSample(timestamp, value);
      
      return;
    }
    // unique id?
    if (value.id_ == Attribute::UNIQUE_ID) {
      //std::assert(indices.size() == 1);
      std::shared_ptr<TimeSeries<T>> timeseries = std::static_pointer_cast<TimeSeries<T>>(timeseries_[indices.front()]);
      // update statistics
      value.updateStatistics(timeseries);
      // add sample
      timeseries->addSample(timestamp, value);

      return;
    }
    // multiple id
    // find a similar attribute
    for (auto& index : indices) {
      std::shared_ptr<TimeSeries<T>> timeseries = std::static_pointer_cast<TimeSeries<T>>(timeseries_[index]);
      const T& otherAttribute = timeseries->newestValue();
      if (otherAttribute.checkSimilarity(value)) { // found similar
        // update statistics
        value.updateStatistics(timeseries);
        // add sample
        timeseries->addSample(timestamp, value);
        
        return;
      }
    }

    // not similar attribute found
    std::shared_ptr<TimeSeries<T>> timeseries = std::make_shared<TimeSeries<T>>(MAX_SIZE);
    addTimeSeries<T>(timeseries);
    // update statistics
    //value.updateStatistics(timeseries);
    // add sample
    timeseries->addSample(timestamp, value);
  }

  template <typename T>
  T getNewestValue(const int& id = Attribute::UNIQUE_ID) const {
    std::shared_ptr<TimeSeries<T>> timeseries = getTimeSeries<T>(id);

    if (timeseries) {
      return timeseries->newestValue();
    }
    return T();
  }

  

  template <typename T>
  std::shared_ptr<TimeSeries<T>> getTimeSeries(const int& id = Attribute::UNIQUE_ID) const {
    // find all indices
    index_vector_t indices = getTimeSeriesIndices<T>();
    // is the id for that timeseries?
    if (id >= indices.size()) {
      return nullptr;
    }

    return std::static_pointer_cast<TimeSeries<T>>(timeseries_[indices[id]]);
  }

  template <typename T>
  std::vector<std::shared_ptr<TimeSeries<T>>> getAllTimeSeries() {
    std::vector<std::shared_ptr<TimeSeries<T>>> all_timeseries;
    // find all indices
    index_vector_t indices = getTimeSeriesIndices<T>();
    for (auto& index : indices) {
      std::shared_ptr<TimeSeries<T>> timeseries = std::static_pointer_cast<TimeSeries<T>>(timeseries_[index]);
      all_timeseries.push_back(timeseries);
    }

    return all_timeseries;
  }

private:
  template <typename T>
  index_vector_t getTimeSeriesIndices() const {
    auto it = index_map_.find(typeid(T));
    if (it != index_map_.end()) {
      return it->second;
    }

    return index_vector_t(); // no indices
  }

  template <typename T>
  void addTimeSeries(std::shared_ptr<void> timeseries) {
    // store new timeseries
    timeseries_.push_back(timeseries);
    index_vector_t& indices = index_map_[typeid(T)];
    indices.push_back(timeseries_.size() - 1);
  }

  //template <typename T>
  //void addSampleIntoTimeSeries(std::shared_ptr<TimeSeries<T>> timeseries, T& samples)

  std::unordered_map<std::type_index, index_vector_t> index_map_;
  
  
  
  std::vector<std::shared_ptr<void>> timeseries_;
};