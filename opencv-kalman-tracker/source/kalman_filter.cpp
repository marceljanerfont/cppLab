#include "kalman_filter.h"

KalmanFilter::KalmanFilter() {
  // Transition State Matrix A
  // Note: set dT (seconds between steps) at each processing step!
  // [ 1 0 dT 0  0 0 ]
  // [ 0 1 0  dT 0 0 ]
  // [ 0 0 1  0  0 0 ]
  // [ 0 0 0  1  0 0 ]
  // [ 0 0 0  0  1 0 ]
  // [ 0 0 0  0  0 1 ]
  cv::setIdentity(kf_.transitionMatrix);
  kf_.transitionMatrix.at<float>(2) = 1.f;
  kf_.transitionMatrix.at<float>(9) = 1.f;

  // Measure Matrix H
  // [ 1 0 0 0 0 0 ]
  // [ 0 1 0 0 0 0 ]
  // [ 0 0 0 0 1 0 ]
  // [ 0 0 0 0 0 1 ]
  kf_.measurementMatrix = cv::Mat::zeros(MEASURE_SIZE, STATE_SIZE, TYPE);
  kf_.measurementMatrix.at<float>(0) = 1.0f;
  kf_.measurementMatrix.at<float>(7) = 1.0f;
  kf_.measurementMatrix.at<float>(16) = 1.0f;
  kf_.measurementMatrix.at<float>(23) = 1.0f;

  // Process Noise Covariance Matrix Q
  // [ Ex   0   0     0     0    0  ]
  // [ 0    Ey  0     0     0    0  ]
  // [ 0    0   Ev_x  0     0    0  ]
  // [ 0    0   0     Ev_y  0    0  ]
  // [ 0    0   0     0     Ew   0  ]
  // [ 0    0   0     0     0    Eh ]
  //cv::setIdentity(kf.processNoiseCov, cv::Scalar(1e-2));
  kf_.processNoiseCov.at<float>(0) = 1e-2;
  kf_.processNoiseCov.at<float>(7) = 1e-2;
  kf_.processNoiseCov.at<float>(14) = 5.0f;
  kf_.processNoiseCov.at<float>(21) = 5.0f;
  kf_.processNoiseCov.at<float>(28) = 1e-2;
  kf_.processNoiseCov.at<float>(35) = 1e-2;

  // Measures Noise Covariance Matrix R
  cv::setIdentity(kf_.measurementNoiseCov, cv::Scalar(1e-1));
  // <<<< Kalman Filter
}

KalmanFilter::~KalmanFilter() {
}

void KalmanFilter::doPrediction() {
  // compute dT since last
  int64_t last_tick_count = curr_tick_count_;
  curr_tick_count_ = cv::getTickCount();
  double dT = static_cast<double>(curr_tick_count_ - last_tick_count) / cv::getTickFrequency(); //seconds

  // >>>> Matrix A
  kf_.transitionMatrix.at<float>(2) = dT;
  kf_.transitionMatrix.at<float>(9) = dT;
  // <<<< Matrix A

  state_ = kf_.predict();
}

void KalmanFilter::getCurrentState(int& x_min, int& y_min, int& x_max, int& y_max) {
  // [x, y, v_x, v_y, w, h]
  int half_width = static_cast<int>(state_.at<float>(4) / 2.f);
  int half_height = static_cast<int>(state_.at<float>(5) / 2.f);
  
  x_min = static_cast<int>(state_.at<float>(0)) - half_width;
  y_min = static_cast<int>(state_.at<float>(1)) - half_height;

  x_max = static_cast<int>(state_.at<float>(0)) + half_width;
  y_max = static_cast<int>(state_.at<float>(1)) + half_height;
}

void KalmanFilter::doMeasure(int x_min, int y_min, int x_max, int y_max, bool is_first) {
  // [z_x, z_y, z_w, z_h]
  float width = static_cast<float>(x_max - x_min);
  float height = static_cast<float>(y_max - y_min);

  measure_.at<float>(0) = static_cast<float>(x_min) + width / 2.f;
  measure_.at<float>(1) = static_cast<float>(y_min) + height / 2.f;
  measure_.at<float>(2) = width;
  measure_.at<float>(3) = height;

  if (is_first) {
    // >>>> Initialization
    kf_.errorCovPre.at<float>(0) = 1.f; // px
    kf_.errorCovPre.at<float>(7) = 1.f; // px
    kf_.errorCovPre.at<float>(14) = 1.f;
    kf_.errorCovPre.at<float>(21) = 1.f;
    kf_.errorCovPre.at<float>(28) = 1.f; // px
    kf_.errorCovPre.at<float>(35) = 1.f; // px

    // [x, y, v_x, v_y, w, h]
    state_.at<float>(0) = measure_.at<float>(0);
    state_.at<float>(1) = measure_.at<float>(1);
    state_.at<float>(2) = 0;
    state_.at<float>(3) = 0;
    state_.at<float>(4) = measure_.at<float>(2);
    state_.at<float>(5) = measure_.at<float>(3);
    // <<<< Initialization

    kf_.statePost = state_;
  }
  else {
    kf_.correct(measure_);
  }
  
}