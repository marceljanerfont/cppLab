#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>


#include "gtest/gtest.h"

#include "timeseries/timeseries.hpp"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}

//typedef DataT<int> Int_t;
//class Int : public DataT<int> {
//public:
//  Int(const int& value) : DataT(value) {}
//};


class Int : public IData {
public:
  Int(const int& value) : IData(), value_(value) {}
  virtual void clear() override {};
  int value_;
};


TEST(TimeSeries, push_data) {
   
  TimeSeries timeseries;

  timeseries.pushData(1, std::make_shared<Int>(100));
  timeseries.pushData(1, std::make_shared<Int>(200));

  auto t1_data = timeseries.getData<Int>(1);
  EXPECT_EQ(2, t1_data.size());
  EXPECT_EQ(100, t1_data[0]->value_);
  EXPECT_EQ(200, t1_data[1]->value_);
}


struct Joint {
  int x_;
  int y_;
};

class Pose: public IData {
public:
  static const unsigned int JOINT_TOTAL{ 14 };
  Joint joints_[JOINT_TOTAL];
};

class Colour : public IData {
public:
  Colour(unsigned int colour): IData(), colour_(colour) {}
  unsigned int colour_;
};


///  POSE TIMESERIE EXAMPLE
TEST(TimeSeries, Creation) {

  TimeSeries timeseries;

  timeseries.pushData(1, std::make_shared<Pose>());
  timeseries.pushData(1, std::make_shared<Colour>(32));




  //EXPECT_EQ(0.3f, dashboard.getTimeSeries<TsPose>()->getAgressionConfidence());
  //EXPECT_EQ(0.6f, dashboard.getTimeSeries<TsPose>()->getTripAndFallConfidence());

}



int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();

  return 0;
}
