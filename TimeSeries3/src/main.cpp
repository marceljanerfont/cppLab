#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>


#include "gtest/gtest.h"

#include "time_series.hpp"
#include "dashboard.hpp"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}


struct MyClass {
  std::string id_;

  MyClass(const std::string &id) : id_(id) {}
  void printMsg(const std::string &msg) {
    std::cout << "MyClass msg: " << msg << std::endl;
 
  }
};

/*
TEST(TimeSeries, out_of_range) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;
  TimeSeries<MAX_SIZE, int> timeseries;

  try {
    Sample<int> newest = timeseries.newestSample();
  }
  catch (const std::out_of_range& e) {
    EXPECT_EQ(e.what(), std::string("Out of range"));
  }
  catch (...) {
    FAIL() << "Expected std::out_of_range";
  }

  try {
    Sample<int> random = timeseries.at(3);
  }
  catch (const std::out_of_range& e) {
    EXPECT_EQ(e.what(), std::string("Out of range"));
  }
  catch (...) {
    FAIL() << "Expected std::out_of_range";
  }

}

TEST(TimeSeries, fill_and_retrieve) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;
  TimeSeries<MAX_SIZE, int> timeseries;

  EXPECT_EQ(timeseries.capacity(), MAX_SIZE);

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(i, i);
    EXPECT_EQ(timeseries.size(), i < MAX_SIZE ? i : MAX_SIZE);
  }

  EXPECT_EQ(timeseries.size(), MAX_SIZE);
  EXPECT_EQ(timeseries.size(), timeseries.capacity());

  Sample<int> oldest = timeseries.oldestSample();
  EXPECT_EQ(oldest.value_, NB_SAMPLES - MAX_SIZE + 1);

  Sample<int> newest = timeseries.newestSample();
  EXPECT_EQ(newest.value_, NB_SAMPLES);

}


TEST(TimeSeries, interator_oldest_first) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;


  TimeSeries<MAX_SIZE, int> timeseries;

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(i, i);
  }

  int oldest = NB_SAMPLES - MAX_SIZE + 1;
  int current = oldest;
  
  for (const auto& sample : timeseries.samples()) {
    EXPECT_EQ(sample.value_, current);
    ++current;
  }

  current = oldest;
  const auto& samples = timeseries.samples();
  const std::vector<int> values = timeseries.valuesCopy();
  for (int i = 0; i < samples.size() - 1; ++i) {
    EXPECT_EQ(samples[i].value_, values[i]);
    EXPECT_EQ(samples[i].value_, current);
    ++current;
  }
  


}

TEST(TimeSeries, interator_newest_first) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;


  TimeSeries<MAX_SIZE, int> timeseries;

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(i, i);
  }

  int newest = NB_SAMPLES;
  int current = newest;
  for (const auto& sample : boost::adaptors::reverse(timeseries.samples())) {
    EXPECT_EQ(sample.value_, current);
    --current;
  }

  current = newest;
  for (auto iter = timeseries.samples().rbegin(); iter != timeseries.samples().rend(); ++iter) {
    EXPECT_EQ(iter->value_, current);
    --current;
  }

  int nb_iterations = 0;
  auto iter = timeseries.samples().rbegin();
  while (iter != timeseries.samples().rend()) {
    int current = iter->value_;
    ++iter;
    if (iter != timeseries.samples().rend()) {
      int previous = iter->value_;
      ++nb_iterations;
      EXPECT_EQ(current-previous, 1);
    }
  }

  EXPECT_EQ(nb_iterations, MAX_SIZE-1);

  // it iterates backwards but moving 2 positions rather than one
  nb_iterations = 0;
  for (auto iter = timeseries.samples().rbegin(); iter != timeseries.samples().rend() - 1; ++iter) {
    int current = iter->value_;
    ++iter;
    int previous = iter->value_;
    ++nb_iterations;
    EXPECT_EQ(current - previous, 1);
  }

  EXPECT_EQ(nb_iterations, (MAX_SIZE - 1)/2);

}
*/
/*
/// Dashboard
TEST(Dashboard, creation_top_frequency) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;

  Dashboard dashboard;

  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("key1", MAX_SIZE, std::make_unique<TopFrequency<unsigned int, unsigned int>>());

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  for (unsigned int i = 1; i <= NB_SAMPLES; ++i) {
    ts->addSample(i, i);
    EXPECT_EQ(ts->size(), i < MAX_SIZE ? i : MAX_SIZE);
  }
  // repeat last value
  ts->addSample(NB_SAMPLES, NB_SAMPLES + 1);

  EXPECT_EQ(ts->size(), MAX_SIZE);
  EXPECT_EQ(ts->size(), ts->capacity());

  Sample<unsigned int> oldest = ts->oldestSample();
  EXPECT_EQ(oldest.value_, NB_SAMPLES - MAX_SIZE + 2);

  Sample<unsigned int> newest = ts->newestSample();
  EXPECT_EQ(newest.value_, NB_SAMPLES);

  EXPECT_EQ(NB_SAMPLES, ts->getBestValue());

}

TEST(Dashboard, newest_value) {
  const size_t MAX_SIZE = 3;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<float, float>("key1", MAX_SIZE, std::make_unique<NewestValue<float, float>>());

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(0.1f, 1);
  ts->addSample(0.2f, 2);
  ts->addSample(0.3f, 3);

  EXPECT_EQ(0.3f, ts->getBestValue());
}



//////

///  TOP FREQUENCY EXAMPLE
/**
* @brief TimeSeriesTopFreq
* Best Value is the most frequently occurring value in the TimeSeries.
*/

