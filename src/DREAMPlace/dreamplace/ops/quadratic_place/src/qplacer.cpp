/**
 * File:    qplacer.cpp
 *
 * Class functions implementations for qpPlacer.
 *
 * Author:  Fly 
 */

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

#include "qplacer.h"
#include "solver.h"

DREAMPLACE_BEGIN_NAMESPACE

using namespace std;
/*
 * Function to create net group to run qp solver.
 * @param nets, insts, pads.
 */
void
QPlaceDB::collectNetGroups(int netK, IntS& nets, IntS& insts, IntS& pads)
{
  if (nets.find(netK) == nets.end() && 
      getNumNetConns(netK) < MAX_NET_DEGREE) 
  {
    nets.insert(netK);

    IntV& padKeys = getNetPadConns(netK);
    for (auto& padK : padKeys) 
    {
      pads.insert(padK);
    }

    IntV& instKeys = getNetInstConns(netK);
    for (auto& instK : instKeys) 
    {
      insts.insert(instK);
      IntV& tmpNs = getInstconnections(instK);
      for (auto& tmpK : tmpNs)
      {
        collectNetGroups(tmpK, nets, insts, pads);
      }
    }
  }
}

// initialize static class member 
QPlaceDB* QPlacer::mc_ = nullptr;
/*
 * Function to creates an object of class "QPlaceDB" from openEDI DB
 */
bool 
QPlacer::createCoreBox() 
{
  PlBox cBox;
  PlRow* row = nullptr;
  forEachRows(row) {
    PlBox box = getRowBox(row);
    if (isBoxInvalid(box)) continue;
    cBox.maxBox(box);
  } endForEachRows

  if (isBoxInvalid(cBox)) {
    cBox = getCoreBox();
  }

  if (isBoxInvalid(cBox)) {
    PlPolygon* poly = getCorePolygon();
    PlInt numPoly = getPolygonNumPoint(poly);
    if (numPoly >= 2) {   // area needs 2 points at least
      setBox(cBox, poly->getPoint(0).getX(), poly->getPoint(0).getY(),
                   poly->getPoint(1).getX(), poly->getPoint(1).getY());
      for (int i = 2; i < numPoly; ++i) 
      {
        if (poly->getPoint(i).getX() < getBoxLLX(cBox)) {
          setBoxLLX(cBox, poly->getPoint(i).getX());
        } else if (poly->getPoint(i).getX() > getBoxURX(cBox)) {
          setBoxURX(cBox, poly->getPoint(i).getX());
        }
        if (poly->getPoint(i).getY() < getBoxLLY(cBox)) {
          setBoxLLY(cBox, poly->getPoint(i).getY());
        } else if (poly->getPoint(i).getY() > getBoxURY(cBox)) {
          setBoxURY(cBox, poly->getPoint(i).getY());
        }
      }
    }
  }
  if (isBoxInvalid(cBox)) {
    cout << "No valid core area found." << endl;
    return false;
  }
  mc_->setBox(cBox);
  return true;
}
/*
 * Function to create QPlaceDB for core area
 */
