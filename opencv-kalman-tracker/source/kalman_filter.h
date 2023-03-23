

#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp>

// to predict object position and size
class KalmanFilter {
public:
  KalmanFilter();
  ~KalmanFilter();

  // computes and set dT (seconds in time betwwen steps)
  void doPrediction();

  void getCurrentState(int &x_min, int &y_min, int &x_max, int &y_max);

  void doMeasure(int x_min, int y_min, int x_max, int y_max, bool is_first = false);


private:
  // to compute dT
  int64_t curr_tick_count_ = 0;

  const unsigned int TYPE = CV_32F;
  
  const int STATE_SIZE = 6;
  cv::Mat state_ { STATE_SIZE, 1, TYPE };  // [x, y, v_x, v_y, w, h]
  
  const int MEASURE_SIZE = 4;
  cv::Mat measure_ { MEASURE_SIZE, 1, TYPE }; // [z_x, z_y, z_w, z_h]

  cv::KalmanFilter kf_ { STATE_SIZE, MEASURE_SIZE, 0, TYPE };
};