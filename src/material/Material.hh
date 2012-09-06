//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   Material.hh
 * \author Jeremy Roberts
 * \brief  Material class definition.
 */
//---------------------------------------------------------------------------//

#ifndef MATERIAL_HH_
#define MATERIAL_HH_

#include "utilities/Definitions.hh"
#include "utilities/DBC.hh"
#include "utilities/SP.hh"
#ifdef DETRAN_ENABLE_BOOST
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#endif

namespace detran_material
{

//---------------------------------------------------------------------------//
/*!
 * \class Material
 * \brief Simple cross section container.
 */
//---------------------------------------------------------------------------//
class Material
{

public:

  //-------------------------------------------------------------------------//
  // TYPEDEFS
  //-------------------------------------------------------------------------//

  typedef detran_utilities::SP<Material> SP_material;
  typedef detran_utilities::vec_dbl      vec_dbl;
  typedef detran_utilities::vec2_dbl     vec2_dbl;
  typedef detran_utilities::vec3_dbl     vec3_dbl;
  typedef detran_utilities::vec_int      vec_int;
  typedef detran_utilities::vec2_int     vec2_int;
  typedef detran_utilities::size_t       size_t;

  //-------------------------------------------------------------------------//
  // PUBLIC INTERFACE
  //-------------------------------------------------------------------------//

  /*!
   *  \brief Constructor.
   *  \param    number_materials    Number of materials.
   *  \param    number_groups       Number of energy groups.
   *  \param    downscatter         Switch on to use only downscatter.
   */
  Material(const size_t number_materials,
           const size_t number_groups,
           bool downscatter = false);

  /*!
   *  \brief SP Constructor.
   *
   *  \param    number_groups       Number of energy groups.
   *  \param    number_materials    Number of materials.
   *  \param    downscatter         Switch on to use only downscatter.
   *  \return                       Smart posize_ter to Material object.
   */
  static SP_material Create(const size_t number_materials,
                            const size_t number_groups,
                            bool downscatter)
  {
    SP_material
      p(new Material(number_groups, number_materials, downscatter));
    return p;
  }

  //--------------------------------------------------------------------------//
  // Setters
  //--------------------------------------------------------------------------//

  void set_sigma_t(size_t m, size_t g, double v);
  void set_sigma_a(size_t m, size_t g, double v);
  void set_nu_sigma_f(size_t m, size_t g, double v);
  void set_sigma_f(size_t m, size_t g, double v);
  void set_nu(size_t m, size_t g, double v);
  void set_chi(size_t m, size_t g, double v);
  void set_sigma_s(size_t m, size_t g, size_t gp, double v);
  void set_diff_coef(size_t m, size_t g, double v);

  // Vectorized setters

  void set_sigma_t(size_t m, vec_dbl &v);
  void set_sigma_a(size_t m, vec_dbl &v);
  void set_nu_sigma_f(size_t m, vec_dbl &v);
  void set_sigma_f(size_t m, vec_dbl &v);
  void set_nu(size_t m, vec_dbl &v);
  void set_chi(size_t m, vec_dbl &v);
  void set_sigma_s(size_t m, size_t g, vec_dbl &v);
  void set_diff_coef(size_t m, vec_dbl &v);

  //------------------------------------------------------------------------//
  // Getters
  //------------------------------------------------------------------------//

  inline double sigma_t(size_t m, size_t g) const;
  inline double sigma_a(size_t m, size_t g) const;
  inline double nu_sigma_f(size_t m, size_t g) const;
  inline double sigma_f(size_t m, size_t g) const;
  inline double nu(size_t m, size_t g) const;
  inline double chi(size_t m, size_t g) const;
  inline double sigma_s(size_t m, size_t g, size_t gp) const;
  inline double diff_coef(size_t m, size_t g) const;

  // Vectorized getters