void 
QPlacer::createCoreDB()
{
  cout << "Creating data structures." << endl;

  std::unordered_map<PlNet*, int> netIds;
  int idx = 0;
  PlNet* net = nullptr;
  forEachNets(net) {
    netIds.insert(std::make_pair(net, idx));
    idx++;
  } endForEachNets

  idx = 0;
  PlInst* inst = nullptr;
  forEachInsts(inst) {
    if (isInstMoveable(inst) &&
        getInstNumPins(inst) > 0) {
      std::set<int> nets;
      forEachInstPins(inst) {
        PlNet* net = getPinNet(pin);
        if (net) {
          nets.insert(netIds[net]);
        }
      } endForEachInstPins

      if (!nets.empty()) {
        PlBox box = getInstBox(inst);
        mc_->setLocX(idx, getBoxLLX(box) + getBoxWidth(box)/2);
        mc_->setLocY(idx, getBoxLLY(box) + getBoxHeight(box)/2);

        IntV instIdV;
        for(auto& id : nets)
        {
          instIdV.push_back(id);
          mc_->addNet(id, idx, false);
        }
        mc_->addInst(idx, instIdV);
        mc_->addInstPtr(inst);
        idx++;
      }
    }
  } endForEachInsts

  // fixed instances as pad
  idx = 0;
  forEachInsts(inst) {
    if (!isInstMoveable(inst) &&
        getInstNumPins(inst) > 0) {
      forEachInstPins(inst) {
        PlNet* net = getPinNet(pin);
        if (!net) continue;
        const int netId = netIds[net];
        mc_->addNet(netId, idx, true);

        FloatV padV;
        PlBox box = getPin1Box(pin);
        padV.push_back(static_cast<float>(netId));
        padV.push_back(getBoxLLX(box)+getBoxWidth(box)/2);
        padV.push_back(getBoxLLY(box)+getBoxHeight(box)/2);
        mc_->addPad(idx, padV);
        idx++;
      } endForEachInstPins
    }
  } endForEachInsts

  // IO pin as pad
  PlPin* pin = nullptr;
  forEachIOPins(pin) {
    PlNet* net = getPinNet(pin);
    if (!net) continue;
    const int netId = netIds[net];
    mc_->addNet(netId, idx, true);
    FloatV padV;
    PlBox box = getPin1Box(pin);
    padV.push_back(static_cast<float>(netId));
    padV.push_back(getBoxLLX(box)+getBoxWidth(box)/2);
    padV.push_back(getBoxLLY(box)+getBoxHeight(box)/2);
    //cout << "IO: " << getBoxLLX(box)+getBoxWidth(box)/2 << " " << getBoxLLY(box)+getBoxHeight(box)/2 << endl;
    mc_->addPad(idx, padV);
    idx++;
  } endForEachIOPins

  cout << "Core box: " << getBoxLLX(mc_->getBox()) << " " << getBoxLLY(mc_->getBox()) 
       << " " << getBoxURX(mc_->getBox()) << " " << getBoxURY(mc_->getBox()) << endl;
  cout << "Create date structures done, Added " << mc_->getNumI() << " insts, " 
       << mc_->getNumP() << " pads, " << mc_->getNumN() << " nets." << endl;
}

/*
 * Function to update new locations to openEDI DB
 */
void 
QPlacer::updateToDB()
{
  cout << "Update new locations to DB." << endl;
  // only update moveable instances
  IntV& iKey = mc_->getInstKeys();
  InstV& iPtr = mc_->getInstPtrs();
  PlBox& cBox = mc_->getBox();

  int size = iKey.size();
  for (int i = 0; i < size; ++i)
  {
    PlInst* inst = iPtr[i];
    int key = iKey[i];
    PlBox box = getInstBox(inst);
    PlPoint loc(round(mc_->getLocX(key)-getBoxWidth(box)/2),
                round(mc_->getLocY(key)-getBoxHeight(box)/2));

    // adjust loc based on core boundary
    if (loc.getX() < getBoxLLX(cBox)) {
      loc.setX(getBoxLLX(cBox));
    } else if (loc.getX() > getBoxURX(cBox) - getBoxWidth(box)) {
      loc.setX(getBoxURX(cBox) - getBoxWidth(box));
    }
    if (loc.getY() < getBoxLLY(cBox)) {
      loc.setY(getBoxLLY(cBox));
    } else if (loc.getY() > getBoxURY(cBox) - getBoxHeight(box)) {
      loc.setY(getBoxURY(cBox) - getBoxHeight(box));
    }

    // update location and status
    setInstLoc(inst, loc);
    setInstPStatus(inst, kPlStatus::kPlaced);
  }

}

/*
 * Function which solves a sparse matrix, of the form Ax=b, using CooMatrix 
 * class from solver.h.
 * @param R Vector containing non-zero row values of the matrix A, in order.
 * @param C Vector containing non-zero column values of the matrix A, in order.
 * @param V Vector containing non-zero values of the matrix A, in order.
 * @param ba Vector containing the b vector in the matrix form Ax=b.
 * @return vector containing values of the solved vector x in the form Ax=b.
 */
