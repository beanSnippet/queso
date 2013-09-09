/*--------------------------------------------------------------------------
 *--------------------------------------------------------------------------
 *
 * Copyright (C) 2008 The PECOS Development Team
 *
 * Please see http://pecos.ices.utexas.edu for more information.
 *
 * This file is part of the QUESO Library (Quantification of Uncertainty
 * for Estimation, Simulation and Optimization).
 *
 * QUESO is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QUESO is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QUESO. If not, see <http://www.gnu.org/licenses/>.
 *
 *--------------------------------------------------------------------------
 *
 * $Id$
 *
 * Brief description of this file: 
 * 
 *--------------------------------------------------------------------------
 *-------------------------------------------------------------------------- */

#ifndef __TGA2_QOI_H__
#define __TGA2_QOI_H__

#include <uqGslMatrix.h>
#include <uqDistArray.h>

//********************************************************
// The (user defined) data class that carries the data
// needed by the (user defined) qoi routine
//********************************************************
struct
qoiRoutine_DataClass
{
  double m_beta;
  double m_criticalMass;
  double m_criticalTime;
};

void qoiRoutine(const QUESO::uqGslVectorClass&                    paramValues,
                const QUESO::uqGslVectorClass*                    paramDirection,
                const void*                                functionDataPtr,
                      QUESO::uqGslVectorClass&                    qoiValues,
                      QUESO::uqDistArrayClass<QUESO::uqGslVectorClass*>* gradVectors,
                      QUESO::uqDistArrayClass<QUESO::uqGslMatrixClass*>* hessianMatrices,
                      QUESO::uqDistArrayClass<QUESO::uqGslVectorClass*>* hessianEffects);

#endif // __TGA2_QOI_H__
