/****************************************************************************************\
*                               Polygonal Approximation                                  *
\****************************************************************************************/

/* Ramer-Douglas-Peucker algorithm for polygon simplification */
/* https://github.com/opencv/opencv/blob/3.4.15/modules/imgproc/src/approx.cpp */


#include "geometry_utils.h"

#include <cassert>
#include <math.h>

#include "memory_utils.h"


static int approxPolyDP_(const gu::Point *src_contour, int count0, gu::Point *dst_contour,
                         bool is_closed0, double eps, mu::AutoBuffer<gu::Range> &_stack) {


#define PUSH_SLICE(slice) \
      if( top >= stacksz ) \
      { \
          _stack.resize(stacksz*3/2); \
          stack = _stack.data(); \
          stacksz = _stack.size(); \
      } \
      stack[top++] = slice

#define READ_PT(pt, pos) \
      pt = src_contour[pos]; \
      if( ++pos >= count ) pos = 0

#define READ_DST_PT(pt, pos) \
      pt = dst_contour[pos]; \
      if( ++pos >= count ) pos = 0

#define WRITE_PT(pt) \
      dst_contour[new_count++] = pt

  int             init_iters = 3;
  gu::Range       slice(0, 0), right_slice(0, 0);
  gu::Point       start_pt(/*-1000000, -1000000*/0, 0), end_pt(0, 0), pt(0, 0);
  int             i = 0, j, pos = 0, wpos, count = count0, new_count = 0;
  int             is_closed = is_closed0;
  bool            le_eps = false;
  size_t top = 0, stacksz = _stack.size();
  gu::Range       *stack = _stack.data();

  if (count == 0)
    return 0;

  eps *= eps;

  if (!is_closed) {
    right_slice.start = count;
    end_pt = src_contour[0];
    start_pt = src_contour[count - 1];

    if (start_pt.x != end_pt.x || start_pt.y != end_pt.y) {
      slice.start = 0;
      slice.end = count - 1;
      PUSH_SLICE(slice);
    } else {
      is_closed = 1;
      init_iters = 1;
    }
  }

  if (is_closed) {
    // 1. Find approximately two farthest points of the contour
    right_slice.start = 0;

    for (i = 0; i < init_iters; i++) {
      double dist, max_dist = 0;
      pos = (pos + right_slice.start) % count;
      READ_PT(start_pt, pos);

      for (j = 1; j < count; j++) {
        double dx, dy;

        READ_PT(pt, pos);
        dx = pt.x - start_pt.x;
        dy = pt.y - start_pt.y;

        dist = dx * dx + dy * dy;

        if (dist > max_dist) {
          max_dist = dist;
          right_slice.start = j;
        }
      }

      le_eps = max_dist <= eps;
    }

    // 2. initialize the stack
    if (!le_eps) {
      right_slice.end = slice.start = pos % count;
      slice.end = right_slice.start = (right_slice.start + slice.start) % count;

      PUSH_SLICE(right_slice);
      PUSH_SLICE(slice);
    } else
      WRITE_PT(start_pt);
  }

  // 3. run recursive process
  while (top > 0) {
    slice = stack[--top];
    end_pt = src_contour[slice.end];
    pos = slice.start;
    READ_PT(start_pt, pos);

    if (pos != slice.end) {
      double dx, dy, dist, max_dist = 0;

      dx = end_pt.x - start_pt.x;
      dy = end_pt.y - start_pt.y;

      assert(dx != 0 || dy != 0);

      while (pos != slice.end) {
        READ_PT(pt, pos);
        dist = fabs((pt.y - start_pt.y) * dx - (pt.x - start_pt.x) * dy);

        if (dist > max_dist) {
          max_dist = dist;
          right_slice.start = (pos + count - 1) % count;
        }
      }

      le_eps = max_dist * max_dist <= eps * (dx * dx + dy * dy);
    } else {
      le_eps = true;
      // read starting point
      start_pt = src_contour[slice.start];
    }

    if (le_eps) {
      WRITE_PT(start_pt);
    } else {
      right_slice.end = slice.end;
      slice.end = right_slice.start;
      PUSH_SLICE(right_slice);
      PUSH_SLICE(slice);
    }
  }

  if (!is_closed)
    WRITE_PT(src_contour[count - 1]);

  // last stage: do final clean-up of the approximated contour -
  // remove extra points on the [almost] straight lines.
  is_closed = is_closed0;
  count = new_count;
  pos = is_closed ? count - 1 : 0;
  READ_DST_PT(start_pt, pos);
  wpos = pos;
  READ_DST_PT(pt, pos);

  for (i = !is_closed; i < count - !is_closed && new_count > 2; i++) {
    double dx, dy, dist, successive_inner_product;
    READ_DST_PT(end_pt, pos);

    dx = end_pt.x - start_pt.x;
    dy = end_pt.y - start_pt.y;
    dist = fabs((pt.x - start_pt.x) * dy - (pt.y - start_pt.y) * dx);
    successive_inner_product = (pt.x - start_pt.x) * (end_pt.x - pt.x) +
                               (pt.y - start_pt.y) * (end_pt.y - pt.y);

    if (dist * dist <= 0.5 * eps * (dx * dx + dy * dy) && dx != 0 && dy != 0 &&
        successive_inner_product >= 0) {
      new_count--;
      dst_contour[wpos] = start_pt = end_pt;
      if (++wpos >= count) wpos = 0;
      READ_DST_PT(pt, pos);
      i++;
      continue;
    }
    dst_contour[wpos] = start_pt = pt;
    if (++wpos >= count) wpos = 0;
    pt = end_pt;
  }

  if (!is_closed)
    dst_contour[wpos] = pt;

  return new_count;
}

void gu::approxPolyDP(const gu::Point *src_contour, const int &src_count, gu::Point *dst_contour, int *dst_count, const double &epsilon) {
  // prevent unreasonable error values (Douglas-Peucker algorithm) from being used.
  if (epsilon < 0.0 || !(epsilon < 1e30)) {
    assert(false && "Epsilon not valid.");
  }

  mu::AutoBuffer<gu::Range> _stack(src_count);
  *dst_count = approxPolyDP_(src_contour, src_count, dst_contour, false, epsilon, _stack);
}