void
QPlacer::solve(const IntV& R, const IntV& C, const FloatV& V, const FloatV& ba, FloatV& aout)
{
  CooMatrix A;

  A.n_ = ba.size();
  A.nnz_ = R.size();
  A.row_.resize(A.nnz_);
  A.col_.resize(A.nnz_);
  A.dat_.resize(A.nnz_);

  A.row_ = valarray<int>(R.data(), R.size());
  A.col_ = valarray<int>(C.data(), C.size());
  A.dat_ = valarray<float>(V.data(), V.size());

  valarray<float> x(A.n_);
  valarray<float> b(ba.data(), A.n_);

  A.solve(b, x);

  aout.assign(begin(x), end(x)); 
}

/*
 * Function to solves a sparse matrix, of the form Ax=b, using CooMatrix
 */
void
QPlacer::solve(CooMatrix& A, const FVA& ba, FloatV& aout,
    const int min, const int max)
{
  valarray<float> x(A.n_);
  A.solve(ba, x, min, max);

  aout.assign(begin(x), end(x)); 
}
/*
 * Function which generates the A matrix and b vector for each coordinate, from
 * an object of the class QPlaceDB and sends it to solve for solving. The
 * result is written back to the QPlaceDB object.
 * @param core Pointer to an object of class "QPlaceDB".
 * @param bound The minimum and maximum values of the x, y coordinates
 * desired. It is an array of 4 numbers, [x_min, x_max, y_min, y_max].
 * @return True if there are no errors, false otherwise.
 */
bool 
QPlacer::solveForLoc(QPlaceDB *core)
{
  cout << "Solving for locations ..." << endl;
  hr_clock_rep start_time = get_globaltime();
  // Initializations
  IntV& keyI = core->getInstKeys();
  IntV& keyP = core->getPadKeys();
  IntV& keyN = core->getNetKeys();
  const int numI = keyI.size();
  const int numP = keyP.size();
  const int numN = keyN.size();

  if (numI == 0 || numP == 0 || numN == 0) {
    return false;
  }

  // Calculating weights using number of connections of nets
  cout << "Calculating weights ..." << endl;
  map <int, float> weights;
  for(int i = 0; i < numN; ++i)
  {
    int netval = keyN[i];
    int cNum = core->getNumNetConns(netval);
    if (cNum > 1 && cNum < MAX_NET_DEGREE) {
      weights[netval] = 1.0/(cNum-1);
      //cout << "getid " << netval << " " << cNum << " " << weights[netval] << endl;
    }
  }

  // Inst numbers may vary, this dictionary keeps them in order
  map <int, int> instOrder;
  for(int i = 0; i < numI; ++i)
  {
    instOrder[keyI[i]] = i;
  }

  // Calculating C and A matrices, bx and by vectors
  cout << "Calculating valid contributions to the cost function ..." << endl;
  MatrixM AA;
  FVA bx(0.0, numI);
  FVA by(0.0, numI);
  for(int k = 0; k < numN; ++k)
  {
    int netval = keyN[k];
    auto iter = weights.find(netval);
    if (iter == weights.end() ||
        0 == iter->second) continue;
    float weight = iter->second;
    // insts
    IntV& insts = core->getNetInstConns(netval);
    int numInsts = insts.size();

    if (numInsts > 1) {
      int i = 0;
      while (i < numInsts-1) {
        int ii = instOrder[insts[i]];
        int j = i+1;
        while (j < numInsts) {
          int jj = instOrder[insts[j]];
          Plong key = static_cast<Plong>(ii) * numI + jj;
          auto iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, -weight));
          } else {
            iter->second -= weight; 
          }
          key = static_cast<Plong>(jj) * numI + ii;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, -weight));
          } else {
            iter->second -= weight; 
          }
          // for diagonal
          key = static_cast<Plong>(ii) * numI + ii;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, weight));
          } else {
            iter->second += weight; 
          }
          key = static_cast<Plong>(jj) * numI + jj;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, weight));
          } else {
            iter->second += weight; 
          }
          j++;
        }
        i++;
      }

      // pad(IOs)
      IntV& pads = core->getNetPadConns(netval);
      int numPads = pads.size();
      if (numPads > 0) {
        int i = 0;
        while (i < numInsts) {
          int ii = instOrder[insts[i]];
          Plong key = static_cast<Plong>(ii) * numI + ii;
          auto iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, numPads * weight));
          } else {
            iter->second += (numPads * weight); 
          }
          int j = 0;
          while (j < numPads) {
            FloatP padCoordinate = core->getPadCoords(pads[j]);
            bx[ii] += (weight * padCoordinate.first);
            by[ii] += (weight * padCoordinate.second);
            j++;
          }
          i++;
        }
      }
    }
  }

  // Derive R, C, V matrices for sparse matrix generation
  cout << "Matrix size: " <<  AA.size() << ", forming R, C, V matrices ..." << endl;

  CooMatrix CA(bx.size(), AA.size());
  int idx = 0;
  for (auto& kv : AA)
  {
    int i = kv.first / numI;
    int j = kv.first % numI;
    CA.row_[idx] = i;
    CA.col_[idx] = j;
    CA.dat_[idx] = kv.second;
    idx++;
  }
  // Solve for x and y vectors
  cout << "Solving for x and y ..." << endl;
  PlBox cBox = core->getBox();
  FloatV locX;
  //solve(Rsm, Csm, Vsm, bx, locX);
  solve(CA, bx, locX, getBoxLLX(cBox), getBoxURX(cBox));
  FloatV locY;
  //solve(Rsm, Csm, Vsm, by, locY);
  solve(CA, by, locY, getBoxLLY(cBox), getBoxURY(cBox));

  hr_clock_rep stop_time = get_globaltime();
  dreamplacePrint(kINFO, "Solver time: %g ms\n", get_timer_period()*(stop_time - start_time));

  if (!core->addLocation(locX, locY, keyI)) {
    cout<<"Error in adding new locations."<<endl;
    return false;
  } else {
    cout<<"Successfully added new locations."<<endl;
    return true;
  }
}