  inline vec_dbl sigma_t(size_t m) const;
  inline vec_dbl sigma_a(size_t m) const;
  inline vec_dbl nu_sigma_f(size_t m) const;
  inline vec_dbl sigma_f(size_t m) const;
  inline vec_dbl nu(size_t m) const;
  inline vec_dbl chi(size_t m) const;
  inline vec2_dbl sigma_s(size_t m) const;
  inline vec_dbl diff_coef(size_t m) const;

  //------------------------------------------------------------------------//
  // OTHER ACCESSORS
  //------------------------------------------------------------------------//

  size_t number_groups() const
  {
    return d_number_groups;
  }

  size_t number_materials() const
  {
    return d_number_materials;
  }

  /*!
   *  \brief Lower scatter group bound.
   *
   *  This is the *lowest* index (highest energy) \f$ g' \f$
   *  that leads to downscatter for a given outgoing group \f$ g \f$.
   */
  size_t lower(size_t g) const;

  /*!
   *  \brief Upper scatter group bound.
   *
   *  This is the *highest* index (lowest energy) \f$ g' \f$
   *  that upscatters size_to the outgoing group \f$ g \f$.
   */
  size_t upper(size_t g) const;

  /// Do we do only downscatter?
  bool downscatter()
  {
    return d_downscatter;
  }

  /// Index below which upscatter doesn't occur for any material.
  size_t upscatter_cutoff()
  {
    return d_upscatter_cutoff;
  }

  /*!
   *  \brief Compute the absorption cross section from total and scattering.
   *
   *  \note this overwrites any data for \f$ \Sigma_a \f$ already stored.
   */
  void compute_sigma_a();

  /*!
   *  \brief Compute the diffusion coefficient from \f$ \Sigma_t \f$.
   *
   *  Assuming isotropic scattering in the LAB, the diffusion
   *  coefficient is simply \f$ D = 1/3\Sigma_t \f$.
   *
   *  \todo Update diffusion definition if anisotropic scattering
   *        is added.
   *
   *  \note This overwrites any data for \f$ D \f$ already stored.
   */
  void compute_diff_coef();

  /// Computes scattering bounds and absorption cross section.
  void finalize();

  /// Pretty print the material database.
  void display();

private:

  /// Number of groups
  size_t d_number_groups;

  /// Number of materials
  size_t d_number_materials;

  /// Downscatter switch (when true, upscatter ignored)
  bool d_downscatter;

  /// Total cross section [material, group]
  vec2_dbl d_sigma_t;

  /// Absorption cross section [material, group]
  vec2_dbl d_sigma_a;

  /// nu * Fission [material, group]
  vec2_dbl d_nu_sigma_f;

  /// Fission [material, group]
  vec2_dbl d_sigma_f;

  /// nu [material, group]
  vec2_dbl d_nu;

  /// Fission spectrum [material, group]
  vec2_dbl d_chi;

  /// Scatter [material, group<-, group']
  vec3_dbl d_sigma_s;

  /// Diffusion coefficient [material, group]
  vec2_dbl d_diff_coef;

  /// Scatter bounds applied to all materials [group, 2]
  vec2_int d_scatter_bounds;

  /*!
   *  Upscatter cutoff.  Only groups equal to or above this cutoff are
   *  subject to upscatter iterations.
   */
  size_t d_upscatter_cutoff;

  /// Are we ready to be used?
  bool d_finalized;

#ifdef DETRAN_ENABLE_BOOST

  Material(){}

  friend class boost::serialization::access;
  template<class Archive>

  void serialize(Archive & ar, const unsigned int version)
  {
    ar & d_number_groups;
    ar & d_number_materials;
    ar & d_downscatter;
    ar & d_sigma_t;
    ar & d_nu_sigma_f;
    ar & d_sigma_f;
    ar & d_nu;
    ar & d_chi;
    ar & d_sigma_s;
    ar & d_diff_coef;
    ar & d_scatter_bounds;
    ar & d_upscatter_cutoff;
    ar & d_finalized;
  }

#endif

};

} // end namespace detran_material

//---------------------------------------------------------------------------//
// INLINE FUNCTIONS
//---------------------------------------------------------------------------//

#include "Material.i.hh"

#endif /* MATERIAL_HH_ */
