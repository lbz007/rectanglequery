/**
 * File:    qplacer.h
 *
 * Class and functions for qplacer.
 *
 * Defines a class "QPlaceDB" to store description of an ASIC. This class is
 * used to recursively place the insts in the ASIC into desirable locations.
 * The algorithm described here is called Quadratic Placement. The reference of
 * the theory is available on (https://www.coursera.org/course/vlsicad).
 * The solver used in this file is obtained from "solver.h" which was proIntVded
 * by the same course material. It uses conjugate gradient method to iteratively
 * solve a sparse matrix of the form Ax=b to find x.
 * The final coordinates of the insts are written back to openEDI DB
 *
 * Author:  Fly
 */
#ifndef EDI_QUADRATIC_H_
#define EDI_QUADRATIC_H_

#include<iostream>
#include<fstream>
#include<cstdlib>
#include<string>
#include<map>
#include<vector>
#include<set>
#include <cmath>
#include <valarray>
#include <cstdio>
#include <unordered_map>
#include "solver.h"
#include "utility/src/Msg.h"
#include "flow/src/common_place_DB.h"

DREAMPLACE_BEGIN_NAMESPACE
#define MAX_NET_DEGREE 100

using namespace std;
typedef vector<int>          IntV;
typedef vector<float>        FloatV;
typedef vector<PlInst*>      InstV;
typedef set<int>             IntS;
typedef valarray<float>      FVA;

typedef pair<float, float>   FloatP;
typedef pair<FloatV, FloatV> VFP;
typedef pair<IntV, IntV>     VIP;

typedef map<int, float>      LocM;
typedef map<int, IntV>       InstM;
typedef map<int, FloatV>     PadM;
typedef map<int, VIP>        NetM;
typedef map<Plong, float>    MatrixM;

/*
 * Class which defines an ASIC and its components.
 */
class QPlaceDB
{
  public:
  /* Constructor of the class 'QPlaceDB' */
  QPlaceDB()  = default;
  ~QPlaceDB() = default;

  PlBox& getBox()                   { return box_;        }
  float  getLocX(int key)           { return instX_[key]; }
  float  getLocY(int key)           { return instY_[key]; }

  void   setBox(PlBox& box)         { box_ = box;         }
  void   setLocX(int key, float v)  { instX_[key] = v;    }
  void   setLocY(int key, float v)  { instY_[key] = v;    }
  /*
   * Function to return the number of instsi/pads/nets.
   * @return The number of insts, pads, nets in the ASIC.
   */
  int    getNumI()                  { return this->insts_.size(); }
  int    getNumP()                  { return this->pads_.size();  }
  int    getNumN()                  { return this->nets_.size();  }
  /*
   * Function to return inst/pad/net keys.
   * @return A vector with the inst/pad/net id of insts in the ASIC.
   */
  InstV& getInstPtrs()              { return instPtrs_; }
  IntV&  getInstKeys()              { return insts_;    }
  IntV&  getPadKeys()               { return pads_;     }
  IntV&  getNetKeys()               { return nets_;     }
  /*
   * Function to return inst coordinates.
   * @param instNum The inst-id for which coordinates are needed.
   * @return The x, y coordinates of the inst with inst-id 'instNum'.
   */
  FloatP getInstCoords(int instNum)
  {
    FloatP v(this->instX_[instNum], this->instY_[instNum]); // x, y coordinate
    return v;
  }

  /*
   * Function to return connections of a inst.
   * @param instNum The inst-id of the inst for which connections are needed.
   * @return A vector of the net-ids connected to the inst with inst-id 'instNum'.
   */
  IntV& getInstconnections(int instNum) 
  {
    return this->instNets_[instNum];
  }

  /*
   * Function to make a new inst and adds list of connections.
   * @param instNum The inst-id of the inst to be added.
   * @param listofconnections A vector of net-ids connected to the inst to
   * be added.
   */
  void addInst(int instNum, IntV& listofconnections) 
  {
    this->insts_.push_back(instNum);
    this->instNets_[instNum] = listofconnections;
  }

  void addInst(int instNum) 
  {
    this->insts_.push_back(instNum);
  }

  void addInstPtr(PlInst* inst) 
  {
    this->instPtrs_.push_back(inst);
  }

  /*
   * Function to return pad coordinates.
   * @param padNum The pad-id of the pad for which coordinates are needed.
   * @return The x, y coordinates of the pad with pad-id 'padNum'.
   */
  FloatP getPadCoords(int padNum)
  {
    FloatP v;
    const auto iter = this->padLocs_.find(padNum);
    if (iter != this->padLocs_.end()) {
      v.first = (iter->second)[1];
      v.second = (iter->second)[2];
    }
    return v;
  }