bool 
QPlacer::solveForX(QPlaceDB *core)
{
  cout << "Solving for locations ..." << endl;
  hr_clock_rep start_time = get_globaltime();
  // Initializations
  IntV& keyN = core->getNetKeys();
  const int numN = keyN.size();
  if (numN == 0) return false;

  //collect net group
  IntS nets;
  IntS insts;
  IntS pads;
  IntS allNets;
  for(int i = 0; i < numN; ++i) 
  {
    int netK = keyN[i];
    if (allNets.find(netK) != allNets.end()) continue;
    nets.clear();
    insts.clear();
    pads.clear();
    core->collectNetGroups(netK, nets, insts, pads);
    if (insts.size() > 1) {
      solveForNetGroup(core, nets, insts, pads);
    }
    for (auto& net : nets) 
    {
      allNets.insert(net);
    }
  }
  hr_clock_rep stop_time = get_globaltime();
  dreamplacePrint(kINFO, "Solver time: %g ms\n", get_timer_period()*(stop_time - start_time));
  return true;
}

void
QPlacer::solveForNetGroup(QPlaceDB *core, IntS& nets, IntS& insts, IntS& pads)
{
  // Calculating weights using number of connections of nets
  cout << "Calculating weights for net group: " << nets.size()
       << " " << insts.size() << " " << pads.size() << endl;
  map <int, float> weights;
  for(auto& net : nets)
  {
    int cNum = core->getNumNetConns(net);
    if (cNum > 1 && cNum < MAX_NET_DEGREE) {
      weights[net] = 1.0/(cNum-1);
    }
  }
  // Inst numbers may vary, this dictionary keeps them in order
  int numI = 0;
  map <int, int> instOrder;
  for(auto& inst : insts)
  {
    instOrder[inst] = numI++;
  }

  // Calculating C and A matrices, bx and by vectors
  MatrixM AA;
  FVA bx(0.0, numI);
  FVA by(0.0, numI);
  for(auto& net : nets)
  {
    auto iter = weights.find(net);
    if (iter == weights.end() ||
        0 == iter->second) continue;
    float weight = iter->second;
    // insts
    IntV& insts = core->getNetInstConns(net);
    int numInsts = insts.size();
    if (numInsts > 1) {
      int i = 0;
      while (i < numInsts-1) {
        int ii = instOrder[insts[i]];
        int j = i+1;
        while (j < numInsts) {
          int jj = instOrder[insts[j]];
          Plong key = static_cast<Plong>(ii) * numI + jj;
          auto iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, -weight));
          } else {
            iter->second -= weight; 
          }
          key = static_cast<Plong>(jj) * numI + ii;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, -weight));
          } else {
            iter->second -= weight; 
          }
          // for diagonal
          key = static_cast<Plong>(ii) * numI + ii;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, weight));
          } else {
            iter->second += weight; 
          }
          key = static_cast<Plong>(jj) * numI + jj;
          iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, weight));
          } else {
            iter->second += weight; 
          }
          j++;
        }
        i++;
      }

      // pad(IOs)
      IntV& pads = core->getNetPadConns(net);
      int numPads = pads.size();
      if (numPads > 0) {
        int i = 0;
        while (i < numInsts) {
          int ii = instOrder[insts[i]];
          Plong key = static_cast<Plong>(ii) * numI + ii;
          auto iter = AA.find(key);
          if (iter == AA.end()) {
            AA.insert(make_pair(key, numPads * weight));
          } else {
            iter->second += (numPads * weight); 
          }
          int j = 0;
          while (j < numPads) {
            FloatP padCoordinate = core->getPadCoords(pads[j]);
            bx[ii] += (weight * padCoordinate.first);
            by[ii] += (weight * padCoordinate.second);
            j++;
          }
          i++;
        }
      }
    }
  }

  // Derive R, C, V matrices for sparse matrix generation
  cout << "Matrix size: " <<  AA.size() << ", forming R, C, V matrices ..." << endl;

  CooMatrix CA(bx.size(), AA.size());
  int idx = 0;
  for (auto& kv : AA)
  {
    int i = kv.first / numI;
    int j = kv.first % numI;
    CA.row_[idx] = i;
    CA.col_[idx] = j;
    CA.dat_[idx] = kv.second;
    idx++;
  }
  // Solve for x and y vectors
  PlBox cBox = core->getBox();
  FloatV locX;
  solve(CA, bx, locX, getBoxLLX(cBox), getBoxURX(cBox));
  FloatV locY;
  solve(CA, by, locY, getBoxLLY(cBox), getBoxURY(cBox));

  core->addLocation(locX, locY, insts);
}

