//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   CurrentTally.hh
 * \brief  CurrentTally 
 * \author Jeremy Roberts
 * \date   Aug 7, 2012
 */
//---------------------------------------------------------------------------//

#ifndef CURRENTTALLY_HH_
#define CURRENTTALLY_HH_

#include "CoarseMesh.hh"
#include "discretization/Equation.hh"
#include "angle/Quadrature.hh"
#include "discretization/DimensionTraits.hh"
#include "utilities/Definitions.hh"
#include "utilities/SP.hh"
#include <vector>

namespace detran
{

/*!
 *  \class CurrentTally
 *  \brief Records partial currents through coarse mesh surfaces.
 *
 *  To illustrate, consider the leakage term of the transport
 *  equation in one coarse cell:
 *
 *  \f[
 *      \Big ( \mu  \frac{\partial}{\partial x}
 *           + \eta \frac{\partial}{\partial y}
 *           + \xi  \frac{\partial}{\partial z}
 *      \Big ) \psi(x, \mu, \eta, \xi) \, .
 *  \f]
 *
 *  Assuming a cell of volume \f$ V = \Delta_x \Delta_y \Delta_z \f$, we
 *  integrate the first term over the cell to get
 *
 *  \f[
 *  \begin{split}
 *      & \mu \int^{\Delta_y}_{0} \int^{\Delta_z}_{0} dy dz
 *         \Bigg ( \frac{\partial \psi}{\partial x} \Big |_{x=\Delta_x}
 *                -\frac{\partial \psi}{\partial x} \Big |_{x=0} \Bigg ) =
 *      \int^{\Delta_y}_{0} \int^{\Delta_z}_{0} dy dz
 *        \Big ( J_x(\Delta_x, y, z) - J_x(0, y, z) \Big ) \, ,
 *  \end{split}
 *  \f]
 *
 *  where \f$ J_x \f$ is the \f$x\f$-directed partial current.  Similar
 *  terms can be found for the other directions.
 *
 *  In 2D and 3D cases, the current must be integrated in space, which
 *  is done using a simple mid-point rule consistent with the underlying
 *  spatial discretizations.
 *
 *  \todo It would be useful to abstract out a general tally for
 *        arbitrary nonlinear schemes.  While we do partial currents,
 *        applicable to CMR, CMFD, and pCMFD, one can image various
 *        schemes that restrict the angular dependence to a coarser
 *        angular mesh for use in low order transport.
 */
/*!
 * \example /transport/test/test_CurrentTally.cc
 *
 * Test of CurrentTally.
 */

template <class D>
class CurrentTally
{

public:

  //-------------------------------------------------------------------------//
  // ENUMERATIONS
  //-------------------------------------------------------------------------//

  enum DIRECTED
  {
    X_DIRECTED,
    Y_DIRECTED,
    Z_DIRECTED
  };

  enum SENSE
  {
    NEGATIVE,
    POSITIVE
  };

  //-------------------------------------------------------------------------//
  // TYPEDEFS
  //-------------------------------------------------------------------------//

  typedef detran_utilities::SP<CurrentTally>            SP_currenttally;
  typedef CoarseMesh::SP_coarsemesh                     SP_coarsemesh;
  typedef detran_angle::Quadrature::SP_quadrature       SP_quadrature;
  typedef CoarseMesh::SP_mesh                           SP_mesh;
  typedef typename EquationTraits<D>::face_flux_type    face_flux_type;
  typedef detran_utilities::size_t                      size_t;
  typedef detran_utilities::vec_int                     vec_int;
  typedef detran_utilities::vec2_int                    vec2_int;
  typedef detran_utilities::vec_dbl                     vec_dbl;
  typedef detran_utilities::vec2_dbl                    vec2_dbl;
  typedef detran_utilities::vec3_dbl                    vec3_dbl;

  //-------------------------------------------------------------------------//
  // CONSTRUCTOR & DESTRUCTOR
  //-------------------------------------------------------------------------//

  /*!
   *  \brief Constructor
   *  \param mesh
   *  \param quadrature
   *  \param number_groups
   */
  CurrentTally(SP_coarsemesh mesh,
               SP_quadrature quadrature,
               const size_t number_groups);

  //-------------------------------------------------------------------------//
  // PUBLIC INTERFACE
  //-------------------------------------------------------------------------//