/*
template <typename T, typename U>
class TimeSeriesTopFreq : public TimeSeries<T, U> {
public:
  TimeSeriesTopFreq(const size_t& max_size) : TimeSeries<T, U>(max_size) {}
  void clear() override {
    TimeSeries<U, U>::clear();
    count_map_.clear();
  }
  void updateRemovingOldestValue() override {
    const unsigned int value = oldestValue();
    count_map_[value]--;
  };
  void updateAddingNewestValue() override {
    const unsigned int value = newestValue();
    count_map_[value]++;
  };
  U getBestValue() override {
    best_value_ = 0;
    std::size_t max_count = 0;
    for (const auto& pair : count_map_) {
      if (pair.second > max_count) {
        max_count = pair.second;
        best_value_ = pair.first;
      }
    }
    return best_value_;
  }

private:
  std::unordered_map<T, U> count_map_;
};

TEST(Dashboard, top_frequency) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("top_ts", std::make_shared<TimeSeriesTopFreq<unsigned int, unsigned int>>(MAX_SIZE));

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  for (unsigned int i = 1; i <= NB_SAMPLES; ++i) {
    ts->addSample(i, i);
    EXPECT_EQ(ts->size(), i < MAX_SIZE ? i : MAX_SIZE);
  }
  // repeat last value
  ts->addSample(NB_SAMPLES, NB_SAMPLES + 1);

  EXPECT_EQ(ts->size(), MAX_SIZE);
  EXPECT_EQ(ts->size(), ts->capacity());

  Sample<unsigned int> oldest = ts->oldestSample();
  EXPECT_EQ(oldest.value_, NB_SAMPLES - MAX_SIZE + 2);

  Sample<unsigned int> newest = ts->newestSample();
  EXPECT_EQ(newest.value_, NB_SAMPLES);
  EXPECT_EQ(NB_SAMPLES, ts->getBestValue());
}

///  TOP FREQUENCY BITMASK EXAMPLE

class TimeSeriesTopFreqBitmask : public TimeSeries<unsigned int, unsigned int> {
public:
  TimeSeriesTopFreqBitmask(const size_t& max_size, const unsigned int& max_flags = 1) : 
    TimeSeries<unsigned int, unsigned int>(max_size), max_flags_(max_flags) {}
  void clear() override {
    TimeSeries<unsigned int, unsigned int>::clear();
    count_map_.clear();
  }
  void updateRemovingOldestValue() override {
    const unsigned int value = oldestValue();
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }
  void updateAddingNewestValue() override {
    const unsigned int value = newestValue();
    // count the bit position
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
  }
  unsigned int getBestValue() override {
    // compute best_value_
    best_value_ = 0;
    using Pair = std::pair<unsigned int, unsigned int>;
    struct CompareCount {
      bool operator() (const Pair& l, const Pair& r) const { return r.second > l.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, CompareCount> queue(count_map_.begin(), count_map_.end());

    for (unsigned int i = 0; (i < max_flags_ && !queue.empty()); ++i) {
      best_value_ |= (1 << queue.top().first);
      queue.pop();
    }

    return best_value_;
  }

private:
  const unsigned int MAX_BITS_{ 32 };
  std::unordered_map<unsigned int, unsigned int> count_map_;
  unsigned int max_flags_;
};

TEST(Dashboard, bitmask_top_one) {
  const size_t MAX_SIZE = 3;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("bitmask_top_one", std::make_shared<TimeSeriesTopFreqBitmask>(MAX_SIZE, 1));

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(0x00000001 << 5, 1);
  ts->addSample(0x00000001 << 5, 2);
  ts->addSample(0x00000001 << 3, 3);

  EXPECT_EQ(0x00000001 << 5, ts->getBestValue());
}

TEST(Dashboard, bitmask_top_two) {
  const size_t MAX_SIZE = 6;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("bitmask_top_one", std::make_shared<TimeSeriesTopFreqBitmask>(MAX_SIZE, 2));

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(0x00000001 << 5, 1);
  ts->addSample(0x00000001 << 5, 2);
  ts->addSample(0x00000001 << 3, 3);
  ts->addSample(0x00000001 << 3, 4);
  ts->addSample(0x00000001 << 0, 5);
  ts->addSample(0x00000001 << 1, 6);

  EXPECT_EQ((0x00000001 << 5) | (0x00000001 << 3), ts->getBestValue()make_shared
}
*/

