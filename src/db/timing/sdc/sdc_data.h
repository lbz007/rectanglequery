/**
 * @file sdc_data.h
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

#ifndef EDI_DB_TIMING_SDC_SDC_DATA_H_
#define EDI_DB_TIMING_SDC_SDC_DATA_H_

#include "db/timing/sdc/general_purpose_commands.h"
#include "db/timing/sdc/object_access_commands.h"
#include "db/timing/sdc/timing_constraints.h"
#include "db/timing/sdc/environment_commands.h"
#include "db/timing/sdc/multivoltage_power_commands.h"

namespace open_edi {
namespace db {

//general purpose commands
class SdcCurrentInstanceContainer {
  public:
    SdcCurrentInstanceContainer() { data_ = std::make_shared<CurrentInstance>(); }

    void setData(const CurrentInstancePtr &data) { data_ = data; }
    const CurrentInstancePtr getData() const { return data_; }
    CurrentInstancePtr getData() { return data_; }
    const std::string getInstName() const;
    const ObjectId getInstId() const { return data_->getInstId(); }

  private:
    CurrentInstancePtr data_;
};
using SdcCurrentInstanceContainerPtr = std::shared_ptr<SdcCurrentInstanceContainer>;

class SdcHierarchySeparatorContainer {
  public:
    SdcHierarchySeparatorContainer() { data_ = std::make_shared<SetHierarchySeparator>(); }

    void setData(const SetHierarchySeparatorPtr &data) { data_ = data; }
    const SetHierarchySeparatorPtr getData() const { return data_; }
    SetHierarchySeparatorPtr getData() { return data_; }
    const char get() const { return data_->getSeparator(); }
    friend std::ostream &operator<<(std::ostream &os, SdcHierarchySeparatorContainer &rhs);

  private:
    SetHierarchySeparatorPtr data_;
};
using SdcHierarchySeparatorContainerPtr = std::shared_ptr<SdcHierarchySeparatorContainer>;

class SdcUnitsContainer {
  public:
    SdcUnitsContainer() { data_ = std::make_shared<SetUnits>(); }
    void setData(const SetUnitsPtr &data) { data_ = data; }
    const SetUnitsPtr getData() const { return data_; }
    SetUnitsPtr getData() { return data_; }

    const float &getSdcCapacitanceUnits() const { return data_->getCapacitanceUnit(); }
    const float &getSdcResistanceUnits() const { return data_->getResistanceUnit(); }
    const float &getSdcTimeUnits() const { return data_->getTimeUnit(); }
    const float &getSdcVoltageUnits() const { return data_->getVoltageUnit(); }
    const float &getSdcCurrentUnits() const { return data_->getCurrentUnit(); }
    const float &getSdcPowerUnits() const { return data_->getPowerUnit(); }
    friend std::ostream &operator<<(std::ostream &os, SdcUnitsContainer &rhs);

  private:
    SetUnitsPtr data_;
};
using SdcUnitsContainerPtr = std::shared_ptr<SdcUnitsContainer>;

//timing constraint commands
class SdcClockContainer {
  public:
    SdcClockContainer() { data_ = std::make_shared<ClockContainerData>(); }

    void setData(const ClockContainerDataPtr &data) { data_ = data; }
    const ClockContainerDataPtr getData() const { return data_; }
    ClockContainerDataPtr getData() { return data_; }

    void getClocks(std::vector<ClockPtr> &clocks) const;
    void getVirtualClocks(std::vector<ClockPtr> &clocks) const;
    void getClockNames(std::vector<std::string> &names) const;
    void getClockIds(std::vector<ClockId> &ids) const;

    ClockPtr getClock(const ClockId &id) const;
    ClockPtr getClock(const std::string &name) const;
    const ClockId &getClockId(const std::string &name) const;
    const std::string &getClockName(const ClockId &id) const;
    
    void getClockOnPin(std::vector<ClockPtr> &clocks, const ObjectId &pin_id) const;
    void getPinOnClock(std::vector<ObjectId> &pins, const ClockId &id) const;
    bool isClockPin(const ObjectId &pin_id) const;

    bool isInvalidClock(const ClockId &id) { return getClock(id) == nullptr; }
    bool isValidClock(const ClockId &id) { return !isInvalidClock(id); }

    friend std::ostream &operator<<(std::ostream &os, SdcClockContainer &rhs);

  private:
    ClockContainerDataPtr data_;
};
using SdcClockContainerPtr = std::shared_ptr<SdcClockContainer>;

class SdcGroupPathContainer {
  public:
    SdcGroupPathContainer() { data_ = std::make_shared<GroupPathContainerData>(); }

    void setData(const GroupPathContainerDataPtr &data) { data_ = data; }
    const GroupPathContainerDataPtr getData() const { return data_; }
    GroupPathContainerDataPtr getData() { return data_; }
    const std::unordered_set<GroupPathPtr> &get() const { return data_->getGroupPaths(); }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcGroupPathContainer &rhs);

  private:
    GroupPathContainerDataPtr data_;
};
using SdcGroupPathContainerPtr = std::shared_ptr<SdcGroupPathContainer>;

class SdcClockGatingCheckContainer {
  public:
    SdcClockGatingCheckContainer() { data_ = std::make_shared<ClockGatingCheckContainerData>(); }

    void setData(const ClockGatingCheckContainerDataPtr &data) { data_ = data; }
    const ClockGatingCheckContainerDataPtr getData() const { return data_; }
    ClockGatingCheckContainerDataPtr getData() { return data_; }

    const SetClockGatingCheckPtr getCurrentDesignCheckTime() const;
    const std::pair<float, float> getPinCheckTime(bool is_rise, bool is_high, const ObjectId &pin_id) const;
    const std::pair<float, float> getInstCheckTime(bool is_rise, bool is_high, const ObjectId &inst_id) const;
    const std::pair<float, float> getClockCheckTime(bool is_rise, bool is_high, const ClockId &clock_id) const;
    const float getPinCheckTime(bool is_rise, bool is_high, bool is_setup, const ObjectId &pin_id) const;
    const float getInstCheckTime(bool is_rise, bool is_high, bool is_setup, const ObjectId &inst_id) const;
    const float getClockCheckTime(bool is_rise, bool is_high, bool is_setup, const ClockId &clock_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcClockGatingCheckContainer &rhs);

  private:
    ClockGatingCheckContainerDataPtr data_;
};
using SdcClockGatingCheckContainerPtr = std::shared_ptr<SdcClockGatingCheckContainer>;

class SdcClockGroupsContainer {
  public:
    SdcClockGroupsContainer() { data_ = std::make_shared<ClockGroupsContainerData>(); }

    void setData(const ClockGroupsContainerDataPtr &data) { data_ = data; }
    const ClockGroupsContainerDataPtr getData() const { return data_; }
    ClockGroupsContainerDataPtr getData() { return data_; }
    RelationshipType getClocksRelationshipType(const ClockId &clock_id, const ClockId &other_clock_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcClockGroupsContainer &rhs);

  private:
    ClockGroupsContainerDataPtr data_;
};
using SdcClockGroupsContainerPtr = std::shared_ptr<SdcClockGroupsContainer>;

class SdcClockLatencyContainer {
  public:
    SdcClockLatencyContainer() { data_ = std::make_shared<ClockLatencyContainerData>(); }

    void setData(const ClockLatencyContainerDataPtr &data) { data_ = data; }
    const ClockLatencyContainerDataPtr getData() const { return data_; }
    ClockLatencyContainerDataPtr getData() { return data_; }
    void getClockLatencyOnPin(std::vector<ClockLatencyOnPinPtr> &latencys, const ObjectId &pin_id) const;
    void getClockLatencyOnClock(std::vector<SetClockLatencyPtr> &latencys, const ClockId &clock_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcClockLatencyContainer &rhs);

  private:
    ClockLatencyContainerDataPtr data_;
};
using SdcClockLatencyContainerPtr = std::shared_ptr<SdcClockLatencyContainer>;

class SdcSenseContainer {
  public:
    SdcSenseContainer() { data_ = std::make_shared<SenseContainerData>(); }

    void setData(const SenseContainerDataPtr &data) { data_ = data; }
    const SenseContainerDataPtr getData() const { return data_; }
    SenseContainerDataPtr getData() { return data_; }
    void getPinSense(std::vector<SetSensePtr> &senses, const ObjectId &pin_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcSenseContainer &rhs);

  private:
    SenseContainerDataPtr data_;
};
using SdcSenseContainerPtr = std::shared_ptr<SdcSenseContainer>;

class SdcClockTransitionContainer {
  public:
    SdcClockTransitionContainer() { data_ = std::make_shared<ClockTransitionContainerData>(); }

    void setData(const ClockTransitionContainerDataPtr &data) { data_ = data; }
    const ClockTransitionContainerDataPtr getData() const { return data_; }
    ClockTransitionContainerDataPtr getData() { return data_; }
    void getTransition(std::vector<SetClockTransitionPtr> &transitions, const ClockId &clock_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcClockTransitionContainer &rhs);

  private:
    ClockTransitionContainerDataPtr data_;
};
using SdcClockTransitionContainerPtr = std::shared_ptr<SdcClockTransitionContainer>;

class SdcClockUncertaintyContainer {
  public:
    SdcClockUncertaintyContainer() { data_ = std::make_shared<ClockUncertaintyContainerData>(); }

    void setData(const ClockUncertaintyContainerDataPtr &data) { data_ = data; }
    const ClockUncertaintyContainerDataPtr getData() const { return data_; }
    ClockUncertaintyContainerDataPtr getData() { return data_; }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcClockUncertaintyContainer &rhs);
    //Apis
    const float getUncertaintyOnInterClocks(bool is_setup, bool is_rise_from, bool is_rise_to, const ClockId &from_clock_id, const ClockId &to_clock_id) const; //TODO
    const float getUncertaintyOnPin(bool is_setup, const ObjectId &pin_id) const;
    const float getUncertaintyOnClock(bool is_setup, const ClockId &clock_id) const;

  private:
    ClockUncertaintyContainerDataPtr data_;
};
using SdcClockUncertaintyContainerPtr = std::shared_ptr<SdcClockUncertaintyContainer>;

class SdcDataCheckContainer {
  public:
    SdcDataCheckContainer() { data_ = std::make_shared<DataCheckContainerData>(); }

    void setData(const DataCheckContainerDataPtr &data) { data_ = data; }
    const DataCheckContainerDataPtr getData() const { return data_; }
    DataCheckContainerDataPtr getData() { return data_; }
    void getDataCheckOnInterPins(std::vector<SetDataCheckPtr> &checks, const ObjectId &from_pin_id, const ObjectId &to_pin_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcDataCheckContainer &rhs);

  private:
    DataCheckContainerDataPtr data_;
};
using SdcDataCheckContainerPtr = std::shared_ptr<SdcDataCheckContainer>;

class SdcDisableTimingContainer {
  public:
    SdcDisableTimingContainer() { data_ = std::make_shared<DisableTimingContainerData>(); }

    void setData(const DisableTimingContainerDataPtr &data) { data_ = data; }
    const DisableTimingContainerDataPtr getData() const { return data_; }
    DisableTimingContainerDataPtr getData() { return data_; }
    void forEachInstValue(const std::function<void(const ObjectId &, const SetDisableTimingPtr &)> &func) const {
        ContainerDataAccess::forEachObjectMap(data_->getInstDisableTiming(), func);
    }
    void forEachTCellValue(const std::function<void(const ObjectId &, const SetDisableTimingPtr &)> &func) const {
        ContainerDataAccess::forEachObjectMap(data_->getTCellDisableTiming(), func);
    }
    void forEachPinValue(const std::function<void(const ObjectId &)> &func) const {
        ContainerDataAccess::forEachObjectSet(data_->getPinDisableTiming(), func);
    }
    void forEachTTermValue(const std::function<void(const ObjectId &)> &func) const {
        ContainerDataAccess::forEachObjectSet(data_->getTTermDisableTiming(), func);
    }
    friend std::ostream &operator<<(std::ostream &os, SdcDisableTimingContainer &rhs);

  private:
    DisableTimingContainerDataPtr data_;
};
using SdcDisableTimingContainerPtr = std::shared_ptr<SdcDisableTimingContainer>;

class SdcFalsePathContainer {
  public:
    SdcFalsePathContainer() { data_ = std::make_shared<FalsePathContainerData>(); }

    void setData(const FalsePathContainerDataPtr &data) { data_ = data; }
    const FalsePathContainerDataPtr getData() const { return data_; }
    FalsePathContainerDataPtr getData() { return data_; }
    void get(std::unordered_set<SetFalsePathPtr> &paths) { paths = data_->getFalsePaths(); }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcFalsePathContainer &rhs);

  private:
    FalsePathContainerDataPtr data_;
};
using SdcFalsePathContainerPtr = std::shared_ptr<SdcFalsePathContainer>;

class SdcIdealLatencyContainer {
  public:
    SdcIdealLatencyContainer() { data_ = std::make_shared<IdealLatencyContainerData>(); }

    void setData(const IdealLatencyContainerDataPtr &data) { data_ = data; }
    const IdealLatencyContainerDataPtr getData() const { return data_; }
    IdealLatencyContainerDataPtr getData() { return data_; }
    void getPinLatency(std::vector<SetIdealLatencyPtr> &latency, const ObjectId &pin_id) const;
    friend std::ostream &operator<<(std::ostream &os, SdcIdealLatencyContainer &rhs);

  private:
    IdealLatencyContainerDataPtr data_;
};
using SdcIdealLatencyContainerPtr = std::shared_ptr<SdcIdealLatencyContainer>;

class SdcIdealNetworkContainer {
  public:
    SdcIdealNetworkContainer() { data_ = std::make_shared<SetIdealNetwork>(); }

    void setData(const SetIdealNetworkPtr &data) { data_ = data; }
    const SetIdealNetworkPtr getData() const { return data_; }
    SetIdealNetworkPtr getData() { return data_; }
    //TODO
    bool isPinIdealSource(const ObjectId &pin_id) const;
    friend std::ostream &operator<<(std::ostream &os, SdcIdealNetworkContainer &rhs);

  private:
    SetIdealNetworkPtr data_;
};
using SdcIdealNetworkContainerPtr = std::shared_ptr<SdcIdealNetworkContainer>;

class SdcIdealTransitionContainer {
  public:
    SdcIdealTransitionContainer() { data_ = std::make_shared<IdealTransitionContainerData>(); }

    void setData(const IdealTransitionContainerDataPtr &data) { data_ = data; }
    const IdealTransitionContainerDataPtr getData() const { return data_; }
    IdealTransitionContainerDataPtr getData() { return data_; }
    void getPinTransition(std::vector<SetIdealTransitionPtr> &transitions, const ObjectId &pin_id) const;
    friend std::ostream &operator<<(std::ostream &os, SdcIdealTransitionContainer &rhs);

  private:
    IdealTransitionContainerDataPtr data_;
};
using SdcIdealTransitionContainerPtr = std::shared_ptr<SdcIdealTransitionContainer>;

class SdcInputDelayContainer {
  public:
    SdcInputDelayContainer() { data_ = std::make_shared<InputDelayContainerData>(); }

    void setData(const InputDelayContainerDataPtr &data) { data_ = data; }
    const InputDelayContainerDataPtr getData() const { return data_; }
    InputDelayContainerDataPtr getData() { return data_; }
    void getPinInputDelay(std::vector<SetInputDelayPtr> &delays, const ObjectId &pin_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcInputDelayContainer &rhs);

  private:
    InputDelayContainerDataPtr data_;
};
using SdcInputDelayContainerPtr = std::shared_ptr<SdcInputDelayContainer>;

class SdcMaxDelayContainer {
  public:
    SdcMaxDelayContainer() { data_ = std::make_shared<MaxDelayContainerData>(); }

    void setData(const MaxDelayContainerDataPtr &data) { data_ = data; };
    const MaxDelayContainerDataPtr getData() const { return data_; };
    MaxDelayContainerDataPtr getData() { return data_; };
    void getMaxDelays(std::unordered_set<SetMaxDelayPtr> &delays) const { delays = data_->getMaxDelays(); }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMaxDelayContainer &rhs);

  private:
    MaxDelayContainerDataPtr data_;
};
using SdcMaxDelayContainerPtr = std::shared_ptr<SdcMaxDelayContainer>;

class SdcMaxTimeBorrowContainer {
  public:
    SdcMaxTimeBorrowContainer() { data_ = std::make_shared<MaxTimeBorrowContainerData>(); }

    void setData(const MaxTimeBorrowContainerDataPtr &data) { data_ = data; }
    const MaxTimeBorrowContainerDataPtr getData() const { return data_; }
    MaxTimeBorrowContainerDataPtr getData() { return data_; }
    const float getMaxTimeBorrowOnPin(const ObjectId &pin_id);
    const float getMaxTimeBorrowOnInst(const ObjectId &inst_id);
    const float getMaxTimeBorrowOnClock(const ClockId &clock_id);
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMaxTimeBorrowContainer &rhs);

  private:
    MaxTimeBorrowContainerDataPtr data_;
};
using SdcMaxTimeBorrowContainerPtr = std::shared_ptr<SdcMaxTimeBorrowContainer>;

class SdcMinDelayContainer {
  public:
    SdcMinDelayContainer() { data_ = std::make_shared<MinDelayContainerData>(); }

    void setData(const MinDelayContainerDataPtr &data) { data_ = data; }
    const MinDelayContainerDataPtr getData() const { return data_; }
    MinDelayContainerDataPtr getData() { return data_; }
    void get(std::unordered_set<SetMinDelayPtr> &delays) { delays = data_->getMinDelays(); }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMinDelayContainer &rhs);

  private:
    MinDelayContainerDataPtr data_;
};
using SdcMinDelayContainerPtr = std::shared_ptr<SdcMinDelayContainer>;

class SdcMinPulseWidthContainer {
  public:
    SdcMinPulseWidthContainer() { data_ = std::make_shared<MinPulseWidthData>(); }

    void setData(const MinPulseWidthDataPtr &data) { data_ = data; }
    const MinPulseWidthDataPtr getData() const { return data_; }
    MinPulseWidthDataPtr getData() { return data_; }
    void getMinPulseWidthOnPin(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &pin_id) const;
    void getMinPulseWidthOnInst(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &inst_id) const;
    void getMinPulseWidthOnClock(std::vector<SetMinPulseWidthPtr> &pulse_width, const ObjectId &clock_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMinPulseWidthContainer &rhs);

  private:
    MinPulseWidthDataPtr data_;
};
using SdcMinPulseWidthContainerPtr = std::shared_ptr<SdcMinPulseWidthContainer>;

class SdcMulticyclePathContainer {
  public:
    SdcMulticyclePathContainer() { data_ = std::make_shared<MulticyclePathContainerData>(); }

    void setData(const MulticyclePathContainerDataPtr &data) { data_ = data; }
    const MulticyclePathContainerDataPtr getData() const { return data_; }
    MulticyclePathContainerDataPtr getData() { return data_; }
    void get(std::unordered_set<SetMulticyclePathPtr> &paths) const { paths = data_->getMultiCyclePaths(); }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMulticyclePathContainer &rhs);

  private:
    MulticyclePathContainerDataPtr data_;
};
using SdcMulticyclePathContainerPtr = std::shared_ptr<SdcMulticyclePathContainer>;

class SdcOutputDelayContainer {
  public:
    SdcOutputDelayContainer() { data_ = std::make_shared<OutputDelayContainerData>(); }

    void setData(const OutputDelayContainerDataPtr &data) { data_ = data; }
    const OutputDelayContainerDataPtr getData() const { return data_; }
    OutputDelayContainerDataPtr getData() { return data_; }
    void getPinOutputDelay(std::vector<SetOutputDelayPtr> &delays, const ObjectId &pin_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcOutputDelayContainer &rhs);

  private:
    OutputDelayContainerDataPtr data_;
};
using SdcOutputDelayContainerPtr = std::shared_ptr<SdcOutputDelayContainer>;

class SdcPropagatedClockContainer {
  public:
    SdcPropagatedClockContainer() { data_ = std::make_shared<SetPropagatedClockContainerData>(); }

    void setData(const SetPropagatedClockContainerDataPtr &data) { data_ = data; }
    const SetPropagatedClockContainerDataPtr getData() const { return data_; }
    SetPropagatedClockContainerDataPtr getData() { return data_; }
    bool isPropagatedPin(const ObjectId &object_id) const;
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcPropagatedClockContainer &rhs);

  private:
    SetPropagatedClockContainerDataPtr data_;
};
using SdcPropagatedClockContainerPtr = std::shared_ptr<SdcPropagatedClockContainer>;


//environment commands
class SdcCaseAnalysisContainer {
  public:
    SdcCaseAnalysisContainer() { data_ = std::make_shared<CaseAnalysisCotnainerData>(); }

    void setData(const CaseAnalysisCotnainerDataPtr &data) { data_ = data; }
    const CaseAnalysisCotnainerDataPtr getData() const { return data_; }
    CaseAnalysisCotnainerDataPtr getData() { return data_; }
    const CaseValue getPinValue(const ObjectId &pin_id) const;
    void forEachPinValue(const std::function<void(const ObjectId &, const SetCaseAnalysis &)> &func) const {
        ContainerDataAccess::forEachObjectMap(data_->getPinToCaseAnalysis(), func);
    }
    friend std::ostream &operator<<(std::ostream &os, SdcCaseAnalysisContainer &rhs);

  private:
    CaseAnalysisCotnainerDataPtr data_;
};
using SdcCaseAnalysisContainerPtr = std::shared_ptr<SdcCaseAnalysisContainer>;

class SdcDriveContainer {
  public:
    SdcDriveContainer() { data_ = std::make_shared<DriveContainerData>(); }
    void setData(const DriveContainerDataPtr &data) { data_ = data; }
    const DriveContainerDataPtr getData() const { return data_; }
    DriveContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcDriveContainer &rhs);

  private:
    DriveContainerDataPtr data_;
};
using SdcDriveContainerPtr = std::shared_ptr<SdcDriveContainer>;

class SdcDrivingCellContainer {
  public:
    SdcDrivingCellContainer() { data_ = std::make_shared<DrivingCellData>(); }

    void setData(const DrivingCellDataPtr &data) { data_ = data; }
    const DrivingCellDataPtr getData() const { return data_; }
    DrivingCellDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcDrivingCellContainer &rhs);
    //Api
    bool hasDrivingCell(const ObjectId &port_id) const;
    SetDrivingCellPtr getDrivingCell(Pin *pin, const SetDrivingCellCondition &condition) const;
    float getInputRiseSlew(Pin* pin, const SetDrivingCellCondition &condition) const;
    float getInputFallSlew(Pin* pin, const SetDrivingCellCondition &condition) const;
    TCell* getCell(Pin* pin, const SetDrivingCellCondition &condition) const;
    TimingArc* getTimingArc(Pin* pin, const SetDrivingCellCondition &condition) const;

  private:
    DrivingCellDataPtr data_;
};
using SdcDrivingCellContainerPtr = std::shared_ptr<SdcDrivingCellContainer>;

class SdcFanoutLoadContainer {
  public:
    SdcFanoutLoadContainer() { data_ = std::make_shared<FanoutLoadContainerData>(); }

    void setData(const FanoutLoadContainerDataPtr &data) { data_ = data; }
    const FanoutLoadContainerDataPtr getData() const { return data_; }
    FanoutLoadContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcFanoutLoadContainer &rhs);

  private:
    FanoutLoadContainerDataPtr data_;
};
using SdcFanoutLoadContainerPtr = std::shared_ptr<SdcFanoutLoadContainer>;

class SdcInputTransitionContainer {
  public:
    SdcInputTransitionContainer() { data_ = std::make_shared<InputTransitionData>(); }

    void setData(const InputTransitionDataPtr &data) { data_ = data; }
    const InputTransitionDataPtr getData() const { return data_; }
    InputTransitionDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcInputTransitionContainer &rhs);

  private:
    InputTransitionDataPtr data_;
};
using SdcInputTransitionContainerPtr = std::shared_ptr<SdcInputTransitionContainer>;

class SdcLoadContainer {
  public:
    SdcLoadContainer() { data_ = std::make_shared<LoadContainerData>(); }

    void setData(const LoadContainerDataPtr &data) { data_ = data; }
    const LoadContainerDataPtr getData() const { return data_; }
    LoadContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcLoadContainer &rhs);

  private:
    LoadContainerDataPtr data_;
};
using SdcLoadContainerPtr = std::shared_ptr<SdcLoadContainer>;

class SdcLogicContainer {
  public:
    SdcLogicContainer() { data_ = std::make_shared<LogicContainerData>(); }

    void setData(const LogicContainerDataPtr &data) { data_ = data; };
    const LogicContainerDataPtr getData() const { return data_; };
    LogicContainerDataPtr getData() { return data_; };
    void forEachPortValue(const std::function<void(const ObjectId &, const SetLogic &)> &func) const {
        ContainerDataAccess::forEachObjectMap(data_->getPortLogicValue(), func);
    }
    friend std::ostream &operator<<(std::ostream &os, SdcLogicContainer &rhs);

  private:
    LogicContainerDataPtr data_;
};
using SdcLogicContainerPtr = std::shared_ptr<SdcLogicContainer>;

class SdcMaxAreaContainer {
  public:
    SdcMaxAreaContainer() { data_ = std::make_shared<MaxAreaContainerData>(); }

    void setData(const MaxAreaContainerDataPtr &data) { data_ = data; }
    const MaxAreaContainerDataPtr getData() const { return data_; }
    MaxAreaContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcMaxAreaContainer &rhs);

  private:
    MaxAreaContainerDataPtr data_;
};
using SdcMaxAreaContainerPtr = std::shared_ptr<SdcMaxAreaContainer>;

class SdcMaxCapacitanceContainer {
  public:
    SdcMaxCapacitanceContainer() { data_ = std::make_shared<MaxCapacitanceContainerData>();}

    void setData(const MaxCapacitanceContainerDataPtr &data) { data_ = data; };
    const MaxCapacitanceContainerDataPtr getData() const { return data_; };
    MaxCapacitanceContainerDataPtr getData() { return data_; };
    const SetMaxCapacitancePtr getPinCap(const ObjectId &pin_id, const ObjectId &top_cell_id) const;

    friend std::ostream &operator<<(std::ostream &os, SdcMaxCapacitanceContainer &rhs);

  private:
    MaxCapacitanceContainerDataPtr data_;
};
using SdcMaxCapacitanceContainerPtr = std::shared_ptr<SdcMaxCapacitanceContainer>;

class SdcMaxFanoutContainer {
  public:
    SdcMaxFanoutContainer() { data_ = std::make_shared<MaxFanoutContainerData>(); }

    void setData(const MaxFanoutContainerDataPtr &data) { data_ = data; }
    const MaxFanoutContainerDataPtr getData() const { return data_; }
    MaxFanoutContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcMaxFanoutContainer &rhs);

  private:
    MaxFanoutContainerDataPtr data_;
};
using SdcMaxFanoutContainerPtr = std::shared_ptr<SdcMaxFanoutContainer>;

class SdcMaxTransitionContainer {
  public:
    SdcMaxTransitionContainer() { data_ = std::make_shared<MaxTransitionContainerData>(); }

    void setData(const MaxTransitionContainerDataPtr &data) { data_ = data; }
    const MaxTransitionContainerDataPtr getData() const { return data_; }
    MaxTransitionContainerDataPtr getData() { return data_; }
    void removeClock(const ClockId &clock_id);
    friend std::ostream &operator<<(std::ostream &os, SdcMaxTransitionContainer &rhs);

  private:
    MaxTransitionContainerDataPtr data_;
};
using SdcMaxTransitionContainerPtr = std::shared_ptr<SdcMaxTransitionContainer>;

class SdcMinCapacitanceContainer {
  public:
    SdcMinCapacitanceContainer() { data_ = std::make_shared<MinCapacitanceContainerData>(); }

    void setData(const MinCapacitanceContainerDataPtr &data) { data_ = data; }
    const MinCapacitanceContainerDataPtr getData() const { return data_; }
    MinCapacitanceContainerDataPtr getData() { return data_; }
    const SetMinCapacitancePtr getPinCap(const ObjectId &pin_id, const ObjectId &top_cell_id) const;
    friend std::ostream &operator<<(std::ostream &os, SdcMinCapacitanceContainer &rhs);

  private:
    MinCapacitanceContainerDataPtr data_;
};
using SdcMinCapacitanceContainerPtr = std::shared_ptr<SdcMinCapacitanceContainer>;

class SdcOperatingConditionsContainer {
  public:
    SdcOperatingConditionsContainer() { data_ = std::make_shared<OperatingConditionsContainerData>(); }

    void setData(const OperatingConditionsContainerDataPtr &data) { data_ = data; }
    const OperatingConditionsContainerDataPtr getData() const { return data_; }
    OperatingConditionsContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcOperatingConditionsContainer &rhs);

  private:
    OperatingConditionsContainerDataPtr data_;
};
using SdcOperatingConditionsContainerPtr = std::shared_ptr<SdcOperatingConditionsContainer>;

class SdcPortFanoutNumberContainer {
  public:
    SdcPortFanoutNumberContainer() { data_ = std::make_shared<PortFanoutNumberContainerData>(); }

    void setData(const PortFanoutNumberContainerDataPtr &data) { data_ = data; }
    const PortFanoutNumberContainerDataPtr getData() const { return data_; }
    PortFanoutNumberContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcPortFanoutNumberContainer &rhs);

  private:
    PortFanoutNumberContainerDataPtr data_;
};
using SdcPortFanoutNumberContainerPtr = std::shared_ptr<SdcPortFanoutNumberContainer>;

class SdcResistanceContainer {
  public:
    SdcResistanceContainer() { data_ = std::make_shared<ResistanceContainerData>(); }

    void setData(const ResistanceContainerDataPtr &data) { data_ = data; }
    const ResistanceContainerDataPtr getData() const { return data_; }
    ResistanceContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcResistanceContainer &rhs);

  private:
    ResistanceContainerDataPtr data_;
};
using SdcResistanceContainerPtr = std::shared_ptr<SdcResistanceContainer>;

class SdcTimingDerateContainer {
  public:
    SdcTimingDerateContainer() { data_ = std::make_shared<TimingDerateContainerData>(); }

    void setData(const TimingDerateContainerDataPtr &data) { data_ = data; }
    const TimingDerateContainerDataPtr getData() const { return data_; }
    TimingDerateContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcTimingDerateContainer &rhs);

  private:
    TimingDerateContainerDataPtr data_;
};
using SdcTimingDerateContainerPtr = std::shared_ptr<SdcTimingDerateContainer>;

class SdcVoltageContainer {
  public:
    SdcVoltageContainer() { data_ = std::make_shared<VoltageContainerData>(); }

    void setData(const VoltageContainerDataPtr &data) { data_ = data; }
    const VoltageContainerDataPtr getData() const { return data_; }
    VoltageContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcVoltageContainer &rhs);

  private:
    VoltageContainerDataPtr data_;
};
using SdcVoltageContainerPtr = std::shared_ptr<SdcVoltageContainer>;

class SdcWireLoadMinBlockSizeContainer {
  public:
    SdcWireLoadMinBlockSizeContainer() { data_ = std::make_shared<WireLoadMinBlockSizeContainerData>(); }

    void setData(const WireLoadMinBlockSizeContainerDataPtr &data) { data_ = data; }
    const WireLoadMinBlockSizeContainerDataPtr getData() const { return data_; }
    WireLoadMinBlockSizeContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcWireLoadMinBlockSizeContainer &rhs);

  private:
    WireLoadMinBlockSizeContainerDataPtr data_;
};
using SdcWireLoadMinBlockSizeContainerPtr = std::shared_ptr<SdcWireLoadMinBlockSizeContainer>;

class SdcWireLoadModeContainer {
  public:
    SdcWireLoadModeContainer() { data_ = std::make_shared<WireLoadModeContainerData>(); }

    void setData(const WireLoadModeContainerDataPtr &data) { data_ = data; }
    const WireLoadModeContainerDataPtr getData() const { return data_; }
    WireLoadModeContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcWireLoadModeContainer &rhs);

  private:
    WireLoadModeContainerDataPtr data_;
};
using SdcWireLoadModeContainerPtr = std::shared_ptr<SdcWireLoadModeContainer>;

class SdcWireLoadModelContainer {
  public:
    SdcWireLoadModelContainer() { data_ = std::make_shared<WireLoadModelContainerData>(); }

    void setData(const WireLoadModelContainerDataPtr &data) { data_ = data; }
    const WireLoadModelContainerDataPtr getData() const { return data_; }
    WireLoadModelContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcWireLoadModelContainer &rhs);

  private:
    WireLoadModelContainerDataPtr data_;
};
using SdcWireLoadModelContainerPtr = std::shared_ptr<SdcWireLoadModelContainer>;

class SdcWireLoadSelectionGroupContainer {
  public:
    SdcWireLoadSelectionGroupContainer() { data_ = std::make_shared<WireLoadSelectionGroupContainerData>(); }

    void setData(const WireLoadSelectionGroupContainerDataPtr &data) { data_ = data; }
    const WireLoadSelectionGroupContainerDataPtr getData() const { return data_; }
    WireLoadSelectionGroupContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcWireLoadSelectionGroupContainer &rhs);

  private:
    WireLoadSelectionGroupContainerDataPtr data_;
};
using SdcWireLoadSelectionGroupContainerPtr= std::shared_ptr<SdcWireLoadSelectionGroupContainer>;

//multivoltage power commands
class SdcVoltageAreaContainer {
  public:
    SdcVoltageAreaContainer() { data_ = std::make_shared<VoltageAreaContainerData>(); }

  public:
    void setData(const VoltageAreaContainerDataPtr &data) { data_ = data; }
    const VoltageAreaContainerDataPtr getData() const { return data_; }
    VoltageAreaContainerDataPtr getData() { return data_; }
    const CreateVoltageAreaPtr getInstVoltageArea(const ObjectId &id) const;

    friend std::ostream &operator<<(std::ostream &os, SdcVoltageAreaContainer &rhs);

  private:
    VoltageAreaContainerDataPtr data_;
};
using SdcVoltageAreaContainerPtr = std::shared_ptr<SdcVoltageAreaContainer>;

class SdcLevelShifterStrategyContainer {
  public:
    void setData(const SetLevelShifterStrategy &data) { data_ = data; }
    const SetLevelShifterStrategy getData() const { return data_; }
    SetLevelShifterStrategy getData() { return data_; }
    const LevelShifterStrategy &getLevelShifterStrategy() const { return data_.getRule(); }
    friend std::ostream &operator<<(std::ostream &os, SdcLevelShifterStrategyContainer &rhs);

  private:
    SetLevelShifterStrategy data_;
};
using SdcLevelShifterStrategyContainerPtr = std::shared_ptr<SdcLevelShifterStrategyContainer>;

class SdcLevelShifterThresholdContainer {
  public:
    SdcLevelShifterThresholdContainer() { data_ = std::make_shared<SetLevelShifterThreshold>(); }

  public:
    void setData(const SetLevelShifterThresholdPtr &data) { data_ = data; }
    const SetLevelShifterThresholdPtr getData() const { return data_; }
    SetLevelShifterThresholdPtr getData() { return data_; }
    const float &getVoltage() const { return data_->getVoltage(); }
    const float &getPercent() const { return data_->getPercent(); }
    friend std::ostream &operator<<(std::ostream &os, SdcLevelShifterThresholdContainer &rhs);

  private:
    SetLevelShifterThresholdPtr data_;
};
using SdcLevelShifterThresholdContainerPtr = std::shared_ptr<SdcLevelShifterThresholdContainer>;

class SdcMaxDynamicPowerContainer {
  public:
    SdcMaxDynamicPowerContainer() { data_ = std::make_shared<MaxDynamicPowerContainerData>(); }

  public:
    void setData(const MaxDynamicPowerContainerDataPtr &data) { data_ = data; }
    const MaxDynamicPowerContainerDataPtr getData() const { return data_; }
    MaxDynamicPowerContainerDataPtr getData() { return data_; }
    const float getCellPower(const ObjectId &cell_id) const;

    friend std::ostream &operator<<(std::ostream &os, SdcMaxDynamicPowerContainer &rhs);

  private:
    MaxDynamicPowerContainerDataPtr data_;
};
using SdcMaxDynamicPowerContainerPtr = std::shared_ptr<SdcMaxDynamicPowerContainer>;

class SdcMaxLeakagePowerContainer {
  public:
    SdcMaxLeakagePowerContainer() { data_ = std::make_shared<MaxLeakagePowerContainerData>(); }

    const float getCellPower(const ObjectId &cell_id) const;
    void setData(const MaxLeakagePowerContainerDataPtr &data) { data_ = data; }
    const MaxLeakagePowerContainerDataPtr getData() const { return data_; }
    MaxLeakagePowerContainerDataPtr getData() { return data_; }
    friend std::ostream &operator<<(std::ostream &os, SdcMaxLeakagePowerContainer &rhs);

  private:
    MaxLeakagePowerContainerDataPtr data_;
};
using SdcMaxLeakagePowerContainerPtr = std::shared_ptr<SdcMaxLeakagePowerContainer>;

//object access commands
class SdcAllClocksContainer {
  public:
    SdcAllClocksContainer() { data_ = std::make_shared<SdcClockContainer>(); }

    void setData(const SdcClockContainerPtr &data) { data_ = data; };
    const SdcClockContainerPtr getData() const { return data_; };
    SdcClockContainerPtr getData() { return data_; };
    void get_all_clock_names(std::vector<std::string> &names) const { return data_->getClockNames(names); };

  private:
    SdcClockContainerPtr data_;
};
using SdcAllClocksContainerPtr = std::shared_ptr<SdcAllClocksContainer>;

class SdcAllInputsContainer {
  public:
    SdcAllInputsContainer() { data_ = std::make_shared<SdcClockContainer>(); }

    void setData(const SdcClockContainerPtr &data) { data_ = data; };
    const SdcClockContainerPtr getData() const { return data_; };
    SdcClockContainerPtr getData() { return data_; };
    const std::vector<std::string> &get_all_inputs(const AllInputsPtr &conditions) const;

  private:
    SdcClockContainerPtr data_;
};
using SdcAllInputsContainerPtr = std::shared_ptr<SdcAllInputsContainer>;

class SdcAllOutputsContainer {
  public:
    SdcAllOutputsContainer() { data_ = std::make_shared<SdcClockContainer>(); }

    const std::vector<std::string> &get_all_outputs(const AllOutputsPtr &conditions) const;

  private:
    SdcClockContainerPtr data_;
};
using SdcAllOutputsContainerPtr = std::shared_ptr<SdcAllOutputsContainer>;

class SdcAllRegistersContainer {
  public:
    SdcAllRegistersContainer() { data_ = std::make_shared<SdcClockContainer>(); }

    const std::vector<std::string> &get_all_registers(const AllRegistersPtr &conditions) const;

  private:
    SdcClockContainerPtr data_;
};
using SdcAllRegistersContainerPtr = std::shared_ptr<SdcAllRegistersContainer>;

class SdcCurrentDesignContainer {
  public:
    SdcCurrentDesignContainer() { data_ = std::make_shared<CurrentDesign>(); }

    void setData(const CurrentDesignPtr &data) { data_ = data; };
    const CurrentDesignPtr getData() const { return data_; };
    CurrentDesignPtr getData() { return data_; };

    const std::string getDesignName() const;
    const ObjectId getDesignId() const{ return data_->getCellId(); };

  private:
    CurrentDesignPtr data_;
};
using SdcCurrentDesignContainerPtr = std::shared_ptr<SdcCurrentDesignContainer>;

class SdcGetCellsContainer {
  public:
    const std::vector<std::string> &get_cells_names(const GetCellsPtr &conditions) const;

  private:
    SdcCurrentDesignContainer current_design_;
    SdcCurrentInstanceContainer current_instance_;
};
using SdcGetCellsContainerPtr = std::shared_ptr<SdcGetCellsContainer>;

class SdcGetClocksContainer {
  public:
    const std::vector<std::string> &get_clocks_names(const GetClocksPtr &conditions) const;
};
using SdcGetClocksContainerPtr = std::shared_ptr<SdcGetClocksContainer>;

class SdcGetLibCellsContainer {
  public:
    const std::vector<std::string> &get_lib_cells_names(const GetLibCellsPtr &conditions) const;
};
using SdcGetLibCellsContainerPtr = std::shared_ptr<SdcGetLibCellsContainer>;

class SdcGetLibPinsContainer {
  public:
    const std::vector<std::string> &get_lib_pins_names(const GetLibPinsPtr &conditions) const;
};
using SdcGetLibPinsContainerPtr = std::shared_ptr<SdcGetLibPinsContainer>;

class SdcGetLibsContainer {
  public:
    const std::vector<std::string> &get_libs_names(const GetLibsPtr &conditions) const;
};
using SdcGetLibsContainerPtr = std::shared_ptr<SdcGetLibsContainer>;

class SdcGetNetsContainer {
  public:
    const std::vector<ObjectId> &get_nets_ids(const GetNetsPtr &conditions) const;
    const std::vector<std::string> &get_nets_names(const GetNetsPtr &conditions) const;
};
using SdcGetNetsContainerPtr = std::shared_ptr<SdcGetNetsContainer>;

class SdcGetPinsContainer {
  public:
    const std::vector<ObjectId> &get_pins_ids(const GetPinsPtr &conditions) const;
    const std::vector<std::string> &get_pins_names(const GetPinsPtr &conditions) const;
};
using SdcGetPinsContainerPtr = std::shared_ptr<SdcGetPinsContainer>;

class SdcGetPortsContainer {
  public:
    const std::vector<ObjectId> &get_ports_ids(const GetPortsPtr &conditions) const;
    const std::vector<std::string> &get_ports_names(const GetPortsPtr &conditions) const;
};
using SdcGetPortsContainerPtr = std::shared_ptr<SdcGetPortsContainer>;





}
}
#endif //EDI_DB_TIMING_SDC_SDC_DATA_H_
