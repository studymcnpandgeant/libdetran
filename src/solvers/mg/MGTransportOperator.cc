//----------------------------------*-C++-*-----------------------------------//
/**
 *  @file  MGTransportOperator.cc
 *  @brief MGTransportOperator member definitions
 *  @note  Copyright(C) 2012-2013 Jeremy Roberts
 */
//----------------------------------------------------------------------------//

#include "MGTransportOperator.hh"
#include "utilities/MathUtilities.hh"

namespace detran
{

//----------------------------------------------------------------------------//
template <class D>
MGTransportOperator<D>::MGTransportOperator(SP_state        state,
                                            SP_boundary     boundary,
                                            SP_sweeper      sweeper,
                                            SP_sweepsource  source,
                                            size_t          cutoff,
                                            bool            adjoint)
  : Base(this)
  , d_state(state)
  , d_boundary(boundary)
  , d_sweeper(sweeper)
  , d_sweepsource(source)
  , d_number_groups(0)
  , d_number_active_groups(0)
  , d_krylov_group_cutoff(cutoff)
  , d_moments_size(0)
  , d_boundary_size(0)
  , d_adjoint(adjoint)
{
  Require(d_state);
  Require(d_boundary);
  Require(d_sweeper);
  Require(d_sweepsource);
  Require(d_krylov_group_cutoff <= d_state->number_groups());

  using detran_utilities::range;

  // Determine the sizes of the moments.
  d_moments_size = d_state->moments_size();

  // Determine the sizes of any reflected boundary fluxes.
  for (int side = 0; side < 2*D::dimension; side++)
  {
    if (boundary->is_reflective(side))
    {
      // We only need to store only half of the unknowns.
      d_boundary_size += boundary->boundary_flux_size(side)/2;
    }
  }

  // Total number of groups (which may not all be subject to Krylov solve)
  d_number_groups = d_state->number_groups();

  // Define the group boundaries for the Krylov block
  int lower = 0;
  int upper = d_number_groups;
  if (d_adjoint)
  {
    lower = d_number_groups - 1;
    upper = -1;
  }
  d_groups = range<size_t>(d_krylov_group_cutoff, upper);

  // Number of active groups
  d_number_active_groups =  d_groups.size();

  // Set the operator size
  set_size(d_number_active_groups * (d_moments_size + d_boundary_size));
}

//----------------------------------------------------------------------------//
template <class D>
void MGTransportOperator<D>::display() const
{
  using std::cout;
  using std::endl;
  cout << "MULTI-GROUP TRANSPORT OPERATOR" << endl;
  cout << "              total size: " << d_m << endl;
  cout << "  total number of groups: " << d_number_groups << endl;
  cout << " number of active groups: " << d_number_active_groups << endl;
  cout << "        upscatter cutoff: " << d_krylov_group_cutoff << endl;
  cout << "      group moments size: " << d_moments_size << endl;
  cout << "     group boundary size: " << d_boundary_size << endl;
}

//----------------------------------------------------------------------------//
template <class D>
void MGTransportOperator<D>::multiply(const Vector &x,  Vector &y)
{
  using std::cout;
  using std::endl;

  // Fill a temporary flux vector with the Krylov vector.  The Krylov
  // vector is  ordered as follows:
  //   [phi(lower) phi(lower+1) ... phi(G-1) psi(lower) ...]
  // where psi is the boundary angular flux, present only if there
  // are reflective conditions
  State::vec_moments_type phi(d_state->all_phi());
  for (groups_iter g_it = d_groups.begin(); g_it != d_groups.end(); ++g_it)
  {
    // d_groups gives the actual groups.  For forward problems, this needs
    // to be translated to start at zero for indexing into the flux vector.
    int g = d_adjoint ? (*g_it) : (*g_it) - d_krylov_group_cutoff;
    int offset = g * d_moments_size;
    for (int i = 0; i < d_moments_size; ++i)
      phi[*g_it][i] = x[i + offset];
  }

  // sweep each applicable group
  for (groups_iter g = d_groups.begin(); g != d_groups.end(); ++g)
  {
    // group index in applicable set
    int g_index  = d_adjoint ? *g : *g - d_krylov_group_cutoff;
    // moment offset, the starting moment index within the Krylov vector
    int m_offset = g_index * d_moments_size;
    // boundary offset, the starting boundary index within the Krylov vector
    int b_offset = d_number_active_groups * d_moments_size +
                   g_index * d_boundary_size;

    // reset the source and place the original outgoing boundary flux.
    d_boundary->clear(*g);

    if (d_boundary->has_reflective())
    {
      // set the incident boundary flux.
      d_boundary->psi(*g, const_cast<double*>(&x[0]) + b_offset,
                      BoundaryBase<D>::IN, BoundaryBase<D>::SET, true);
    }

    // reset the source to zero.
    d_sweepsource->reset();
    d_sweepsource->build_total_scatter(*g, d_krylov_group_cutoff, phi);

    // copy group flux for sweep
    typename State::moments_type phi_g(phi[*g]);

    // set the sweeper and sweep.
    d_sweepsource->set_discrete_external_source_flag(false);
    d_sweeper->setup_group(*g);
    d_sweeper->sweep(phi_g);
    d_sweepsource->set_discrete_external_source_flag(true);

    // assign the moment values.
    for (int i = 0; i < d_moments_size; i++)
      y[i + m_offset] = x[i + m_offset] - phi_g[i];

    // assign boundary fluxes, if applicable
    if (d_boundary->has_reflective())
    {
      // update the boundary (redirect outgoing as incident)
      d_boundary->update(*g);

      // extract the incident boundary
      State::angular_flux_type psi_update(d_boundary_size, 0.0);
      d_boundary->psi(*g, &psi_update[0],
                      BoundaryBase<D>::IN, BoundaryBase<D>::GET, true);

      // add the boundary values.
      for (int a = 0; a < d_boundary_size; a++)
        y[a + b_offset] = x[a + b_offset] - psi_update[a];
    }

  } // end groups

}

//----------------------------------------------------------------------------//
// EXPLICIT INSTANTIATIONS
//----------------------------------------------------------------------------//

template class MGTransportOperator<_1D>;
template class MGTransportOperator<_2D>;
template class MGTransportOperator<_3D>;

} // end namespace detran

//----------------------------------------------------------------------------//
//              end of file MGTransportOperator.cc
//----------------------------------------------------------------------------//