/**
 * Function which sorts given insts according to their locations, horizontally 
 * or vertically. Horizontally if hORv = 0; Vertically if hORv = 1
 * @param core Pointer to an object of class "QPlaceDB".
 * @param instKeys Vector of inst-ids of insts that need to be sorted.
 * @param hORv It is used to decide if the sorting is done based on x-coordinate or
 * y-coordinate. If hORv = 0, x-coordinate is used, and if it is 1,
 * y-coordinate is used.
 * @return A pair of 2 vectors. First vector contains the inst-ids which are
 * on the lower values of the sorting coordinate. The second vector contains 
 * the inst-ids which are on the higher values of the sorting coordinate.
 */
void 
QPlacer::assign(IntV& instKeys, int hORv, VIP& returnPairs)
{ 
  cout << "Assignment ..." << endl;

  int numkeys = instKeys.size();
  IntV sortedInsts(instKeys);
  std::sort(sortedInsts.begin(), sortedInsts.end(), Sorter(hORv));

  int halfOfTotalInsts = numkeys/2;
  if (halfOfTotalInsts > 0) {
    IntV tempvector(halfOfTotalInsts, 0);
    for (int i = 0; i < halfOfTotalInsts; ++i)
    {
      tempvector[i] = sortedInsts[i];
    }
    returnPairs.first = tempvector;

    tempvector.resize(numkeys-halfOfTotalInsts, 0);
    for (int i = halfOfTotalInsts; i < numkeys; ++i) 
    {
      tempvector[i-halfOfTotalInsts] = sortedInsts[i];
    }
    returnPairs.second = tempvector;
  }
}

