/**
 * @file timing_constraints.h
 * @date 2020-11-25
 * @brief
 *
 * Copyright (C) 2020 NIIC EDA
 *
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 *
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef EDI_DB_TIMING_SDC_TIMING_CONSTRAINTS_H_
#define EDI_DB_TIMING_SDC_TIMING_CONSTRAINTS_H_

#include <string>
#include <vector>
#include <memory>
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/unordered_multiset_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>

#include "db/timing/sdc/sdc_common.h"
#include "db/timing/sdc/command_get_set_property.h"
#include "db/timing/sdc/base_container_data.h"

namespace open_edi {
namespace db {

class CreateClock {
  public:
    CreateClock();

  private:
    std::string comment_ = "";

  public:
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_FLAG(add, Add)
};
using CreateClockPtr = std::shared_ptr<CreateClock>;

class CreateGeneratedClock {
  public:
    CreateGeneratedClock();

    void addEdgeShift(const float& edge_shift) { edge_shifts_.emplace_back(edge_shift); }
    void addEdge(const int edge) { edges_.emplace_back(edge); }
    bool addSourceMasterPin(const std::string &pin_name) { return ContainerDataAccess::addToPinSet(source_master_pins_, pin_name); }

  private:
    std::string comment_ = "";
    std::unordered_set<ObjectId> source_master_pins_;
    std::vector<float> edge_shifts_;
    std::vector<int> edges_;
    float duty_cycle_ = 50;
    ClockId master_clock_;
    int divided_by_ = 1;
    int multiply_by_ = 1;

  public:
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_VAR(edge_shifts, EdgeShifts)
    COMMAND_GET_SET_VAR(source_master_pins, SourceMasterPins)
    COMMAND_GET_SET_VAR(edges, Edges)
    COMMAND_GET_SET_VAR(duty_cycle, DutyCycle)
    COMMAND_GET_SET_VAR(master_clock, MasterClock)
    COMMAND_GET_SET_VAR(divided_by, DividedBy)
    COMMAND_GET_SET_VAR(multiply_by, MultiplyBy)
    COMMAND_GET_SET_FLAG(invert, Invert)
    COMMAND_GET_SET_FLAG(combinational, Combinational)
    COMMAND_GET_SET_FLAG(add, Add)
};
using CreateGeneratedClockPtr = std::shared_ptr<CreateGeneratedClock>;

using PinClockMap = boost::bimaps::bimap <
    boost::bimaps::unordered_multiset_of<ObjectId>,
    boost::bimaps::unordered_multiset_of<ClockId>,
    boost::bimaps::unordered_set_of_relation<>
>;
class ClockContainerData : public ClockBaseContainerData {
  public:
    const size_t getClockNum() const { return name_to_id_.size(); }
    void updateClock(ClockPtr &clock, CreateClockPtr &create_clock, const ClockId &clock_id);
    void updateClock(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock, const ClockId &clock_id);
    void addClock(ClockPtr &clock, CreateClockPtr &create_clock);
    void addClock(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock);
    bool addClockPin(const ObjectId &pin_id, ClockPtr &clock, const bool is_add);
    void removeClockPin(const ObjectId &pin_id);
    void removeClock(const ClockId &clock_id, bool update_sdc = true);
    void addVirtualClock(ClockPtr &clock, CreateClockPtr &create_clock);
    void addVirtualClock(ClockPtr &clock, CreateGeneratedClockPtr &generated_create_clock);
    void calcGeneratedClockPeriod(ClockPtr &clock, CreateGeneratedClockPtr &create_generated_clock);

  public:
    static const std::string default_clock_name_;
    static open_edi::util::UInt32 count_;
  
  private:
    std::unordered_map<std::string, ClockId> name_to_id_;
    std::unordered_map<ClockId, ClockPtr> id_to_ptr_;
    std::unordered_map<ClockId, CreateClockPtr> create_clocks_;
    std::unordered_map<ClockId, CreateGeneratedClockPtr> create_generated_clocks_;
    PinClockMap pin_clock_value_;

  public:
    COMMAND_GET_SET_VAR(count, Count)
    COMMAND_GET_SET_VAR(name_to_id, NameToId)
    COMMAND_GET_SET_VAR(id_to_ptr, IdToPtr)
    COMMAND_GET_SET_VAR(create_clocks, CreateClocks)
    COMMAND_GET_SET_VAR(create_generated_clocks, CreateGeneratedClocks)
    COMMAND_GET_SET_VAR(pin_clock_value, PinClockValue)
};
using ClockContainerDataPtr = std::shared_ptr<ClockContainerData>;

class PathNodes {
  public:
    PathNodes();

    bool addPin(const std::string &pin_name) { return ContainerDataAccess::addToPinSet(pins_, pin_name); }
    bool addInst(const std::string &inst_name) { return ContainerDataAccess::addToInstSet(insts_, inst_name); }
    bool addClock(const ClockId &clock_id) { return ContainerDataAccess::addToClockSet(clocks_, clock_id); }
    bool hasObject() { return pins_.size() or insts_.size() or clocks_.size(); }
    void checkFlags();

  private:
    std::unordered_set<ObjectId> pins_;
    std::unordered_set<ObjectId> insts_;
    std::unordered_set<ClockId> clocks_;

  public:
    COMMAND_GET_SET_VAR(pins, Pins)
    COMMAND_GET_SET_VAR(insts, Insts)
    COMMAND_GET_SET_VAR(clocks, Clocks)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
};
using PathNodesPtr = std::shared_ptr<PathNodes>;

class PathThroughNodes {
  public:
    PathThroughNodes();

    bool addPin(const std::string &pin_name) { return ContainerDataAccess::addToPinSet(pins_, pin_name); }
    bool addInst(const std::string &inst_name) { return ContainerDataAccess::addToInstSet(insts_, inst_name); }
    bool addNet(const std::string &net_name) { return ContainerDataAccess::addToNetSet(nets_, net_name); }
    void checkFlags();

  private:
    std::unordered_set<ObjectId> pins_;
    std::unordered_set<ObjectId> insts_;
    std::unordered_set<ObjectId> nets_;
    int index_ = 0;

  public:
  public:
    COMMAND_GET_SET_VAR(pins, Pins)
    COMMAND_GET_SET_VAR(insts, Insts)
    COMMAND_GET_SET_VAR(nets, Nets)
    COMMAND_GET_SET_VAR(index, Index)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
};
using PathThroughNodesPtr = std::shared_ptr<PathThroughNodes>;

class PathThroughNodesPtrLess {
  public:
    bool operator() (const PathThroughNodesPtr &lhs, const PathThroughNodesPtr &rhs) const {
      return lhs->getIndex() < rhs->getIndex();
    }
};

class ExceptionPath {
  public:
    ExceptionPath();

    void addThrough(PathThroughNodesPtr &through) { throughs_.emplace_back(through); }
    void sortThroughs() { std::sort(throughs_.begin(), throughs_.end(), PathThroughNodesPtrLess()); }

  private:
    PathNodesPtr from_;
    PathNodesPtr to_;
    std::vector<PathThroughNodesPtr> throughs_;

  public:
    COMMAND_GET_SET_VAR(from, From)
    COMMAND_GET_SET_VAR(to, To)
    COMMAND_GET_SET_VAR(throughs, Throughs)
};
using ExceptionPathPtr = std::shared_ptr<ExceptionPath>();

class GroupPath : public ExceptionPath {
  public:
    GroupPath();

  private:
    std::string comment_ = "";
    std::string name_ = "";
    float weight_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_VAR(name, Name)
    COMMAND_GET_SET_VAR(weight, Weight)
    COMMAND_GET_SET_FLAG(default_value, DefaultValue)
};
using GroupPathPtr = std::shared_ptr<GroupPath>;

class GroupPathContainerData : public BaseContainerData {
  public:
    void add(const GroupPathPtr &path) { group_paths_.emplace(path); }

  private:
    std::unordered_set<GroupPathPtr> group_paths_;

  public:
    COMMAND_GET_SET_VAR(group_paths, GroupPaths)
};
using GroupPathContainerDataPtr = std::shared_ptr<GroupPathContainerData>;

class SetClockGatingCheck {
  public:
    SetClockGatingCheck();
    bool operator==(const SetClockGatingCheck &rhs) const;

  private:
    float setup_ = 0.0;
    float hold_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(setup, Setup)
    COMMAND_GET_SET_VAR(hold, Hold)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(high, High)
    COMMAND_GET_SET_FLAG(low, Low)
};
using SetClockGatingCheckPtr = std::shared_ptr<SetClockGatingCheck>;

class ClockGatingCheckContainerData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetClockGatingCheckPtr &check) { return ContainerDataAccess::addToPinValueMap(pin_to_check_, pin_name, check); }
    bool addToInst(const std::string &inst_name, const SetClockGatingCheckPtr &check) { return ContainerDataAccess::addToInstValueMap(inst_to_check_, inst_name, check); }
    bool addToClock(const ClockId &clock_id, const SetClockGatingCheckPtr &check) { return ContainerDataAccess::addToClockValueMap(clock_to_check_, clock_id, check); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetClockGatingCheckPtr &check) { return ContainerDataAccess::addToCurrentDesignValueMap(design_to_check_, cell_id, check); }

  private:
    std::unordered_multimap<ObjectId, SetClockGatingCheckPtr> pin_to_check_;
    std::unordered_multimap<ObjectId, SetClockGatingCheckPtr> inst_to_check_;
    std::unordered_multimap<ClockId, SetClockGatingCheckPtr> clock_to_check_;
    std::unordered_map<ObjectId, SetClockGatingCheckPtr> design_to_check_;

  public:
    COMMAND_GET_SET_VAR(pin_to_check, PinToCheck)
    COMMAND_GET_SET_VAR(inst_to_check, InstToCheck)
    COMMAND_GET_SET_VAR(clock_to_check, ClockToCheck)
    COMMAND_GET_SET_VAR(design_to_check, DesignToCheck)
  
  public:
    SPLIT_ONE_GROUP(SetClockGatingCheck, SetClockGatingCheckPtr, Rise, Fall)
};
using ClockGatingCheckContainerDataPtr = std::shared_ptr<ClockGatingCheckContainerData>;

enum class RelationshipType : Bits8 {kPhysicallyExclusive = 0, kAsynchronousAllowPaths, kAsynchronous, kLogicallyExclusive, kUnknown};
std::string toString(const RelationshipType &value);

class SetClockGroups {
  public:
    void addToGroups(std::unordered_set<ClockId> &&clock_ids) { groups_.emplace_back(std::move(clock_ids)); }

  private:
    std::string name_ = "";
    std::string comment_ = "";
    std::vector<std::unordered_set<ClockId> > groups_;
    RelationshipType relation_type_ = RelationshipType::kUnknown;

  public:
    COMMAND_GET_SET_VAR(name, Name)
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_VAR(groups, Groups)
    COMMAND_GET_SET_VAR(relation_type, RelationType)
};
using SetClockGroupsPtr = std::shared_ptr<SetClockGroups>;

using UnorderedClockPair = UnorderedPair<ClockId, ClockId>;
class ClockGroupsContainerData : public BaseContainerData {
  public:
    void addGroup(const SetClockGroupsPtr &group) { all_groups_.emplace(group); }
    bool addClockRelationship(const UnorderedClockPair &clock_pair, const RelationshipType &relation) { return ContainerDataAccess::updateMapValue(clock_relationship_, clock_pair, relation); }
    void setRelationBetweenClockGroups(const std::unordered_set<ClockId> &lhs, const std::unordered_set<ClockId> &rhs, const RelationshipType &relation);
    void buildClockRelationship(SetClockGroupsPtr &set_clock_groups, const std::unordered_set<ClockId> &all_clocks);

  private:
    std::unordered_set<SetClockGroupsPtr> all_groups_;
    std::unordered_map<UnorderedClockPair, RelationshipType, boost::hash<UnorderedClockPair> > clock_relationship_;

  public:
    COMMAND_GET_SET_VAR(all_groups, AllGroups)
    COMMAND_GET_SET_VAR(clock_relationship, ClockRelationship)
};
using ClockGroupsContainerDataPtr = std::shared_ptr<ClockGroupsContainerData>;

class SetClockLatency {
  public:
    SetClockLatency();
    bool operator==(const SetClockLatency &rhs) const;

  private:
    float delay_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(delay, Delay)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    COMMAND_GET_SET_FLAG(dynamic, Dynamic)
    COMMAND_GET_SET_FLAG(source, Source)
    COMMAND_GET_SET_FLAG(early, Early)
    COMMAND_GET_SET_FLAG(late, Late)
};
using SetClockLatencyPtr = std::shared_ptr<SetClockLatency>;

class ClockLatencyOnPin : public SetClockLatency {
  public:
    using Base = SetClockLatency;
    ClockLatencyOnPin() = default;
    ClockLatencyOnPin(Base &base) : Base(base), pin_associated_clocks_() {};
    bool addClock(const ClockId &id) { return ContainerDataAccess::addToClockSet(pin_associated_clocks_, id); }
    bool operator==(const ClockLatencyOnPin &rhs) const;

  private:
    std::unordered_set<ClockId> pin_associated_clocks_;

  public:
    COMMAND_GET_SET_VAR(pin_associated_clocks, PinAssociatedClocks)
};
using ClockLatencyOnPinPtr = std::shared_ptr<ClockLatencyOnPin>;

class ClockLatencyContainerData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name, const ClockLatencyOnPinPtr &latency) { return ContainerDataAccess::addToPinValueMap(pin_to_latency_, pin_name, latency); }
    bool addToClock(const ClockId &clock_id, const SetClockLatencyPtr &latency) { return ContainerDataAccess::addToClockValueMap(clock_to_latency_, clock_id, latency); }

  private:
    std::unordered_multimap<ObjectId, ClockLatencyOnPinPtr> pin_to_latency_;
    std::unordered_multimap<ClockId, SetClockLatencyPtr> clock_to_latency_;

  public:
    COMMAND_GET_SET_VAR(pin_to_latency, PinToLatency)
    COMMAND_GET_SET_VAR(clock_to_latency, ClockToLatency)
  
  public:
    SPLIT_TWO_GROUP(ClockLatencyOnPin, ClockLatencyOnPinPtr, Rise, Fall, Min, Max)
    SPLIT_TWO_GROUP(SetClockLatency, SetClockLatencyPtr, Rise, Fall, Min, Max)
};
using ClockLatencyContainerDataPtr = std::shared_ptr<ClockLatencyContainerData>;

enum class DataType : Bits8 { kClock = 0, kData, kUnknown };
std::string toString(const DataType &value);

enum class PulseType : Bits8 { kRiseTriggeredHighPulse = 0, kRiseTriggeredLowPulse, kFallTriggeredHighPulse, kFallTriggeredLowPulse, kUnknown };
std::string toString(const PulseType &value);

class SetSense {
  public:
    SetSense();

    bool setType(const std::string &type_name);
    bool setPulse(const std::string &pulse_name);
    bool addClock(const ClockId &clock_id) { return ContainerDataAccess::addToClockSet(clocks_, clock_id); }
    bool operator==(const SetSense &rhs) const;

  private:
    DataType type_ = DataType::kClock;
    PulseType pulse_ = PulseType::kUnknown;
    //If clocks is empty, all clocks passing through the given pin need to be considered.
    std::unordered_set<ClockId> clocks_;

  public:
    COMMAND_GET_SET_VAR(type, Type)
    COMMAND_GET_SET_VAR(pulse, Pulse)
    COMMAND_GET_SET_VAR(clocks, Clocks)
    COMMAND_GET_SET_FLAG(non_unate, NonUnate)
    COMMAND_GET_SET_FLAG(positive, Positive)
    COMMAND_GET_SET_FLAG(negative, Negative)
    COMMAND_GET_SET_FLAG(clock_leaf, ClockLeaf)
    COMMAND_GET_SET_FLAG(stop_propagation, StopPropation)
};
using SetSensePtr = std::shared_ptr<SetSense>;

class SenseContainerData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetSensePtr &sense) { return ContainerDataAccess::addToPinValueMap(pin_sense_, pin_name, sense); }

  private:
    //Not support arc to sense yet
    std::unordered_multimap<ObjectId, SetSensePtr> pin_sense_;

  public:
    COMMAND_GET_SET_VAR(pin_sense, PinSense)
};
using SenseContainerDataPtr = std::shared_ptr<SenseContainerData>;

class SetClockTransition {
  public:
    SetClockTransition();
    bool operator==(const SetClockTransition &rhs) const;

  private:
    float transition_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(transition, Transition)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetClockTransitionPtr = std::shared_ptr<SetClockTransition>;

class ClockTransitionContainerData : public BaseContainerData {
  public:
    bool add(const ClockId &clock_id, const SetClockTransitionPtr &transition) { return ContainerDataAccess::addToClockValueMap(clock_transitions_, clock_id, transition); }

  private:
    std::unordered_multimap<ClockId, SetClockTransitionPtr> clock_transitions_;

  public:
    COMMAND_GET_SET_VAR(clock_transitions, ClockTransitions)
  
  public:
    SPLIT_TWO_GROUP(SetClockTransition, SetClockTransitionPtr, Rise, Fall, Min, Max)
};
using ClockTransitionContainerDataPtr = std::shared_ptr<ClockTransitionContainerData>;

class SetClockUncertainty {
  public:
    SetClockUncertainty();
    bool operator==(const SetClockUncertainty &rhs) const;

  private:
    float uncertainty_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(uncertainty, Uncertainty)
    COMMAND_GET_SET_FLAG(setup, Setup)
    COMMAND_GET_SET_FLAG(hold, Hold)
};
using SetClockUncertaintyPtr = std::shared_ptr<SetClockUncertainty>;

class InterClockUncertainty : public SetClockUncertainty {
  public:
    using Base = SetClockUncertainty;
    InterClockUncertainty(Base &base);
    InterClockUncertainty();
    bool operator==(const InterClockUncertainty &rhs) const;

  public:
    COMMAND_GET_SET_FLAG(rise_from, RiseFrom)
    COMMAND_GET_SET_FLAG(fall_from, FallFrom)
    COMMAND_GET_SET_FLAG(rise_to, RiseTo)
    COMMAND_GET_SET_FLAG(fall_to, FallTo)
};
using InterClockUncertaintyPtr = std::shared_ptr<InterClockUncertainty>;

using InterClockPair = std::pair<ClockId, ClockId>;
using InterClockPairPtr = std::shared_ptr<InterClockPair>;
class ClockUncertaintyContainerData : public BaseContainerData {
  public:
    bool addToInterClock(const InterClockPair &clocks, const InterClockUncertaintyPtr &uncertainty) { return ContainerDataAccess::updateMapValue(interclock_uncertainty_, clocks, uncertainty); }
    bool addToPin(const std::string &pin_name, const SetClockUncertaintyPtr &uncertainty) { return ContainerDataAccess::addToPinValueMap(pin_uncertainty_, pin_name, uncertainty); }
    bool addToClock(const ClockId &clock_id, const SetClockUncertaintyPtr &uncertainty) { return ContainerDataAccess::addToClockValueMap(clock_uncertainty_, clock_id, uncertainty); }

  private:
    std::unordered_multimap<InterClockPair, InterClockUncertaintyPtr, boost::hash<std::pair<ClockId, ClockId> > > interclock_uncertainty_;
    std::unordered_multimap<ObjectId, SetClockUncertaintyPtr> pin_uncertainty_;
    std::unordered_multimap<ClockId, SetClockUncertaintyPtr> clock_uncertainty_;

  public:
    COMMAND_GET_SET_VAR(interclock_uncertainty, InterClockUncertainty)
    COMMAND_GET_SET_VAR(pin_uncertainty, PinUncertainty)
    COMMAND_GET_SET_VAR(clock_uncertainty, ClockUncertainty)
  
  public:
    SPLIT_ONE_GROUP(SetClockUncertainty, SetClockUncertaintyPtr, Setup, Hold)
    SPLIT_THREE_GROUP(InterClockUncertainty, InterClockUncertaintyPtr, RiseFrom, FallFrom, RiseTo, FallTo, Setup, Hold)
};
using ClockUncertaintyContainerDataPtr = std::shared_ptr<ClockUncertaintyContainerData>;

class SetDataCheck {
  public:
    SetDataCheck();

    bool operator==(const SetDataCheck &rhs) const;

  private:
    float value_ = 0.0;
    //Sdc2.1 said it was clock list, but other references said it contain only one clock object
    ClockId clock_ = kInvalidClockId;

  public:
    COMMAND_GET_SET_VAR(value, Value)
    COMMAND_GET_SET_VAR(clock, Clock)
    COMMAND_GET_SET_FLAG(rise_from, RiseFrom)
    COMMAND_GET_SET_FLAG(fall_from, FallFrom)
    COMMAND_GET_SET_FLAG(rise_to, RiseTo)
    COMMAND_GET_SET_FLAG(fall_to, FallTo)
    COMMAND_GET_SET_FLAG(setup, Setup)
    COMMAND_GET_SET_FLAG(hold, Hold)
};
using SetDataCheckPtr = std::shared_ptr<SetDataCheck>;

using PinPair = std::pair<ObjectId, ObjectId>;
class DataCheckContainerData : public BaseContainerData {
  public:
    bool addCheck(const std::string &from_pin, const std::string &to_pin, const SetDataCheckPtr &check);
    bool add(const PinPair &pin_pair, const SetDataCheckPtr &check) { return ContainerDataAccess::updateMapValue(pin_data_check_, pin_pair, check); }

  private:
    std::unordered_multimap<PinPair, SetDataCheckPtr, boost::hash<std::pair<ObjectId, ObjectId> > > pin_data_check_;

  public:
    COMMAND_GET_SET_VAR(pin_data_check, PinDataCheck)
    SPLIT_THREE_GROUP(SetDataCheck, SetDataCheckPtr, RiseFrom, FallFrom, RiseTo, FallTo, Setup, Hold)
};
using DataCheckContainerDataPtr = std::shared_ptr<DataCheckContainerData>;

class SetDisableTiming {
  public:
    SetDisableTiming();
    bool operator==(const SetDisableTiming &rhs) const;

  private:
    ObjectId from_id_ = UNINIT_OBJECT_ID;
    ObjectId to_id_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(from_id, FromId)
    COMMAND_GET_SET_VAR(to_id, ToId)
    COMMAND_GET_SET_FLAG(all_arcs, AllArcs)
};
using SetDisableTimingPtr = std::shared_ptr<SetDisableTiming>;

class DisableTimingContainerData {
  public:
    bool addToInst(const std::string &inst_name, const std::string &from, const std::string &to);
    bool addToTCell(AnalysisCorner *corner, const std::string &lib_cell_name, const std::string &from, const std::string &to);
    bool addToPin(const std::string &pin_name, const std::string &from, const std::string &to);
    bool addToTTerm(AnalysisCorner *corner, const std::string &lib_pin_name, const std::string &from, const std::string &to);

  private:
    std::unordered_multimap<ObjectId, SetDisableTimingPtr> inst_disable_timing_;
    std::unordered_multimap<ObjectId, SetDisableTimingPtr> tcell_disable_timing_;
    std::unordered_set<ObjectId> pin_disable_timing_;
    std::unordered_set<ObjectId> tterm_disable_timing_;

  public:
    COMMAND_GET_SET_VAR(inst_disable_timing, InstDisableTiming)
    COMMAND_GET_SET_VAR(tcell_disable_timing, TCellDisableTiming)
    COMMAND_GET_SET_VAR(pin_disable_timing, PinDisableTiming)
    COMMAND_GET_SET_VAR(tterm_disable_timing, TTermDisableTiming)
};
using DisableTimingContainerDataPtr = std::shared_ptr<DisableTimingContainerData>;

class SetFalsePath : public ExceptionPath {
  public:
    SetFalsePath();
    void checkFlags();

  private:
    std::string comment_ = "";

  public:
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_FLAG(setup, Setup)
    COMMAND_GET_SET_FLAG(hold, Hold)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
};
using SetFalsePathPtr = std::shared_ptr<SetFalsePath>;

class FalsePathContainerData : public BaseContainerData {
  public:
    void add(const SetFalsePathPtr &path) { false_paths_.emplace(path); };

  private:
    std::unordered_set<SetFalsePathPtr> false_paths_;

  public:
    COMMAND_GET_SET_VAR(false_paths, FalsePaths);
};
using FalsePathContainerDataPtr = std::shared_ptr<FalsePathContainerData>;

class SetIdealLatency {
  public:
    SetIdealLatency();
    bool operator==(const SetIdealLatency &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetIdealLatencyPtr = std::shared_ptr<SetIdealLatency>;

class IdealLatencyContainerData {
  public:
    bool add(const std::string &pin_name, const SetIdealLatencyPtr &latency) { return ContainerDataAccess::addToPinValueMap(pin_to_latency_, pin_name, latency); }

  private:
    std::unordered_multimap<ObjectId, SetIdealLatencyPtr> pin_to_latency_;

  public:
    COMMAND_GET_SET_VAR(pin_to_latency, PinToLatency)
    SPLIT_TWO_GROUP(SetIdealLatency, SetIdealLatencyPtr, Rise, Fall, Min, Max)
};
using IdealLatencyContainerDataPtr = std::shared_ptr<IdealLatencyContainerData>;

class SetIdealNetwork {
  public:
    SetIdealNetwork()=default;

    bool addToPin(const std::string &pin_name, const bool is_no_propagated) { return ContainerDataAccess::addToPinValueMap(pins_, pin_name, is_no_propagated); }
    bool addToNet(const std::string &net_name, const bool is_no_propagated);

  private:
    std::unordered_map<ObjectId, bool> pins_;
    std::unordered_set<ObjectId> nets_;

  public:
    COMMAND_GET_SET_VAR(pins, Pins)
    COMMAND_GET_SET_VAR(nets, Nets)
};
using SetIdealNetworkPtr = std::shared_ptr<SetIdealNetwork>;

class SetIdealTransition {
  public:
    SetIdealTransition();
    bool operator==(const SetIdealTransition &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
};
using SetIdealTransitionPtr = std::shared_ptr<SetIdealTransition>;

class IdealTransitionContainerData {
  public:
    bool add(const std::string &pin_name, const SetIdealTransitionPtr &transition) { return ContainerDataAccess::addToPinValueMap(pin_ideal_transitions_, pin_name, transition); }

  private:
    std::unordered_multimap<ObjectId, SetIdealTransitionPtr> pin_ideal_transitions_;

  public:
    COMMAND_GET_SET_VAR(pin_ideal_transitions, PinIdealTransitions)
    SPLIT_TWO_GROUP(SetIdealTransition, SetIdealTransitionPtr, Rise, Fall, Min, Max)
};
using IdealTransitionContainerDataPtr = std::shared_ptr<IdealTransitionContainerData>;

class PortDelay {
  public:
    PortDelay();

    bool setReferencePin(const std::string &pin_name) { return ContainerDataAccess::setToPin(reference_pin_, pin_name); }
    bool operator==(const PortDelay &rhs) const;

  private:
    float delay_value_ = 0.0;
    ClockId clock_ = kInvalidClockId;
    ObjectId reference_pin_ = UNINIT_OBJECT_ID;

  public:
    COMMAND_GET_SET_VAR(delay_value, DelayValue)
    COMMAND_GET_SET_VAR(clock, Clock)
    COMMAND_GET_SET_VAR(reference_pin, ReferencePin)
    COMMAND_GET_SET_FLAG(clock_fall, ClockFall)
    COMMAND_GET_SET_FLAG(level_sensitive, LevelSensitive)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(min, Min)
    COMMAND_GET_SET_FLAG(max, Max)
    COMMAND_GET_SET_FLAG(add_delay, AddDelay)
    COMMAND_GET_SET_FLAG(network_latency_included, NetworkLatencyIncluded)
    COMMAND_GET_SET_FLAG(source_latency_included, SourceLatencyIncluded)
};
using PortDelayPtr = std::shared_ptr<PortDelay>;

class SetInputDelay : public PortDelay {

};
using SetInputDelayPtr = std::shared_ptr<SetInputDelay>;

class InputDelayContainerData : public BaseContainerData {
  public:
    bool add(const std::string &pin_name, const SetInputDelayPtr &delay) { return ContainerDataAccess::addToPinValueMap(pin_input_delays_, pin_name, delay); }

  private:
    //TODO: Should we need another map that defined ref pin to input delay?
    std::unordered_multimap<ObjectId, SetInputDelayPtr> pin_input_delays_;

  public:
    COMMAND_GET_SET_VAR(pin_input_delays, PinInputDelays)
    SPLIT_TWO_GROUP(SetInputDelay, SetInputDelayPtr, Rise, Fall, Min, Max)
};
using InputDelayContainerDataPtr = std::shared_ptr<InputDelayContainerData>;

class SetMaxDelay : public ExceptionPath {
  public:
    SetMaxDelay();
    void checkFlags();

  private:
    float delay_value_ = 0.0;
    std::string comment_ = "";

  public:
    COMMAND_GET_SET_VAR(delay_value, DelayValue)
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(ignore_clock_latency, IgnoreClockLatency)
};
using SetMaxDelayPtr = std::shared_ptr<SetMaxDelay>;

class MaxDelayContainerData : public BaseContainerData {
  public:
    void add(const SetMaxDelayPtr &delay) { max_delays_.emplace(delay); }

  private:
    std::unordered_set<SetMaxDelayPtr> max_delays_;

  public:
    COMMAND_GET_SET_VAR(max_delays, MaxDelays)
};
using MaxDelayContainerDataPtr = std::shared_ptr<MaxDelayContainerData>;

class SetMaxTimeBorrow {
  public:
    bool operator==(const SetMaxTimeBorrow &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
};
using SetMaxTimeBorrowPtr = std::shared_ptr<SetMaxTimeBorrow>;

class MaxTimeBorrowContainerData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetMaxTimeBorrow &time_borrow) { return ContainerDataAccess::addToPinValueMap(pin_max_time_borrow_, pin_name, time_borrow); }
    bool addToInst(const std::string &inst_name, const SetMaxTimeBorrow &time_borrow) { return ContainerDataAccess::addToInstValueMap(inst_max_time_borrow_, inst_name, time_borrow);}
    bool addToClock(const ClockId &clock_id, const SetMaxTimeBorrow &time_borrow) { return ContainerDataAccess::addToClockValueMap(clock_max_time_borrow_, clock_id, time_borrow); }

  private:
    std::unordered_map<ObjectId, SetMaxTimeBorrow> pin_max_time_borrow_;
    std::unordered_map<ObjectId, SetMaxTimeBorrow> inst_max_time_borrow_;
    std::unordered_map<ClockId, SetMaxTimeBorrow> clock_max_time_borrow_;

  public:
    COMMAND_GET_SET_VAR(pin_max_time_borrow, PinMaxTimeBorrow)
    COMMAND_GET_SET_VAR(inst_max_time_borrow, InstMaxTimeBorrow)
    COMMAND_GET_SET_VAR(clock_max_time_borrow, ClockMaxTimeBorrow)
};
using MaxTimeBorrowContainerDataPtr = std::shared_ptr<MaxTimeBorrowContainerData>;

class SetMinDelay : public ExceptionPath {
  public:
    SetMinDelay();
    void checkFlags();

  private:
    float delay_value_ = 0.0;
    std::string comment_ = "";

  public:
    COMMAND_GET_SET_VAR(delay_value, DelayValue)
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(ignore_clock_latency, IgnoreClockLatency)
};
using SetMinDelayPtr = std::shared_ptr<SetMinDelay>;

class MinDelayContainerData : public BaseContainerData {
  public:
    void add(const SetMinDelayPtr &delay) { min_delays_.emplace(delay); }

  private:
    std::unordered_set<SetMinDelayPtr> min_delays_;

  public:
    COMMAND_GET_SET_VAR(min_delays, MinDelays)
};
using MinDelayContainerDataPtr = std::shared_ptr<MinDelayContainerData>;

class SetMinPulseWidth {
  public:
    SetMinPulseWidth();
    bool operator==(const SetMinPulseWidth &rhs) const;

  private:
    float value_ = 0.0;

  public:
    COMMAND_GET_SET_VAR(value, Value)
    COMMAND_GET_SET_FLAG(low, Low)
    COMMAND_GET_SET_FLAG(high, High)
};
using SetMinPulseWidthPtr = std::shared_ptr<SetMinPulseWidth>;

class MinPulseWidthData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name, const SetMinPulseWidthPtr &pulse_width) { return ContainerDataAccess::addToPinValueMap(pin_min_pulse_width_, pin_name, pulse_width); }
    bool addToInst(const std::string &inst_name, const SetMinPulseWidthPtr &pulse_width) { return ContainerDataAccess::addToInstValueMap(inst_min_pulse_width_, inst_name, pulse_width); }
    bool addToClock(const ClockId &clock_id, const SetMinPulseWidthPtr &pulse_width) { return ContainerDataAccess::addToClockValueMap(clock_min_pulse_width_, clock_id, pulse_width); }
    bool addToCurrentDesign(const ObjectId &cell_id, const SetMinPulseWidthPtr &pulse_width) { return ContainerDataAccess::addToCurrentDesignValueMap( design_min_pulse_width_, cell_id, pulse_width); }

  private:
    std::unordered_multimap<ObjectId, SetMinPulseWidthPtr> pin_min_pulse_width_;
    std::unordered_multimap<ObjectId, SetMinPulseWidthPtr> inst_min_pulse_width_;
    std::unordered_multimap<ClockId, SetMinPulseWidthPtr> clock_min_pulse_width_;
    std::unordered_multimap<ObjectId, SetMinPulseWidthPtr> design_min_pulse_width_;

  public:
    COMMAND_GET_SET_VAR(pin_min_pulse_width, PinMinPulseWidth)
    COMMAND_GET_SET_VAR(inst_min_pulse_width, InstMinPulseWidth)
    COMMAND_GET_SET_VAR(clock_min_pulse_width, ClockMinPulseWidth)
    COMMAND_GET_SET_VAR(design_min_pulse_width, DesignMinPulseWidth)
  
  public:
    SPLIT_ONE_GROUP(SetMinPulseWidth, SetMinPulseWidthPtr, Low, High)
};
using MinPulseWidthDataPtr = std::shared_ptr<MinPulseWidthData>;

class SetMulticyclePath : public ExceptionPath {
  public:
    SetMulticyclePath();
    void checkFlags();

  private:
    std::string comment_ = "";
    UInt32 path_multiplier_ = 0;

  public:
    COMMAND_GET_SET_VAR(comment, Comment)
    COMMAND_GET_SET_VAR(path_multiplier, PathMultiplier)
    COMMAND_GET_SET_FLAG(setup, Setup)
    COMMAND_GET_SET_FLAG(hold, Hold)
    COMMAND_GET_SET_FLAG(rise, Rise)
    COMMAND_GET_SET_FLAG(fall, Fall)
    COMMAND_GET_SET_FLAG(start, Start)
    COMMAND_GET_SET_FLAG(end, End)
};
using SetMulticyclePathPtr = std::shared_ptr<SetMulticyclePath>;

class MulticyclePathContainerData : public BaseContainerData {
  public:
    void add(const SetMulticyclePathPtr &path) { multi_cycle_paths_.emplace(path); };

  private:
    std::unordered_set<SetMulticyclePathPtr> multi_cycle_paths_;

  public:
    COMMAND_GET_SET_VAR(multi_cycle_paths, MultiCyclePaths)
};
using MulticyclePathContainerDataPtr = std::shared_ptr<MulticyclePathContainerData>;

class SetOutputDelay : public PortDelay {

};
using SetOutputDelayPtr = std::shared_ptr<SetOutputDelay>;

class OutputDelayContainerData : public BaseContainerData {
  public:
    bool add(const std::string &pin_name, const SetOutputDelayPtr &delay) { return ContainerDataAccess::addToPinValueMap(pin_output_delays_, pin_name, delay); }

  private:
    //Should we need another map that defined ref pin to output delay?
    std::unordered_multimap<ObjectId, SetOutputDelayPtr> pin_output_delays_;

  public:
    COMMAND_GET_SET_VAR(pin_output_delays, PinOutputDelays)
    SPLIT_TWO_GROUP(SetOutputDelay, SetOutputDelayPtr, Rise, Fall, Min, Max)
};
using OutputDelayContainerDataPtr = std::shared_ptr<OutputDelayContainerData>;

class SetPropagatedClockContainerData : public BaseContainerData {
  public:
    bool addToPin(const std::string &pin_name) { return ContainerDataAccess::addToPinSet(pins_, pin_name); }
    bool addToClock(ClockPtr &clock);

  private:
    std::unordered_set<ObjectId> pins_;
    std::unordered_set<ClockId> clocks_;

  public:
    COMMAND_GET_SET_VAR(pins, Pins)
    COMMAND_GET_SET_VAR(clocks, Clocks)
};
using SetPropagatedClockContainerDataPtr = std::shared_ptr<SetPropagatedClockContainerData>;


}
}
#endif //EDI_DB_TIMING_SDC_TIMING_CONSTRAINTS_H_