  /*
   * Function to make a new pad and adds its connections and location
   * @param padNum The pad-id of the pad to be added.
   * @param netandlocation A vector with net-id connected to the pad to
   * be added, and its x, y coordinate.
   */
  void addPad(int padNum, FloatV& netandlocation)
  {
    this->pads_.push_back(padNum);
    this->padLocs_[padNum] = netandlocation;
  }

  /*
   * Function to return number of net connections
   * @param netNum The net-id of the net for which the information is desired.
   * @return Total number of insts and pads, combined, connected to a net of
   * net-id 'netNum'.
   */
  int getNumNetConns(int netNum)
  {
    return (netConns_[netNum].first.size() + netConns_[netNum].second.size());
  }

  /*
   * Function to return inst connections to a net
   * @param netNum The net-id of the net for which the information is desired.
   * @return A vector of inst-ids of the insts connected to the net of
   * net-id 'netNum'.
   */
  IntV& getNetInstConns(int netNum) 
  {
    return netConns_[netNum].first;
  }

  /*
   * Function to return pad connections to a net
   * @param netNum The net-id of the net for which the information is desired.
   * @return A vector of pad-ids of the pads connected to the net of
   * net-id 'netNum'.
   */
  IntV& getNetPadConns(int netNum)
  {
    return netConns_[netNum].second;
  }
  
  /*
   * Function which makes a new net, if needed, and appends a connection to the net 'netnum'
   * @param netNum The net-id of the net to be added.
   * @param connection The inst-id/ pad-id of the inst/pad to which the net is
   * connected to.
   * @param instorpad It shows if the given connection is a inst or a pad. It
   * is 0 for inst, 1 for pad.
   */
  void addNet(int netNum, int connection, bool isPad)
  {
    // 0 for inst, 1 for pad and fix inst
    // if netnum doesn't already exist in dictionary
    if (this->netConns_.find(netNum) == this->netConns_.end()) {
      VIP empty_netconn; //(insts, pads);
      this->nets_.push_back(netNum);
      this->netConns_.insert(make_pair(netNum, empty_netconn));
    }

    if (isPad) {
      this->netConns_[netNum].second.push_back(connection);
    } else {
      this->netConns_[netNum].first.push_back(connection);
    }
  }

  /*
   * Function to add location values for given inst keys
   * @param x A vector containing x-coordinates of insts in the same order as
   * the inst-ids in the vector 'instKeys'
   * @param y A vector containing y-coordinates of insts in the same order as
   * the inst-ids in the vector 'instKeys'
   * @param instKeys A vector containing inst-ids of the insts for which
   * location is given and to be updated.
   * @param bound The minimum and maximum values of the x, y coordinates
   * desired. It is an array of 4 numbers, [x_min, x_max, y_min, y_max]. The 
   * argument is not necessarily used.
   * @return false if there is a mismatch amongst sizes of 'x', 'y', and 'instKeys'.
   */
  bool addLocation(FloatV& x, FloatV& y, IntV& instKeys)
  {
    if ((instKeys.size() == x.size()) && (x.size() == y.size())) {
      for (int l = 0; l < instKeys.size(); ++l)
      {
        this->instX_[instKeys[l]] = x[l];
        this->instY_[instKeys[l]] = y[l];
      }
      return true;
    } else {
      return false;
    }
  }

  void addLocation(FloatV& x, FloatV& y, IntS& instKeys)
  {
    int l = 0;
    for (auto& inst : instKeys)
    {
      this->instX_[inst] = x[l];
      this->instY_[inst] = y[l];
      l++;
    }
  }
  /*
   * Function to get the location values for given inst keys
   * @param instKeys A vector containing inst-ids of the insts for which
   * location data is needed.
   * @return A vector of x, y coordinates for each inst-id in vector 'instKeys'
   * , in the same order as the inst-ids in 'instKeys'. 
   */
  void getLocations(VFP& returnvec, IntV& instKeys)
  {
    int size = instKeys.size();
    FloatV xloc(size, 0);
    FloatV yloc(size, 0);
    for (int l = 0; l < size; ++l)
    {
      xloc[l] = this->instX_[instKeys[l]];
      yloc[l] = this->instY_[instKeys[l]];
    }
    returnvec = make_pair(xloc, yloc);
  }
  /*
   * Function to print the locations of all insts in the present core.
   */
  void printAllInsts(int num)
  {
    IntV& keyI = getInstKeys();
    int maxNum = std::min(this->getNumI(), num);
    cout << "Locations:";
    for (int i = 0; i < maxNum; ++i)
    { 
      cout << endl;
      cout << "Inst " << keyI[i] << ": " << this->instX_[keyI[i]]
           << ", " << this->instY_[keyI[i]];
    }
    cout << endl;
  }
  /*
   * Function to print the pads in the present core.
   */
  void printAllPads(int num)
  {
    IntV& keyP = this->getPadKeys();
    int maxNum = std::min(this->getNumP(), num);
    cout << "Pads:";
    for (int i = 0; i < maxNum; ++i)
    { 
      int padnum = keyP[i];
      cout << endl;
      cout << "Pad " << padnum << ": Net - " << this->padLocs_[padnum][0] << ", Location - ("
           << this->padLocs_[padnum][1] << ", " << this->padLocs_[padnum][2] << ")";
    }
    cout << endl;
  }

