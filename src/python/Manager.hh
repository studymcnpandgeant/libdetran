//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   Manager.hh
 * \brief  Manager for external libraries.
 * \author Jeremy Roberts
 * \date   Jul 26, 2012
 */
//---------------------------------------------------------------------------//

#ifndef Manager_HH_
#define Manager_HH_

// Configuration
#include "detran_config.hh"
// Callow
#include "callow/utils/Initialization.hh"

namespace detran
{

/*!
 *  \class Manager
 *  \brief Manager for initializing and finalizing external libraries.
 *
 *  This is useful if the user wants finer control on using
 *  detran classes than PyExecute affords but still requires
 *  libraries to be initialized and finalized.
 */
class Manager
{

public:

  /// Initialize libraries (PETSc/SLEPc through callow)
  static void initialize(int argc, char *argv[])
  {
    callow_initialize(argc, argv);
  }

  /// Finalize libraries
  static void finalize()
  {
    callow_finalize();
  }

};

} // end namespace detran

#endif // MANAGER_HH_

//---------------------------------------------------------------------------//
//              end of file Manager.hh
//---------------------------------------------------------------------------//
