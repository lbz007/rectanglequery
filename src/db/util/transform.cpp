/* @file  transform.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */
#include "db/util/transform.h"
#include "db/core/inst.h"
#include "db/core/cell.h"
#include "db/core/term.h"

namespace open_edi {
namespace db {

bool transformByInst(const Inst *inst, Point &pt) {
    ediAssert(inst != nullptr);
    Transform transform(inst);
    transform.transform(pt);
    return true;
}

bool transformByInst(const Inst *inst, Box &box) {
    ediAssert(inst != nullptr);
    Transform transform(inst);
    transform.transform(box);
    return true;
}

bool transformByIOPin(const Pin *io_pin, Point &pt) {
    if (io_pin == nullptr) {
        return false;
    }
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    //For IO pin, inst should be nullptr.
    if (term == nullptr || inst != nullptr) {
        return false;
    }
    Port *io_port = term->getPort(0);

    Transform transform(io_pin, io_port);
    transform.transform(pt);
    return true;
}

bool reverseTransformByIOPin(const Pin *io_pin, Point &pt) {
    if (io_pin == nullptr) {
        return false;
    }
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    //For IO pin, inst should be nullptr.
    if (term == nullptr || inst != nullptr) {
        return false;
    }
    Port *io_port = term->getPort(0);

    Transform transform(io_pin, io_port);
    transform.reverseTransform(pt);
    return true;
}

// For read_def:
// transform from original coordinates to top cell's coordinates.
bool transformByIOPin(const Pin *io_pin, Box &box) {
    if (io_pin == nullptr) {
        return false;
    }
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    //For IO pin, inst should be nullboxr.
    if (term == nullptr || inst != nullptr) {
        return false;
    }
    Port *io_port = term->getPort(0);
    Transform transform(io_pin, io_port);
    transform.transform(box);
    return true;
}

// For write_def:
// transform from top cell's coordinates to original coordinates
bool reverseTransformByIOPin(const Pin *io_pin, Box &box) {
    if (io_pin == nullptr) {
        return false;
    }
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    //For IO pin, inst should be nullboxr.
    if (term == nullptr || inst != nullptr) {
        return false;
    }
    Port *io_port = term->getPort(0);
    Transform transform(io_pin, io_port);
    transform.reverseTransform(box);
    return true;
}

static void transformPolygon(
    Transform &transform, Polygon *orig_poly, Polygon **new_poly
) {
    ediAssert(orig_poly != nullptr);
    for (int i = 0; i < orig_poly->getNumPoints(); ++i) {
        Point pt = orig_poly->getPoint(i);
        transform.transform(pt);
        if (i == 0) {
            *new_poly = new Polygon;
        }
        Point *new_pt = new Point;
        new_pt->setX(pt.getX());
        new_pt->setY(pt.getY());
        (*new_poly)->addPoint(new_pt);
    }
}

bool transformPlacedIOPin(const Pin *io_pin) {
    if (io_pin == nullptr) {
        return false;
    }
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    if (term == nullptr || inst != nullptr) {
        return false;
    }

    for (int i = 0; i < term->getPortNum(); ++i) {
        Port *io_port = term->getPort(i);
        if (!io_port || io_port->getStatus() == PlaceStatus::kUnplaced) {
            continue;
        }
        Transform transform(io_pin, io_port);
        Port::shapeIter iter(io_port);
        Geometry *geom = nullptr;
        while ((geom = iter.getNext()) != nullptr) {
            if (geom->isPath() || geom->isPolygon()) {
                Polygon *poly = geom->getPolygon();
                Polygon *new_poly;
                transformPolygon(transform, poly, &new_poly);
                geom->setPolygon(new_poly);
            } else if (geom->isRect()) {
                Box rect = geom->getBox();
                transform.transform(rect);
                geom->setBox(&rect);
            } else if (geom->isPoint()) {
                Point pt = geom->getPoint();
                transform.transform(pt);
                geom->setPoint(&pt);
            } else {
                continue;
            }
        }
    }
    return true;
}


Transform::Transform(const Inst* inst) {
    ediAssert(inst != nullptr);
    Point pt = inst->getLocation();
    Orient ort = inst->getOrient();
    Cell* cell = inst->getMaster();
    ediAssert(cell != nullptr);

    int size_x = cell->getSizeX();
    int size_y = cell->getSizeY();
    Point origin;
    origin.setX(cell->getOriginX());
    origin.setY(cell->getOriginY());

    orient_ = ort;
    __setOffset(pt, origin, size_x, size_y);
}

Transform::Transform(const Pin *io_pin, const Port *io_port) {
    ediAssert(io_pin != nullptr && io_port != nullptr);
    Inst *inst = io_pin->getInst();
    Term *term = io_pin->getTerm();
    //For IO pin, inst should be nullptr.
    ediAssert(term != nullptr && inst == nullptr);
    if (const_cast<Port *>(io_port)->getTerm()->getId() != term->getId()) {
        return;
    }

    Point placed_pt = {0, 0};
    Orient ort = Orient::kN;
    Point origin = {0, 0};

    if (io_port->getHasPlacement() ||
        io_port->getStatus() != PlaceStatus::kUnplaced) {
        placed_pt = io_port->getLocation();
        ort = io_port->getOrient();
    }
    
    orient_ = ort;
    // Based on lef/def_5.8 manual,
    // the origin is using {0 0}:
    __setOffset(placed_pt, origin);
}

// For the offset on the IO pin:
void Transform::__setOffset(
    Point &placed_pt, Point &origin
) {
    offset_.setX(placed_pt.getX() - origin.getX());
    offset_.setY(placed_pt.getY() - origin.getY());
}

// For the offset inside Inst's bounding-box
void Transform::__setOffset(
    Point &placed_pt, Point &origin,
    int size_x, int size_y
) {
    switch (orient_) {
    case Orient::kN:
        offset_.setX(placed_pt.getX() - origin.getX());
        offset_.setY(placed_pt.getY() - origin.getY());
        break;
    case Orient::kW:
        offset_.setX(placed_pt.getX() + origin.getY() + size_y);
        offset_.setY(placed_pt.getY() - origin.getX());
        break;
    case Orient::kS:
        offset_.setX(placed_pt.getX() + origin.getX() + size_x);
        offset_.setY(placed_pt.getY() + origin.getY() + size_y);
        break;
    case Orient::kE:
        offset_.setX(placed_pt.getX() - origin.getY());
        offset_.setY(placed_pt.getY() + origin.getX() + size_x);
        break;
    case Orient::kFN:
        offset_.setX(placed_pt.getX() + origin.getX() + size_x);
        offset_.setY(placed_pt.getY() - origin.getY());
        break;
    case Orient::kFE:
        offset_.setX(placed_pt.getX() + origin.getY() + size_y);
        offset_.setY(placed_pt.getY() + origin.getX() + size_x);
        break;
    case Orient::kFS:
        offset_.setX(placed_pt.getX() - origin.getX());
        offset_.setY(placed_pt.getY() + origin.getY() + size_y);
        break;
    case Orient::kFW:
        offset_.setX(placed_pt.getX() - origin.getY());
        offset_.setY(placed_pt.getY() - origin.getX());
        break;
    }
}

void Transform::reverseTransform(Point &pt) {
    Point result_pt;
    switch (orient_) {
    case Orient::kN:
        result_pt.setX(pt.getX() - offset_.getX());
        result_pt.setY(pt.getY() - offset_.getY());
        break;
    case Orient::kW:
        result_pt.setX(pt.getX() - offset_.getY());
        result_pt.setY(offset_.getX() - pt.getX());
        break;
    case Orient::kS:
        result_pt.setX(offset_.getX() - pt.getX());
        result_pt.setY(offset_.getY() - pt.getY());
        break;
    case Orient::kE:
        result_pt.setX(offset_.getY() - pt.getY());
        result_pt.setY(pt.getX() - offset_.getX());
        break;
    case Orient::kFN:
        result_pt.setX(offset_.getX() - pt.getX());
        result_pt.setY(pt.getY() - offset_.getY());
        break;
    case Orient::kFE:
        result_pt.setX(offset_.getY() - pt.getY());
        result_pt.setY(offset_.getX() - pt.getX());
        break;
    case Orient::kFS:
        result_pt.setX(pt.getX() - offset_.getX());
        result_pt.setY(offset_.getY() - pt.getY());
        break;
    case Orient::kFW:
        result_pt.setX(offset_.getY() - pt.getY());
        result_pt.setY(offset_.getX() - pt.getX());
        break;
    }
    pt = result_pt;
}

void Transform::transform(Point& pt) {
    Point result_pt;
    switch (orient_) {
    case Orient::kN:
        result_pt.setX(offset_.getX() + pt.getX());
        result_pt.setY(offset_.getY() + pt.getY());
        break;
    case Orient::kW:
        result_pt.setX(offset_.getX() - pt.getY());
        result_pt.setY(offset_.getY() + pt.getX());
        break;
    case Orient::kS:
        result_pt.setX(offset_.getX() - pt.getX());
        result_pt.setY(offset_.getY() - pt.getY());
        break;
    case Orient::kE:
        result_pt.setX(offset_.getX() + pt.getY());
        result_pt.setY(offset_.getY() - pt.getX());
        break;
    case Orient::kFN:
        result_pt.setX(offset_.getX() - pt.getX());
        result_pt.setY(offset_.getY() + pt.getY());
        break;
    case Orient::kFE:
        result_pt.setX(offset_.getX() - pt.getY());
        result_pt.setY(offset_.getY() - pt.getX());
        break;
    case Orient::kFS:
        result_pt.setX(offset_.getX() + pt.getX());
        result_pt.setY(offset_.getY() - pt.getY());
        break;
    case Orient::kFW:
        result_pt.setX(offset_.getX() + pt.getY());
        result_pt.setY(offset_.getY() + pt.getX());
        break;
    }
    pt = result_pt;
}

void Transform::__adjustBoxPoints(Point &ll, Point &ur, Box &box) {
    if (ll.getX() < ur.getX()) {
        box.setLLX(ll.getX());
        box.setURX(ur.getX());
    } else {
        box.setLLX(ur.getX());
        box.setURX(ll.getX());
    }
    if (ll.getY() < ur.getY()) {
        box.setLLY(ll.getY());
        box.setURY(ur.getY());
    } else {
        box.setLLY(ur.getY());
        box.setURY(ll.getY());
    }
}

void Transform::transform(Box& box) {
    Point ll(box.getLLX(), box.getLLY());
    Point ur(box.getURX(), box.getURY());

    transform(ll);
    transform(ur);
    __adjustBoxPoints(ll, ur, box);
}
/**
 * Transform 
 * 
 * @param  {Point} pt      : output point , which is needed to tranform
 * @param  {Orient} orient : orient
 * @param  {Point} origin  : origin
 * @param  {int} size_x    : size_x
 * @param  {int} size_y    : size_y
 */
void Transform::transform(Point& pt, Orient orient, Point& origin, int size_x, int size_y) {
    orient_ = orient;
    __setOffset(pt, origin, size_x, size_y);
    pt = offset_;
}
/**
 * Transform 
 * 
 * @param  {Box} box       : output box , which is needed to tranform
 * @param  {Orient} orient : orient
 * @param  {Point} origin  : origin
 * @param  {int} size_x    : size_x
 * @param  {int} size_y    : size_y
 */
void Transform::transform(Box& box, Orient orient, Point& origin, int size_x, int size_y) {
    Point ll(box.getLLX(), box.getLLY());
    transform(ll, orient, origin, size_x, size_y);
    Point ur(box.getURX(), box.getURY());
    transform(ur, orient, origin, size_x, size_y);
    __adjustBoxPoints(ll, ur, box);
}

void Transform::reverseTransform(Box &box) {
    Point ll(box.getLLX(), box.getLLY());
    Point ur(box.getURX(), box.getURY());

    reverseTransform(ll);
    reverseTransform(ur);
    __adjustBoxPoints(ll, ur, box);
}

}  // namespace db 
}  // namespace open_edi