///  POSE TIMESERIE EXAMPLE

class TimeSeriesInt : public TimeSeries2<int> {
public:
  const size_t MAX_SIZE{ 10 };
  TimeSeriesInt() : TimeSeries2<int>(MAX_SIZE) {}
  //void updateRemovingOldestValue() override {};
  //void updateAddingNewestValue() override {};
};

TEST(Dashboard, POSE) {

  try {
    //std::shared_ptr<TimeSeriesPose> timeseries = std::make_shared<TimeSeriesPose>();
    //auto timeseries = std::make_shared<TimeSeriesInt>();

    TimeSeries2<int> ts1(10);
    std::cout << "- - - - - - - - - - - - " << std::endl;
    TimeSeriesInt ts2 = TimeSeriesInt();

  }
  catch (const std::exception& e) {
    std::cout << "* * * * * * * * * * * " << e.what() << std::endl;
  }

  Dashboard dashboard;
  //dashboard.registerTimeSeries<TimeSeriesPose>();
  dashboard.addSample(Pose(), 1);
  dashboard.addSample(Pose(), 2);
  dashboard.addSample(Pose(), 3);



  std::shared_ptr<TimeSeriesPose> ts_pose = dashboard.getTimeSeries<TimeSeriesPose>();

 

  EXPECT_EQ(0.3f, ts_pose->getAgressionConfidence());
  EXPECT_EQ(0.6f, ts_pose->getTripAndFallConfidence());
}
/////

//TEST(TimeSeries, average_integer) {
//  TimeSeries<int> timeseries(MAX_SIZE);
//  
//  timeseries.addSample(Sample(100, 1));
//  timeseries.addSample(Sample(200, 2));
//  timeseries.addSample(Sample(300, 3));
//
//  EXPECT_EQ(timeseries.averageValue(), 200);
//}

//TEST(TimeSeries, average_double) {
//  TimeSeries<double> timeseries(MAX_SIZE);
//
//  timeseries.addSample(Sample(100.0, 1));
//  timeseries.addSample(Sample(200.0, 2));
//  timeseries.addSample(Sample(300.0, 3));
//
//  EXPECT_EQ(timeseries.averageValue(), 200.0);
//}

//TEST(TimeSeries, frequencies) {
//
//  TimeSeries<int> timeseries(MAX_SIZE);
//
//  timeseries.addSample(Sample(1, 1));
//  timeseries.addSample(Sample(2, 2));
//  timeseries.addSample(Sample(2, 3));
//  timeseries.addSample(Sample(3, 4));
//  timeseries.addSample(Sample(3, 5));
//  timeseries.addSample(Sample(3, 6));
//
//  std::vector<std::pair<int, unsigned int>> freq = timeseries.frequencies();
//
//  EXPECT_EQ(freq[0].first, 3);
//  EXPECT_EQ(freq[0].second, 3);
//  EXPECT_EQ(freq[1].first, 2);
//  EXPECT_EQ(freq[1].second, 2);
//  EXPECT_EQ(freq[2].first, 1);
//  EXPECT_EQ(freq[2].second, 1);
//}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();
}