/*
 * Function to updates the coordinates of the given x, y coordinates
 * according to the given bound.
 * @param padlocation Pointer to a vector containing x, y coordinates.
 * @param bound The minimum and maximum values of the x, y coordinates
 * desired. It is an array of 4 numbers, [x_min, x_max, y_min, y_max].
 */
void 
QPlacer::updateCoordinates(FloatP* padlocation, PlBox& box)
{
  if (padlocation->first < (float)getBoxLLX(box)) {
    padlocation->first = (float)getBoxLLX(box); // xmin
  }
  if (padlocation->first > (float)getBoxURX(box)) {
    padlocation->first = (float)getBoxURX(box); // xmax
  }
  if (padlocation->second < (float)getBoxLLY(box)) {
    padlocation->second = (float)getBoxLLY(box); // ymin
  }
  if (padlocation->second > (float)getBoxURY(box)) {
    padlocation->second = (float)getBoxURY(box); // ymax
  }
  return;
}

/**
 * Function to contains the given inst-ids within the given bound, creates
 * Virtual pads, and runs the resulting QPlaceDB oect.
 * @param reference locations of the insts whose inst-ids are in "instKeys".
 * @param instKeys Vector of inst-ids of insts that need to be contained within
 * the given bound.
 * @param bound The minimum and maximum values of the x, y coordinates
 * desired. It is an array of 4 numbers, [x_min, x_max, y_min, y_max].
 * @param hORv It is used to correct the coordinate of some virtual pads. hORv =
 * 0 means that the present bound is the result of a horizontal cut. hORv = 1
 * means that the present bound is the result of a vertical cut. 
 * @param lORr It is used to correct the coordinate of some virtual pads. lORr =
 * 0 means that the present inst-ids are present in the lower part of a cut. 
 * lORr = 1 means that the present inst-ids are present in the higher part of a cut.
 */
