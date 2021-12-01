/**
 * File:    solve.cpp
 *
 * Main class and functions for solve.
 *
 * Author:  Fly
 */
#include "solver.h"

DREAMPLACE_BEGIN_NAMESPACE
/*
 * Function to read matrix from a file
 */
void
CooMatrix::readCooMatrix(const char *fname)
{
  // matrix format:
  //  n nnz
  //  1 j1 A[i1,j1]
  //  2 j2 A[i2,j2]
  //    .  .
  //    .  .
  ifstream fin(fname);
  fin >> n_ >> nnz_;
  row_.resize(nnz_);
  col_.resize(nnz_);
  dat_.resize(nnz_);
  for(int i = 0; i < nnz_; ++i)
  {
    fin >> row_[i] >> col_[i] >> dat_[i];
  }
}

/*
 * Function to calculate the sum of squares
 */
float
dot(const valarray<float> &x, const valarray<float> &y)
{
  return (x * y).sum();
}
/*
 * Function to move with vector
 */
void
CooMatrix::matVec(const valarray<float> &x, valarray<float> &y)
{
  // need to reset to 0 first.
  y = 0.0;
  // y = A * x
  for(int i = 0; i < nnz_; ++i)
  {
    y[row_[i]] += dat_[i] * x[col_[i]];
  }
}

/*
 * Old function to calculate x = A^{-1} b with CG
 */
void
CooMatrix::solve(const valarray<float> &b, valarray<float> &x)
{
  // x = A^{-1} b with CG
  // https://en.wikipedia.org/wiki/Conjugate_gradient#Example_code_in_Matlab

  const int maxit = 1000;
  const float stopV = 1e-3;
  valarray<float> Ax(n_);
  valarray<float> Ap(n_);
  valarray<float> r(n_);
  valarray<float> p(n_);
  float alpha, rnorm = 0.0;
  float error, errorold = 1.0;

  for(size_t i=0; i<x.size(); ++i)
  {
    x[i] = static_cast<float>(rand())/RAND_MAX;
  }
  
  matVec(x, Ax);
  r = b - Ax;
  p = r;
  float rnormold = dot(r, r);

  int i;
  for(i=0; i<maxit; ++i)
  {
    matVec(p,Ap);
    alpha = rnormold / dot(p, Ap);

    // p *= alpha;
    x += alpha * p;

    // Ap *= alpha;
    r -= alpha * Ap;

    rnorm = dot(r,r);
    if(sqrt(rnorm) < 1e-3) { 
      break;
    } else { 
      error = abs( dot( r, x) );
      // clog << "||e[" << i << "]||_A = " << error 
      // << "     ratio = " << error/errorold << endl;
      errorold = error;
    }

    p *= (rnorm/rnormold);
    p += r;

    rnormold = rnorm;
  }

  if ( i == maxit ) {
    cerr << "Warning: reaches maximum iteration " << maxit << endl;
  } else {
    cout << "Completed solver with iteration " << i << endl;
  }
}

/*
 * New function to calculate x = A^{-1} b in range(min, max) with CG
 */
void
CooMatrix::solve(const valarray<float> &b, valarray<float> &x,
    const int min, const int max)
{
  // x = A^{-1} b with CG
  const int maxit = 1000;
  const float stopV = 1e-3;
  float alpha, rnorm = 0.0;

  valarray<float> Ap(n_);
  valarray<float> r(n_);
  valarray<float> p(n_);

  for(size_t i = 0; i < x.size(); ++i)
  {
    x[i] = static_cast<float>(rand() % max + min);
  }
  
  matVec(x, Ap);
  r = b - Ap;
  p = r;
  float rnormold = dot(r, r);

  int i = 0;;
  while(i < maxit)
  {
    matVec(p, Ap);
    alpha = rnormold / dot(p, Ap);

    // p *= alpha;
    x += alpha * p;

    // Ap *= alpha;
    r -= alpha * Ap;

    rnorm = dot(r,r);
    if(sqrt(rnorm) < stopV) { 
      break;
    }

    p *= (rnorm/rnormold);
    p += r;

    rnormold = rnorm;
    i++;
  }
  if ( i == maxit ) {
    cerr << "Warning: reaches maximum iteration " << maxit << endl;
  } else {
    cout << "Completed solver with iteration " << i << endl;
  }
}

DREAMPLACE_END_NAMESPACE
