#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>


#include "gtest/gtest.h"

//#include "TimeSeries.h"
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

TEST(Dashboard, bitmask_top_one) {
  const size_t MAX_SIZE = 3;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("key1", MAX_SIZE, std::make_unique<TopFrequencyBitmask>(1));

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(0x00000001 << 5, 1);
  ts->addSample(0x00000001 << 5, 2);
  ts->addSample(0x00000001 << 3, 3);

  EXPECT_EQ(0x00000001 << 5, ts->getBestValue());
}

TEST(Dashboard, bitmask_top_two) {
  const size_t MAX_SIZE = 6;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<unsigned int, unsigned int>("key1", MAX_SIZE, std::make_unique<TopFrequencyBitmask>(2));

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(0x00000001 << 5, 1);
  ts->addSample(0x00000001 << 5, 2);
  ts->addSample(0x00000001 << 3, 3);
  ts->addSample(0x00000001 << 3, 4);
  ts->addSample(0x00000001 << 0, 5);
  ts->addSample(0x00000001 << 1, 6);

  EXPECT_EQ((0x00000001 << 5) | (0x00000001 << 3), ts->getBestValue());
}


///  POSE TIMESERIE EXAMPLE
class PoseAlgorithm : public BestAlgorithm<Pose, Pose> {
public:
  void clear() override {}
  void removeOldValue(const Pose& value) override {
  }
  void addNewValue(const Pose& value) override { 
    // update aggression confidence
    agrression_confidence_ += 0.1f;

    // update aggression confidence
    trip_and_fall_confidence_ += 0.2f;
  }
  Pose getBestValue() const override {
    return best_value_;
  }
  float getAgressionConfidence() {
    return agrression_confidence_;
  }
  float getTripAndFallConfidence() {
    return trip_and_fall_confidence_;
  }

  float agrression_confidence_{ 0.f };
  float trip_and_fall_confidence_{ 0.f };
  Pose best_value_;
};

TEST(Dashboard, POSE) {
  const size_t MAX_SIZE = 3;

  Dashboard dashboard;
  auto ts = dashboard.registerTimeSeries<Pose, Pose>("pose_ts", MAX_SIZE, std::make_unique<PoseAlgorithm>());

  EXPECT_EQ(ts->capacity(), MAX_SIZE);

  ts->addSample(Pose(), 1);
  ts->addSample(Pose(), 2);
  ts->addSample(Pose(), 3);

  PoseAlgorithm* pose_algo = dynamic_cast<PoseAlgorithm*>(ts->getBestValueAlgorithm());



  EXPECT_EQ(0.3f, pose_algo->getAgressionConfidence());
  EXPECT_EQ(0.6f, pose_algo->getTripAndFallConfidence());
}

//////




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
