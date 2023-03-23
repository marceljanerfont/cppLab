#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>


#include "gtest/gtest.h"

#include "TimeSeries.h"
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

TEST(TimeSeries, out_of_range) {
  const size_t MAX_SIZE = 13;
  int NB_SAMPLES = 2370;
  TimeSeries<int, MAX_SIZE> timeseries;

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
  TimeSeries<int, MAX_SIZE> timeseries;

  EXPECT_EQ(timeseries.capacity(), MAX_SIZE);

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(Sample(i, i));
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


  TimeSeries<int, MAX_SIZE> timeseries;

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(Sample(i, i));
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


  TimeSeries<int, MAX_SIZE> timeseries;

  for (int i = 1; i <= NB_SAMPLES; ++i) {
    timeseries.addSample(Sample(i, i));
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
