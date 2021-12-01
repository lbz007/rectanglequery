/**
 * File:    solve.h
 *
 * Main class and functions for solve.
 *
 * Author:  Fly
 */
#ifndef EDI_QUADRATIC_SOLVER_H_
#define EDI_QUADRATIC_SOLVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <valarray>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "utility/src/Msg.h"

DREAMPLACE_BEGIN_NAMESPACE
using namespace std;

class CooMatrix
{
  // COOrdinate sparse matrix
  public:
    CooMatrix() = default;
    CooMatrix(int n, int nnz) 
    {
      n_ = n;
      nnz_ = nnz;
      row_.resize(nnz);
      col_.resize(nnz);
      dat_.resize(nnz);
    }
    ~CooMatrix() = default;

    void readCooMatrix(const char *fname);
    void matVec(const valarray<float> &x, valarray<float> &y);
    void solve(const valarray<float> &b, valarray<float> &x);
    void solve(const valarray<float> &b, valarray<float> &x, const int min, const int max);

    int n_;
    int nnz_;
    valarray<int> row_;
    valarray<int> col_;
    valarray<float> dat_;
};

template<typename T>
void printValarray(valarray<T>& v)
{
  for (size_t i = 0; i < v.size(); ++i)
  {
    cout << v[i] << endl;
  }
}

DREAMPLACE_END_NAMESPACE
#endif  // EDI_QUADRATIC_SOLVER_H_