void 
QPlacer::containNrun(VFP& locs, IntV& instKeys, PlBox& box, int hORv, int lORr)
{
  cout << "Containment ..." << endl;

  // add insts to the new core, which are on appropriate side
  QPlaceDB newcore;
  for (int i = 0; i < instKeys.size(); ++i)
  {
    IntV& pinstconns = mc_->getInstconnections(instKeys[i]);
    newcore.addInst(instKeys[i]);
  }
  newcore.setBox(box);

  cout << "Containment box:" << getBoxLLX(box) << " " << getBoxLLY(box) << " " 
       << getBoxURX(box) << " " << getBoxURY(box) << endl;

  IntS donenets;
  // add pads and nets to the new core
  int ppadnum = 0;
  int newconn = 0;	// pad number
  FloatV padtemp(3, 0);
  // i recurses through the insts in instKeys
  for (int i = 0; i < instKeys.size(); ++i)
  {
    int pinstnum = instKeys[i];
    IntV& pinstconns = mc_->getInstconnections(pinstnum);

    // k recurses through the nets in the presentinst
    for (int k = 0; k < pinstconns.size(); ++k)
    {
      int pnet = pinstconns[k];
      if (donenets.find(pnet) != donenets.end()) {
        // processed the present net already
        continue;
      } else {
        donenets.insert(pnet);
      }

      // for each net get all the pad connections
      IntV& netconn = mc_->getNetPadConns(pnet);

      // process connected pads as new pads
      for (int l = 0; l < netconn.size(); ++l)
      {
        newconn = netconn[l];	// pad number
        ppadnum++;
        newcore.addNet(pnet, ppadnum, true/*pad*/);

        padtemp[0] = static_cast<float>(pnet);
        FloatP padlocation = mc_->getPadCoords(newconn);

        // correct coordinates which are outside the bounding box
        updateCoordinates(&padlocation, box);
        padtemp[1] = padlocation.first;
        padtemp[2] = padlocation.second;
        // add new pad
        newcore.addPad(ppadnum, padtemp);
      }

      // for each net get all the inst connections
      netconn = mc_->getNetInstConns(pnet);

      // process connected insts as new pads
      for (int l = 0; l < netconn.size(); ++l)
      {
        newconn = netconn[l]; // inst number

        // skipping if newconn is a inst inside the bounding box
        if (find(instKeys.begin(), instKeys.end(), newconn) != instKeys.end()) {
          newcore.addNet(pnet, newconn, false/*inst*/);
          continue;
        }
        ppadnum++;
        newcore.addNet(pnet, ppadnum, true);

        padtemp[0] = static_cast<float>(pnet);
        FloatP padlocation = mc_->getInstCoords(newconn);

        // correct coordinates of insts if they are outside the bounding box
        updateCoordinates(&padlocation, box);
        // correct coordinates of insts if they are inside the bounding box
        if (hORv == 0) {
          if (lORr == 0) {
            // left of a horizontal cut
            padlocation.first = getBoxURX(box);
          } else {
            // right of a horizontal cut
            padlocation.first = getBoxLLX(box);
          }
        } else {
          if (lORr == 0) {
            // top of a vertical cut
            padlocation.second = getBoxURY(box);
          } else {
            // bottom of a vertical cut
            padlocation.second = getBoxLLY(box);
          }
        }
        padtemp[1] = padlocation.first;
        padtemp[2] = padlocation.second;
        // add new pad
        newcore.addPad(ppadnum, padtemp);
      }
    }
  }

  cout << "Done. Added " << newcore.getNumI() << " insts, " ;
  cout << newcore.getNumP() << " pads, " << newcore.getNumN() << " nets." << endl;

  // solve "newcore" for locations of insts inside bound
  solveForLoc(&newcore);
  newcore.getLocations(locs, instKeys);
}

/*
 * Function to recursively calls itself to place the given insts within the
 * bound. The bound keeps shortening as the depth of the recursion increases.
 * Also the number of insts in each bound decreases as the depth of the 
 * recursion increases. It aims to find a uniform distribution of the insts in
 * all the divisions of the ASIC.
 * @param instKeys Vector of inst-ids of insts that need to be placed within
 * the given bound.
 * @param bound The minimum and maximum values of the x, y coordinates
 * desired. It is a box, [x_min, y_min, x_max, y_max].
 * @param n The level of the iteration. A nth iteration means that there are
 * 2^n divisions in the ASIC.
 */
