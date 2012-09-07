//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   FissionSource.hh
 * \author robertsj
 * \date   Apr 4, 2012
 * \brief  FissionSource class definition.
 */
//---------------------------------------------------------------------------//

#ifndef FISSIONSOURCE_HH_
#define FISSIONSOURCE_HH_

#include "State.hh"
#include "geometry/Mesh.hh"
#include "material/Material.hh"
#include "utilities/DBC.hh"
#include "utilities/Definitions.hh"
#include "utilities/SP.hh"

namespace detran
{

//---------------------------------------------------------------------------//
/*!
 * \class FissionSource
 * \brief Defines the isotropic source from fission reactions.
 */
//---------------------------------------------------------------------------//

class FissionSource
{

public:

  //-------------------------------------------------------------------------//
  // TYPEDEFS
  //-------------------------------------------------------------------------//

  typedef detran_utilities::SP<FissionSource>       SP_fissionsource;
  typedef State::SP_state                           SP_state;
  typedef detran_geometry::Mesh::SP_mesh            SP_mesh;
  typedef detran_material::Material::SP_material    SP_material;
  typedef detran_utilities::vec_int                 vec_int;
  typedef detran_utilities::size_t                  size_t;

  //-------------------------------------------------------------------------//
  // CONSTRUCTOR & DESTRUCTOR
  //-------------------------------------------------------------------------//

  /*!
   *  \brief Constructor
   *
   *  \param state      State vector
   *  \param mesh       Cartesian mesh
   *  \param material   Materials
   *
   */
  FissionSource(SP_state state, SP_mesh mesh, SP_material material);

  /// SP Constructor
  static SP_fissionsource
  Create(SP_state state, SP_mesh mesh, SP_material material)
  {
    SP_fissionsource p;
    p = new FissionSource(state, mesh, material);
    return p;
  }

  //-------------------------------------------------------------------------//
  // PUBLIC INTERFACE
  //-------------------------------------------------------------------------//

  /// Initialize to thermal fission cross-section, normalized.
  void initialize();

  /// Update the fission density.
  void update();

  /*!
   *   \brief Setup the fission source for an outer iteration.
   *
   *   This sets a new scaling factor \f$ k \f$ and precomputes the
   *   quantity \f$ v = fd(k)^{-1} \f$.
   *
   *   \param scale     Scaling factor (typically 1/keff)
   */
  void setup_outer(const double scale);

  /*!
   *   \brief Return the fission source in a group.
   *
   *   The group fission source is just that component of the density
   *   released in  a particular group.  Mathematically, this is just
   *
   *   \f[
   *     q_{f,g} = \frac{\chi_g}{4\pi k} \sum_g \nu\Sigma_{f,g} \phi_g \, .
   *   \f]
   *
   *   Note, the scaling factor is actually arbitrary.  For 2-D and 3-D, it
   *   is \f$ 4\pi \f$, possibly with the eigenvalue \f$ k \f$.  The client
   *   sets this in \ref update.
   *
   *   Note also that this returns a moments source, so the client must
   *   apply the moments-to-discrete operator.
   *
   *   \param   g   Group of the source.
   *   \return      Source vector.
   */
  const State::moments_type& source(const size_t g);

  /*!
   *   \brief Return the fission density.
   *
   *   \f[
   *     fd = \sum_g \nu\Sigma_{f,g} \phi_g \, .
   *   \f]
   *
   *   \return      Fission density vector.
   */
  const State::moments_type& density();

  /*!
   *   \brief Set the fission density.
   *   \param   f   User-defined density.
   */
  void set_density(std::vector<double> &f)
  {
    d_density = f;
  }

private:

  //-------------------------------------------------------------------------//
  // DATA
  //-------------------------------------------------------------------------//

  /// State vector
  SP_state d_state;

  /// Mesh
  SP_mesh d_mesh;

  /// Materials
  SP_material d_material;

  /// \f$ q_{fg} = norm \times \chi_g \sum_g \nu \Sigma_{fg} \phi_g \f$ .
  State::moments_type d_source;

  /// \f$ d = \sum_g \nu \Sigma_{fg} \phi_g \f$ .
  State::moments_type d_density;

  /// Scaling factor
  double d_scale;

  /// Number of groups.
  size_t d_number_groups;

};

} // end namespace detran

#endif /* FISSIONSOURCE_HH_ */

//---------------------------------------------------------------------------//
//              end of FissionSource.hh
//---------------------------------------------------------------------------//
