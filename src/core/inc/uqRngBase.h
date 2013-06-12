//-----------------------------------------------------------------------bl-
//--------------------------------------------------------------------------
// 
// QUESO - a library to support the Quantification of Uncertainty
// for Estimation, Simulation and Optimization
//
// Copyright (C) 2008,2009,2010,2011,2012,2013 The PECOS Development Team
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the Version 2.1 GNU Lesser General
// Public License as published by the Free Software Foundation.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc. 51 Franklin Street, Fifth Floor, 
// Boston, MA  02110-1301  USA
//
//-----------------------------------------------------------------------el-
// 
// $Id$
//
//--------------------------------------------------------------------------

#ifndef __UQ_RNG_BASE_H__
#define __UQ_RNG_BASE_H__

#include <uqDefines.h>
#include <iostream>




/*! \file uqRngBase.h
    \brief Random Number Generation class.
*/

/*! \class uqRngBaseClass
    \brief Class for random number generation (base class). 
    
    This class is  a “virtual” class of generic random number generator, in order 
    to accommodate either GSL or Boost RNG. 
*/


class uqRngBaseClass
{
public:
  
    //! @name Constructor/Destructor methods
  //@{ 
  //! Default Constructor: it should not be used.
  uqRngBaseClass();
  
  //! Constructor with seed.
  uqRngBaseClass(int seed, int worldRank);
	   
  //! Virtual destructor.
  virtual ~uqRngBaseClass();
  //@}
  
  
    //! @name Sampling methods
  //@{ 
  //! Sets the seed.
          int    seed          () const;
	  
  //! Resets the seed with value \c newSeed.
  virtual void   resetSeed     (int newSeed);
  
  //! Samples a value from a uniform distribution.
  virtual double uniformSample ()                          const = 0;
  
  //! Samples a value from a Gaussian distribution with standard deviation given by \c stdDev.
  virtual double gaussianSample(double stdDev)             const = 0;
  
  //! Samples a value from a Beta distribution.
  virtual double betaSample    (double alpha, double beta) const = 0;
  
  //! Samples a value from a Gamma distribution.
  virtual double gammaSample   (double a, double b)        const = 0;

  //@}
protected:
  //! Seed.
          int m_seed;
	  
  //! Rank of processor.
          int m_worldRank;

private:
  
  //! Reset seed.
          void privateResetSeed();
};

#endif // __UQ_RNG_BASE_H__
