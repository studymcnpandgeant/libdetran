//----------------------------------*-C++-*----------------------------------//
/**
 *  @file   Jacobi.i.hh
 *  @brief  Jacobi inline member definitions
 *  @author Jeremy Roberts
 *  @date   Sep 13, 2012
 */
//---------------------------------------------------------------------------//

#ifndef callow_JACOBI_I_HH_
#define callow_JACOBI_I_HH_

#include "callow/matrix/Matrix.hh"
#include <cmath>
#include <cstdio>

namespace callow
{

//---------------------------------------------------------------------------//
// SOLVE
//---------------------------------------------------------------------------//

inline void Jacobi::solve_impl(const Vector &b, Vector &x)
{

  Insist(dynamic_cast<Matrix*>(d_A.bp()),
    "Need an explicit matrix for use with Jacobi iteration");
  typename Matrix::SP_matrix A = d_A;

  // temporary storage and pointers for swapping
  Vector temp(x.size(), 0.0);
  Vector* x0 = &x;
  Vector* x1 = &temp;
  Vector* swap;

  // iteration count
  int &iteration = d_number_iterations;

  // compute initial residual Ax - b and its norm
  A->multiply((*x0), (*x1));
  double r = x1->norm_residual(b, L2);
  if (monitor_init(r))
  {
    //return;
  }

  // perform iterations
  for (int iteration = 1; iteration <= d_maximum_iterations; ++iteration)
  {

    //---------------------------------------------------//
    // compute X1 <-- -inv(D)*(L+U)*X0 + inv(D)*b
    //---------------------------------------------------//

    double* a = A->values();
    for (int i = 0; i < A->number_rows(); i++)
    {
      double v = 0;
      int p = A->start(i);
      int d = A->diagonal(i);
      // L * X0
      for (; p < d; ++p)
        v += a[p] * (*x0)[A->column(p)];
      ++p; // skip diagonal
      // U * X0
      for (; p < A->end(i); ++p)
        v += a[p] * (*x0)[A->column(p)];
      (*x1)[i] = (b[i] - v) / a[d];
    }
    a = 0; // nullify pointer

    //---------------------------------------------------//
    // compute residual norm
    //---------------------------------------------------//

    r = x1->norm_residual(*x0, L2);

    //---------------------------------------------------//
    // swap pointers
    //---------------------------------------------------//
    swap = x0;
    x0   = x1;
    x1   = swap;

    //---------------------------------------------------//
    // check convergence
    //---------------------------------------------------//

    if (monitor(iteration, r)) break;

  }

  // copy into the solution vector if needed
  if (x0 != &x) x.copy(*x0);

}

} // end namespace callow

#endif // callow_JACOBI_I_HH_

//---------------------------------------------------------------------------//
//              end of file Jacobi.i.hh
//---------------------------------------------------------------------------//