  void collectNetGroups(int netK, IntS& nets, IntS& insts, IntS& pads);

  private:
    PlBox  box_;
    InstV  instPtrs_;
    IntV   insts_;              /* inst id */
    InstM  instNets_;           /* A map storing informations about insts 
                                in the ASIC. It lists the net ids connected to 
                                each inst. */
    IntV   pads_;
    PadM   padLocs_;            /* A map storing informations about pads 
                                in the ASIC. It lists the net connected to each
                                pad, x and y coordinate of the pad. Assuming each
                                pad is connected to only one net. */
    IntV   nets_;
    NetM   netConns_;           /* A map storing informations about nets 
                                in the ASIC. One vector consists of the inst ids
                                connected by this net. Other vector consists of 
                                the net ids connected by this net. */
    LocM   instX_;              /* A map storing x-coordinates of insts in the ASIC. */
    LocM   instY_;              /* A map storing y-coordinates of insts in the ASIC. */
};

/*
 * Function to sort 2 insts by locations.
 */
inline bool
sortCmpXY(const int a , const int b, const int hORv, QPlaceDB* mc) 
{
  FloatP locA = mc->getInstCoords(a);
  FloatP locB = mc->getInstCoords(b);
  if (0 == hORv) {
    if(locA.first == locB.first) {
      return (locA.second <= locB.second);
    } else {
      return (locA.first < locB.first);
    }
  } else {
    if(locA.second == locB.second) {
      return (locA.first <= locB.first);
    } else {
      return (locA.second < locB.second);
    }
  }
  return true;
}

/*
 * Class for quadratic placer.
 */
class QPlacer
{
  public:
    QPlacer()
    {
      mc_ = new QPlaceDB();
    }
    QPlacer(int partitionNum) : par_num_(partitionNum)
    {
      mc_ = new QPlaceDB();
    }
    ~QPlacer()
    {
      delete mc_;
      mc_ = nullptr;
    }
    QPlacer(const QPlacer&)              = delete; // no copy
    QPlacer &operator=(const QPlacer&)   = delete; // no copy

    class Sorter
    {
      public:
        Sorter(const int hORv) : hORv_(hORv) {}
        bool operator()(const int a, const int b) 
        {
          return sortCmpXY(a, b, hORv_, mc_);
        }
      private:
        int hORv_ = 0;
    };

    bool isGPU()                const { return gpu_;        }
    int  getPartitionNum()      const { return par_num_;    }

    bool createCoreBox();
    void createCoreDB();
    void updateToDB();
    void assign(IntV& instKeys, int hORv, VIP& returnPairs);
    void updateCoordinates(FloatP* padlocation, PlBox& box);
    void containNrun(VFP& locs, IntV& instKeys, PlBox& box, int hORv, int lORr);
    void solve(const IntV& R, const IntV& C, const FloatV& V, const FloatV& ba, FloatV& aout);
    void solve(CooMatrix& A, const FVA& ba, FloatV& aout, const int min, const int max);
    void solveForNetGroup(QPlaceDB *core, IntS& nets, IntS& insts, IntS& pads);
    bool solveForX(QPlaceDB *core);
    bool solveForLoc(QPlaceDB *core);
    void place(IntV& instKeys, PlBox& box, int n);
    void placePartitions();
    void placeCore();
    void run();  // flow run
  private:
    bool             gpu_       = false; 
    int              par_num_   = 0;
    static QPlaceDB* mc_;
};

DREAMPLACE_END_NAMESPACE

#endif // EDI_QUADRATIC_H_