void 
QPlacer::place(IntV& instKeys, PlBox& box, int n)
{
  if (n >= getPartitionNum()) return;

  cout << endl << "n = " << n << endl;
  int nnext = n + 2;

  // sort overall horizontally
  cout << endl << "Horizontal Sort ..." << endl;
  VIP leftrightInsts;
  assign(instKeys, 0, leftrightInsts);
  int midX = getBoxLLX(box)+ getBoxWidth(box)/2;

  // call containNrun
  PlBox left_box(getBoxLLX(box), getBoxLLY(box),
                 getBoxURX(box)-midX, getBoxURY(box));
  cout << endl << "Containing Left Insts:" << endl;
  VFP leftlocs;
  containNrun(leftlocs, leftrightInsts.first, left_box, 0, 0);

  PlBox right_box(getBoxLLX(box)+midX, getBoxLLY(box),
                  getBoxURX(box), getBoxURY(box));
  cout << endl << "Containing Right Insts:" << endl;
  VFP rightlocs;
  containNrun(rightlocs, leftrightInsts.second, right_box, 0, 1);

  mc_->addLocation(leftlocs.first, leftlocs.second, leftrightInsts.first);
  mc_->addLocation(rightlocs.first, rightlocs.second, leftrightInsts.second);
  //mc_->printAllInsts();

  // sort left half vertically
  cout << endl << "Left Half Vertical Sort ..." << endl;
  VIP left_topbottomInsts;
  assign(leftrightInsts.first, 1, left_topbottomInsts);
  int midY = getBoxLLY(box) + getBoxHeight(box)/2;

  // call containNrun
  PlBox leftbottom_box(getBoxLLX(box), getBoxLLY(box),
                       getBoxURX(box)-midX, getBoxURY(box)-midY);
  PlBox lefttop_box(getBoxLLX(box), getBoxLLY(box)+midY,
                    getBoxURX(box)-midX, getBoxURY(box));

  cout << endl << "Containing Left Bottom Insts:" << endl;
  VFP leftbottomlocs;
  containNrun(leftbottomlocs, left_topbottomInsts.first, leftbottom_box, 1, 0);

  cout << endl << "Containing Left Top Insts:" << endl;
  VFP lefttoplocs;
  containNrun(lefttoplocs, left_topbottomInsts.second, lefttop_box, 1, 1);

  // sort right half vertically
  cout << endl << "Right Half Vertical Sort ..." << endl;
  VIP right_topbottomInsts;
  assign(leftrightInsts.second, 1, right_topbottomInsts);

  // call containNrun
  PlBox rightbottom_box(getBoxLLX(box)+midX, getBoxLLY(box),
                        getBoxURX(box), getBoxURY(box)-midY);
  PlBox righttop_box(getBoxLLX(box)+midX, getBoxLLY(box)+midY,
                        getBoxURX(box), getBoxURY(box));

  cout << endl << "Containing Right Bottom Insts:" << endl;
  VFP rightbottomlocs;
  containNrun(rightbottomlocs, right_topbottomInsts.first, rightbottom_box, 1, 0);

  cout << endl << "Containing Right Top Insts:" << endl;
  VFP righttoplocs;
  containNrun(righttoplocs, right_topbottomInsts.second, righttop_box, 1, 1);

  mc_->addLocation(leftbottomlocs.first, leftbottomlocs.second, left_topbottomInsts.first);
  mc_->addLocation(lefttoplocs.first, lefttoplocs.second, left_topbottomInsts.second);
  mc_->addLocation(rightbottomlocs.first, rightbottomlocs.second, right_topbottomInsts.first);
  mc_->addLocation(righttoplocs.first, righttoplocs.second, right_topbottomInsts.second);

  //recursively calls itself for 4 partition areas
  place(left_topbottomInsts.first, leftbottom_box, nnext);
  place(left_topbottomInsts.second, lefttop_box, nnext);
  place(right_topbottomInsts.first, rightbottom_box, nnext);
  place(right_topbottomInsts.second, righttop_box, nnext);

  return;
}
/*
 * Function to place insts in core area
 */
void 
QPlacer::placeCore()
{
  createCoreDB();
  solveForLoc(mc_);
  //mc_->printAllInsts(20);
}
/*
 * Function to place insts in partition areas
 */
void 
QPlacer::placePartitions()
{
  if (getPartitionNum() > 1) {
    IntV& keyI = mc_->getInstKeys();
    place(keyI, mc_->getBox(), 1);
  }
}
/*
 * The run function: It creates a new QPlaceDB object from a file and runs
 * place for recursive placing. Final placement is written back to DB by
 * updateToDB
 */
void QPlacer::run() 
{
  /* Function: main to run quadratic place */
  hr_clock_rep start_time = get_globaltime();
  if (createCoreBox()) {
    // call QP place for core area
    placeCore();
    // call QP place for partition areas
    placePartitions();
    // update new locations to DB
    updateToDB();
  }
  hr_clock_rep stop_time = get_globaltime();
  dreamplacePrint(kINFO, "Quadratic placer time: %g ms\n", get_timer_period()*(stop_time - start_time));
}

DREAMPLACE_END_NAMESPACE
