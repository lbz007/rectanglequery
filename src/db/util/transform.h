/* @file  transform.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#ifndef EDI_UTIL_TRANSFROM_H_
#define EDI_UTIL_TRANSFROM_H_

#include "util/util.h"
#include "db/util/box.h"
namespace open_edi {
namespace db {
class Inst;
class Pin;
class Port;

bool transformByInst(const Inst *inst, Box &box);
bool transformByInst(const Inst *inst, Point &pt);

bool transformByIOPin(const Pin *pin, Box &box);
bool transformByIOPin(const Pin *pin, Point &pt);
bool reverseTransformByIOPin(const Pin *pin, Box &box);
bool reverseTransformByIOPin(const Pin *pin, Point &pt);

bool transformPlacedIOPin(const Pin *pin);
class Transform {
  public:
    Transform() : offset_(0, 0), orient_(Orient::kN) {}
    ~Transform() {}

    Transform(const Inst *inst);
    Transform(const Pin *pin, const Port *port);

    void transform(Point& pt);
    void transform(Box& box);
    void transform(Point& pt, Orient orient, Point& origin, int size_x, int size_y);
    void transform(Box& box, Orient orient, Point& origin, int size_x, int size_y);
    void reverseTransform(Point& pt);
    void reverseTransform(Box& box);

  private:
    void __setOffset(Point &placed_pt, Point &origin, int size_x, int size_y);
    void __setOffset(Point &placed_pt, Point &origin);
    void __adjustBoxPoints(Point &ll, Point &ur, Box &box);

    Point  offset_;
    Orient orient_;
    Point  origin_{0, 0};
    int    size_x_{0};
    int    size_y_{0};
};

}  // namespace db
}  // namespace open_edi

#endif  // EDI_UTIL_TRANSFROM_H_
