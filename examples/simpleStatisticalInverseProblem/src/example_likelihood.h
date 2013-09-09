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
// $Id: example_likelihood.h 37704 2013-03-08 21:10:36Z karl $
//
//--------------------------------------------------------------------------
#ifndef __EX_LIKELIHOOD_H__
#define __EX_LIKELIHOOD_H__

#include <uqGslMatrix.h>

struct
likelihoodRoutine_DataType
{
  const QUESO::uqGslVectorClass* meanVector;
  const QUESO::uqGslMatrixClass* covMatrix;
};

double likelihoodRoutine(
  const QUESO::uqGslVectorClass& paramValues,
  const QUESO::uqGslVectorClass* paramDirection,
  const void*             functionDataPtr,
  QUESO::uqGslVectorClass*       gradVector,
  QUESO::uqGslMatrixClass*       hessianMatrix,
  QUESO::uqGslVectorClass*       hessianEffect);

#endif
