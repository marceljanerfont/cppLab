#ifndef GEOMETRY_UTILS_H
#define GEOMETRY_UTILS_H

// computational goemetry utilities
namespace gu {
struct Point {
  Point(): x(0), y(0) {}
  Point(int _x, int _y): x(_x), y(_y) {}

  int x;
  int y;
};

struct Range {
  Range(): start(0), end(0) {};
  Range(int _start, int _end): start(_start), end(_end) {}

  int start;
  int end;
};

/* Ramer-Douglas-Peucker algorithm for polygon simplification */
// dst_contour should be already allocated
void approxPolyDP(const Point *src_contour, const int &src_count, Point *dst_contour, int &dst_count, const double &epsilon);
}
#endif // GEOMETRY_UTILS_H
