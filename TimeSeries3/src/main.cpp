#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>


#include "gtest/gtest.h"

#include "timeseries/timeseries.hpp"
#include "timeseries/timeseries_pose.hpp"
#include "timeseries/dashboard.hpp"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}


///  POSE TIMESERIE EXAMPLE
TEST(Dashboard, POSE) {

  Dashboard dashboard;

  dashboard.addSample<TsPose>(1, Pose());
  dashboard.addSample<TsPose>(2, Pose());
  dashboard.addSample<TsPose>(3, Pose());

  std::shared_ptr<TsPose> ts_pose = dashboard.getTimeSeries<TsPose>();
  EXPECT_EQ(0.3f, ts_pose->getAgressionConfidence());
  EXPECT_EQ(0.6f, ts_pose->getTripAndFallConfidence());

}
/////
TEST(Dashboard, COLOUR) {

  Dashboard dashboard;

  dashboard.addSample<TsColour>(1, (0x00000001 << 1));
  dashboard.addSample<TsColourTop>(1, (0x00000001 << 2));

  dashboard.addSample<TsColour>(2, (0x00000001 << 2));
  dashboard.addSample<TsColourTop>(2, (0x00000001 << 5));

  dashboard.addSample<TsColour>(3, (0x00000001 << 2));
  dashboard.addSample<TsColourTop>(3, (0x00000001 << 5));

  dashboard.addSample<TsColourTop>(4, (0x00000001 << 3));

  dashboard.addSample<TsColourTop>(5, (0x00000001 << 2));

  EXPECT_EQ((0x00000001 << 1), dashboard.oldestValue<TsColour>());
  EXPECT_EQ((0x00000001 << 2), dashboard.newestValue<TsColour>());
  EXPECT_EQ(0x00000001 << 2, dashboard.getBestValue<TsColour>());
  EXPECT_EQ((0x00000001 << 2) | (0x00000001 << 5), dashboard.getBestValue<TsColourTop>());

  //there is no ColoutTop data
  EXPECT_THROW(dashboard.newestValue<TsColourBottom>(), std::logic_error);
}

TEST(Dashboard, HUMAN_FACE_ATTRIBUTES) {
  Dashboard dashboard;


  dashboard.addSample<TsHumanAttr>(1, std::vector<float>(42, 0.1f));
  dashboard.addSample<TsFaceAttr>(1, std::vector<float>(18, 0.2f));

  dashboard.addSample<TsHumanAttr>(2, std::vector<float>(42, 0.9f));
  dashboard.addSample<TsFaceAttr>(2, std::vector<float>(18, 0.8f));

  float humman_attr_0 = dashboard.getBestValue<TsHumanAttr>()[0];
  float humman_face_0 = dashboard.getBestValue<TsFaceAttr>()[0];

  EXPECT_EQ(1.f, humman_attr_0);
  EXPECT_EQ(1.f, humman_face_0);}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();

  return 0;
}