  /*!
   *  \brief Add angular flux to the current tally
   *
   *  \param  i           x mesh index
   *  \param  j           y mesh index
   *  \param  k           z mesh index
   *  \param  g           group index
   *  \param  o           octant
   *  \param  a           angle within octant
   *  \param  psi         edge angular flux
   */
  inline void tally(const size_t i,
                    const size_t j,
                    const size_t k,
                    const size_t g,
                    const size_t o,
                    const size_t a,
                    const face_flux_type psi);

  /*!
   *  \brief Add angular flux to the tally for a single incident direction
   *
   *  This is used to sweep over incident boundary containers so that
   *  the incident conditions can be avoided for the sweep.
   *
   *  \param  i           x mesh index
   *  \param  j           y mesh index
   *  \param  k           z mesh index
   *  \param  g           group index
   *  \param  o           octant
   *  \param  a           angle within octant
   *  \param  d           axis index for the incident flux
   *  \param  psi         edge angular flux
   */
  inline void tally(const size_t i,
                    const size_t j,
                    const size_t k,
                    const size_t g,
                    const size_t o,
                    const size_t a,
                    const size_t d,
                    const double psi);

  /*!
   *  \brief Get the partial current from a surface and sense
   *  \param  i       x mesh index
   *  \param  j       y mesh index
   *  \param  k       z mesh index
   *  \param  g       group index
   *  \param  axis    0, 1, or 2 for x, y, or z
   *  \param  sense   true for positive (e.g. +x)
   */
  inline double partial_current(const size_t i,
                                const size_t j,
                                const size_t k,
                                const size_t g,
                                const size_t axis,
                                const size_t sense);

  /// Print all the partial currents (for debugging)
  void display()
  {
    for (int d = 0; d < d_partial_current.size(); d++)
      for (int g = 0; g < d_partial_current[d].size(); g++)
        for (int s = 0; s < 2; s++)
          for (int i = 0; i < d_partial_current[d][g][s].size(); i++)
          {
            std::cout << " J(d=" << d << ",g=" << g << ",s=" << s << ",i="
                      << i << ") = " << d_partial_current[d][g][s][i]
                      << std::endl;
          }
  }

  /// Reset a group
  void reset(const size_t group)
  {
    Require(group < d_number_groups);
    for (int d = 0; d < d_partial_current.size(); d++)
      for (int s = 0; s < 2; s++)
        for (int i = 0; i < d_partial_current[d][group][s].size(); i++)
        {
          d_partial_current[d][group][s][i] = 0.0;
        }
  }

private:

  /// Coarse mesh
  SP_coarsemesh d_coarsemesh;

  /// Quadrature
  SP_quadrature d_quadrature;

  /// Number of groups
  const size_t d_number_groups;

  /// Partial currents [dimension][group][sense][index]
  std::vector<vec3_dbl> d_partial_current;

  /// Fine mesh coarse edge flags
  vec2_int d_coarse_edge_flag;

  /// Shifts the spatial index depending on direction
  vec2_int d_octant_shift;

  /// Index of axes perpendicular to partial current
  /// \note Why does initializing require new standard?
  size_t d_perpendicular_index[3][2];

  inline size_t index(const size_t i,
                      const size_t j,
                      const size_t k,
                      const size_t axis)
  {
    // Precondition
    Require(axis < D::dimension);
    Require(i <= d_coarsemesh->get_coarse_mesh()->number_cells_x());
    Require(j <= d_coarsemesh->get_coarse_mesh()->number_cells_y());
    Require(k <= d_coarsemesh->get_coarse_mesh()->number_cells_z());

    size_t nx = d_coarsemesh->get_coarse_mesh()->number_cells_x();
    size_t ny = d_coarsemesh->get_coarse_mesh()->number_cells_y();
    size_t nz = d_coarsemesh->get_coarse_mesh()->number_cells_z();
    if (axis == 0)
      return i + j * (nx + 1) + k * (nx + 1) * ny;
    else if (axis == 1)
      return i + j * nx + k * nx * (ny + 1);
    else
      return i + j * nx + k * nx * ny;
  }

};

} // end namespace detran

//---------------------------------------------------------------------------//
// INLINE MEMBER DEFINITIONS
//---------------------------------------------------------------------------//

#include "CurrentTally.i.hh"

#endif // CURRENTTALLY_HH_ 

//---------------------------------------------------------------------------//
//              end of file CurrentTally.hh
//---------------------------------------------------------------------------//
