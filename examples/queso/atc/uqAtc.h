/* uq/examples/queso/pyramid/uqAtcValidation.h
 *
 * Copyright (C) 2008 The QUESO Team, http://queso.ices.utexas.edu
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __UQ_ATC_VALIDATION_H__
#define __UQ_ATC_VALIDATION_H__

#include <uqModelValidation.h>
#include <uqVectorSubset.h>
#include <uqAsciiTable.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv.h>

#define R_CONSTANT 8.314472

//********************************************************
// Likelihood function object for both inverse problems of the validation cycle.
// A likelihood function object is provided by user and is called by the UQ library.
// This likelihood function object consists of data and routine.
//********************************************************

// The (user defined) data class that carries the data needed by the (user defined) likelihood routine
template<class P_V, class P_M>
struct
atcLikelihoodRoutine_DataClass
{
  atcLikelihoodRoutine_DataClass(const uqBaseEnvironmentClass& env,
                                 const char* experimentDescriptionFileName,
                                 const char* reactionFileName,
                                 const char* scenarioFileName,
                                 const char* experimentalDataFileName);
 ~atcLikelihoodRoutine_DataClass();

  // Experiment description
  int    NREST;
  int    NSPECI;
  int    NO_STEPS;
  int    NO_PARAM;
  int    NO_EXP;
  double DTIME;
  double UREF;
  double TREF;
  double PREF;
  double RHOREF;
  char Species_name[13][100];
  std::vector<double>      Species_con;
  std::vector<double>      Species_mass;
  std::vector<double>      Species_conm;

  // Reaction data
  int  reac[19][13];
  int  prod[19][13];

  // Scenario parameters
  double ex_SystemTemperature;
  std::vector<double> lkc;
  std::vector<double> la;
  std::vector<double> p;
  std::vector<double> s;

  // Experimental data
  std::vector<double> Time_ex;
  std::vector<double> Ocon_ex;
  std::vector<double> Variance_ex;
};

template<class P_V, class P_M>
atcLikelihoodRoutine_DataClass<P_V,P_M>::atcLikelihoodRoutine_DataClass(
  const uqBaseEnvironmentClass& env,
  const char* experimentDescriptionFileName,
  const char* reactionFileName,
  const char* scenarioFileName,
  const char* experimentalDataFileName)
  :
#if 1 // KENJI
  Species_con (13,0.),
  Species_mass(13,0.),
  Species_conm(13,0.),
  lkc      (18,0.),
  la        (18,0.),
  p        (18,0.),
  s        (18,0.),
  Time_ex    (4,0.),
  Ocon_ex    (4,0.),
  Variance_ex(4,0.)
{
  // Read description of experiment
  char dummyNAME1[100];
  char dummyNAME2[100];
  char dummyNAME3[100];
  char dummyNAME4[100];
  double tmp_con, tmp_mass;
  unsigned int tmpCount = 0;

  FILE *inp = fopen(experimentDescriptionFileName,"r");
   fscanf(inp,"%s %s      ",dummyNAME1,dummyNAME2);  // NSPECI     NO_STEPS  NO_PARAM
   fscanf(inp,"%d %lf     ",&NREST,&DTIME);
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // NSPECI     NO_STEPS  NO_PARAM
   fscanf(inp,"%d %d %d   ",&NSPECI,&NO_STEPS,&NO_PARAM);                    
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // UREF  TREF_K  PREF_Pa
   fscanf(inp,"%lf %lf %lf",&UREF, &TREF, &PREF);
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // SPNAME  CONCO_mass_base  MWT
   while (fscanf(inp,"%s %lf %lf",Species_name[tmpCount],&tmp_con,&tmp_mass) != EOF) {
    Species_con[tmpCount]  = tmp_con;
    Species_mass[tmpCount] =tmp_mass;
    tmpCount++;
   }
  // Close file
  fclose(inp);

  int  tmp_Num,tmpREAC,tmpPROD;

  inp = fopen(reactionFileName,"r");
  for (int j=0; j<NO_STEPS; j++){
       fscanf(inp,"%s",dummyNAME1);
       fscanf(inp,"%s %d %s",dummyNAME2,&tmp_Num,dummyNAME3);
       fscanf(inp,"%s",dummyNAME4);
       for (int i=0; i<NSPECI; i++) {
          fscanf(inp,"%d",&tmpREAC);
          reac[j][i] = tmpREAC;
       }
       for (int i=0; i<NSPECI; i++) {
          fscanf(inp,"%d",&tmpPROD);
          prod[j][i] = tmpPROD;
       }
  }
  // Close reaction file
  fclose(inp);

  if (scenarioFileName && experimentalDataFileName) {
    // Read scenario parameters
     if (env.rank() == 0) {
     std::cout << "In atcLikelihoodRoutine_DataClass(), reading file '"
               << scenarioFileName << "'\n"
               << std::endl;
     }
       inp = fopen(scenarioFileName,"r");    
       double tmpEQC,tmpA,tmpms,tmpTs;
       fscanf(inp,"%lf",&ex_SystemTemperature); 
       for (int i=0; i<NO_STEPS; i++){
		fscanf(inp,"%s %s %s %s",dummyNAME1,dummyNAME2,dummyNAME3,dummyNAME4);
                fscanf(inp,"%lf %lf %lf %lf",&tmpEQC,&tmpA,&tmpms,&tmpTs);
                lkc[i] = tmpEQC;
                la[i]   = tmpA;
                p[i]   = tmpms;
                s[i]   = tmpTs;
       }

    // Close file
    fclose(inp);

    // Read experimental data
    if (env.rank() == 0) {
      std::cout << "In atcLikelihoodRoutine_DataClass(), reading file '"
                << experimentalDataFileName << "'\n"
                << std::endl;
    }
    inp = fopen(experimentalDataFileName,"r");
    unsigned int numObservations = 0;
    double tmpTime;
    double tmpOcon;
    double tmpVar;
    fscanf(inp,"%d",&NO_EXP);
    while (fscanf(inp,"%lf %lf %lf",&tmpTime,&tmpOcon,&tmpVar) != EOF) {
      Time_ex[numObservations] = tmpTime;
      Ocon_ex[numObservations] = tmpOcon;
      Variance_ex[numObservations] = tmpVar;
      numObservations++;
    }
    fclose(inp);
  }

//   initial set up for concentration
    double tmpRG = 0.0;
    double universalGasConstant = 8.31451e3;
    for (int i=0; i < NSPECI; i++){
      tmpRG  +=  universalGasConstant / Species_mass[i] * Species_con[i];
    }
    RHOREF =  PREF / (tmpRG * TREF);

    for (int i=0; i < NSPECI; i++){
      Species_conm[i] = RHOREF * Species_con[i] / Species_mass[i]; // Kmol/m^3
    }

#endif // KENJI
}

template<class P_V, class P_M>
atcLikelihoodRoutine_DataClass<P_V,P_M>::~atcLikelihoodRoutine_DataClass()
{
}

// The actual (user defined) likelihood routine
template<class P_V,class P_M>
double
atcLikelihoodRoutine(const P_V& paramValues, const void* functionDataPtr)
{
  double resultValue = 0.;

  // Compute likelihood given parameters
  double la_param  = paramValues[0];    // 19th Reaction parameters  Log_10 (A)
  double p_param   = paramValues[1];    // 19th Reaction parameters  T^p
  double s_param   = paramValues[2];    // 19th Reaction parameters  exp(-s/T)
  double lkc_param = paramValues[3];    // 19th Reaction parameters  K_b=k_f/Kc

  double Ts       = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->ex_SystemTemperature;
  double DTIME    = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->DTIME;
  int    NO_EXP   = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->NO_EXP;
  int    NREST    = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->NREST;
  int    NSPECI   = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->NSPECI;
  int    NO_STEPS = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->NO_STEPS;
  const std::vector<double>& lkc = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->lkc;
  const std::vector<double>& la = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->la;
  const std::vector<double>& p = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->p;
  const std::vector<double>& s = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->s;
  const std::vector<double>& Time_ex = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->Time_ex;
  const std::vector<double>& Ocon_ex = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->Ocon_ex;
  const std::vector<double>& Variance_ex = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->Variance_ex;
  const std::vector<double>& conmoInitial = ((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->Species_conm;
  int reac[NO_STEPS][NSPECI];
  int prod[NO_STEPS][NSPECI];
  double Ocon_num[NO_EXP];

  for (int i = 0; i < NO_STEPS; ++i) {
     for (int j = 0; j < NSPECI; ++j) {
         reac[i][j] = (((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->reac)[i][j];
         prod[i][j] = (((atcLikelihoodRoutine_DataClass<P_V,P_M> *) functionDataPtr)->prod)[i][j];
     }
  }
  std::vector<double>      conmo(NSPECI,0.);
  std::vector<double>      conmo_old(NSPECI,0.);

  double WDOT[2][NSPECI];
  double WKIN[NO_STEPS];
  double RKF[NO_STEPS];
  double RKB[NO_STEPS];
  double CO,CN,CC,CARG,CO2,CN2,CNO,CCO,CCN,CNCO,CNCN,CN2O,CC2N2,time,numAvgad;

  for (int i=0; i<NO_STEPS-1; i++){
     WKIN[i] = 0.0;    
     RKF[i]   = (pow(10.,la[i])) * (pow(Ts,p[i])) * exp(- s[i] / Ts); 
     RKB[i]   = RKF[i] / pow(10.,lkc[i]);
  }
  RKF[NO_STEPS-1]   = (pow(10.,la_param)) * (pow(Ts,p_param)) * exp(- s_param / Ts); 
  RKB[NO_STEPS-1]   = RKF[NO_STEPS-1] / pow(10.,lkc_param);
//  RKF[NO_STEPS-1]   = (pow(10.,12.66)) * (pow(Ts,0.0)) * exp(- 1500. / Ts); 
//  RKB[NO_STEPS-1]   = RKF[NO_STEPS-1] / pow(10.,0.5);


  for (int i=0; i<NSPECI; i++){
     conmo[i]  = conmoInitial[i] / 1e+3;       // kmol/m^3 -> mol/cm^3
  }
  for (int TSP=0; TSP<NREST; TSP++) {
     time = TSP * DTIME;

     for (int i=0; i<NSPECI; i++){
          conmo_old[i]  = conmo[i]; 
     }

     for (int NSP=0; NSP<2; NSP++) {
          CO    = conmo[0];
          CN    = conmo[1];
          CC    = conmo[2];
          CARG  = conmo[3];
          CO2   = conmo[4];
          CN2   = conmo[5];
          CNO   = conmo[6];
          CCO   = conmo[7];
          CCN   = conmo[8];
          CNCO  = conmo[9];;
          CNCN  = conmo[10];
          CN2O  = conmo[11];
          CC2N2 = conmo[12];

          WKIN[0]  = -RKF[0]  * CN2O  * CARG + RKB[0]  * CN2  * CO * CARG;
          WKIN[1]  = -RKF[1]  * CC2N2 * CO   + RKB[1]  * CCN  * CNCO;
          WKIN[2]  = -RKF[2]  * CNCO  * CO   + RKB[2]  * CCO  * CNO;
          WKIN[3]  = -RKF[3]  * CNCO  * CARG + RKB[3]  * CN   * CCO * CARG;
          WKIN[4]  = -RKF[4]  * CCN   * CO2  + RKB[4]  * CNCO * CO;
          WKIN[5]  = -RKF[5]  * CCN   * CO   + RKB[5]  * CCO  * CN;
          WKIN[6]  = -RKF[6]  * CN2O  * CO   + RKB[6]  * CNO  * CNO;
          WKIN[7]  = -RKF[7]  * CN2O  * CO   + RKB[7]  * CN2  * CO2;
          WKIN[8]  = -RKF[8]  * CN2   * CO   + RKB[8]  * CN   * CNO;
          WKIN[9]  = -RKF[9]  * CNO   * CO   + RKB[9]  * CN   * CO2;
          WKIN[10] = -RKF[10] * CNCO  * CN   + RKB[10] * CN2  * CCO;
          WKIN[11] = -RKF[11] * CNCO  * CN   + RKB[11] * CCN  * CNO;
          WKIN[12] = -RKF[12] * CNCO  * CC   + RKB[12] * CCN  * CCO;
          WKIN[13] = -RKF[13] * CCN   * CN   + RKB[13] * CC   * CN2;
          WKIN[14] = -RKF[14] * CN2O  * CCN  + RKB[14] * CNCN * CNO;
          WKIN[15] = -RKF[15] * CNCN  * CO   + RKB[15] * CCN  * CNO;
          WKIN[16] = -RKF[16] * CNCN  * CN   + RKB[16] * CCN  * CN2;
          WKIN[17] = -RKF[17] * CC2N2 * CARG + RKB[17] * CCN  * CCN * CARG;
          WKIN[18] = -RKF[18] * CC2N2 * CO   + RKB[18] * CCO  * CNCN;


          for(int NR = 0; NR < NSPECI; NR++) {
             WDOT[NSP][NR] = 0.0;
          }

          for(int NS = 0; NS < NSPECI; NS++){
             for(int NR = 0; NR < NO_STEPS; NR++){
                WDOT[NSP][NS] += (reac[NR][NS]-prod[NR][NS])*WKIN[NR];
             }
          }

          if( NSP == 0) {
             for(int NS = 0; NS < NSPECI; NS++){
               conmo[NS] = conmo_old[NS] + DTIME * WDOT[0][NS];
             }
          }
          else if (NSP == 1) {
             for(int NS = 0; NS < NSPECI; NS++){
               conmo[NS] = conmo_old[NS] + 0.5e0 *  DTIME *  ( WDOT[1][NS] + WDOT[1][NS] );
             }
          }
     }

     for(int NS = 0; NS < NO_EXP; NS++){
          if( time <= Time_ex[NS] ){
             Ocon_num[NS] = conmo[0];
          }  
     } 

  } // for TSP

//    Calculate misfit
  resultValue = 0.0;
  numAvgad    = 6.02214179e+23;

  for(int NS = 0; NS < NO_EXP; NS++){
    Ocon_num[NS] = Ocon_num[NS] * numAvgad / 1e+13;      //  (# / 1D+13)   /cm^3
    resultValue += (Ocon_num[NS]-Ocon_ex[NS])* (Ocon_num[NS]-Ocon_ex[NS]) / Variance_ex[NS];
  }

  for (int i=0; i<NSPECI; i++){
     conmo[i]  = conmo[i] * 1e+3;   // mol/cm^3 -> kmol/m^3
  }
  return resultValue;
}


//********************************************************
// QoI function object for both forward problems of the validation cycle.
// A QoI function object is provided by user and is called by the UQ library.
// This QoI function object consists of data and routine.
//********************************************************
// The (user defined) data class that carries the data needed by the (user defined) qoi routine
template<class P_V,class P_M,class Q_V, class Q_M>
struct
atcQoiRoutine_DataClass
{
  atcQoiRoutine_DataClass(const uqBaseEnvironmentClass& env,
                          const char* experimentDescriptionFileName,
                          const char* reactionFileName,
                          const char* scenarioFileName);
 ~atcQoiRoutine_DataClass();

  double m_criticalmaxOcon;     // Not read from file, but set inside program instead
  double m_criticalmaxOcontime; // Not read from file, but set inside program instead

  // Experiment description
  int    NREST;
  int    NSPECI;
  int    NO_STEPS;
  int    NO_PARAM;
  int    NO_EXP;
  double DTIME;
  double UREF;
  double TREF;
  double PREF;
  double RHOREF;
  char Species_name[13][100];
  std::vector<double> Species_con;
  std::vector<double> Species_mass;
  std::vector<double> Species_conm;  // Initial concentrations: set in constructor

  // Reaction data
  int  reac[19][13];
  int  prod[19][13];

  // Scenario parameters
  double m_SystemTemperature;
  std::vector<double> lkc;
  std::vector<double> la;
  std::vector<double> p;
  std::vector<double> s;
};

template<class P_V, class P_M,class Q_V, class Q_M>
atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>::atcQoiRoutine_DataClass(
  const uqBaseEnvironmentClass& env,
  const char* experimentDescriptionFileName,
  const char* reactionFileName,
  const char* scenarioFileName)
  :
  Species_con (13,0.),
  Species_mass(13,0.),
  Species_conm(13,0.),
  lkc      (18,0.),
  la        (18,0.),
  p        (18,0.),
  s        (18,0.)
{
  // Read description of experiment
  char dummyNAME1[100];
  char dummyNAME2[100];
  char dummyNAME3[100];
  char dummyNAME4[100];
  double tmp_con, tmp_mass;
  unsigned int tmpCount = 0;

  FILE *inp = fopen(experimentDescriptionFileName,"r");
   fscanf(inp,"%s %s      ",dummyNAME1,dummyNAME2);  // NSPECI     NO_STEPS  NO_PARAM
   fscanf(inp,"%d %lf     ",&NREST,&DTIME);
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // NSPECI     NO_STEPS  NO_PARAM
   fscanf(inp,"%d %d %d   ",&NSPECI,&NO_STEPS,&NO_PARAM);                    
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // UREF  TREF_K  PREF_Pa
   fscanf(inp,"%lf %lf %lf",&UREF, &TREF, &PREF);
   fscanf(inp,"%s %s %s   ",dummyNAME1,dummyNAME2,dummyNAME3);  // SPNAME  CONCO_mass_base  MWT
   while (fscanf(inp,"%s %lf %lf",Species_name[tmpCount],&tmp_con,&tmp_mass) != EOF) {
    Species_con[tmpCount]  = tmp_con;
    Species_mass[tmpCount] =tmp_mass;
    tmpCount++;
   }
  // Close file
  fclose(inp);

  int  tmp_Num,tmpREAC,tmpPROD;

  inp = fopen(reactionFileName,"r");
  for (int j=0; j<NO_STEPS; j++){
       fscanf(inp,"%s",dummyNAME1);
       fscanf(inp,"%s %d %s",dummyNAME2,&tmp_Num,dummyNAME3);
       fscanf(inp,"%s",dummyNAME4);
       for (int i=0; i<NSPECI; i++) {
          fscanf(inp,"%d",&tmpREAC);
          reac[j][i] = tmpREAC;
       }
       for (int i=0; i<NSPECI; i++) {
          fscanf(inp,"%d",&tmpPROD);
          prod[j][i] = tmpPROD;
       }
  }
  // Close reaction file
  fclose(inp);

  if (scenarioFileName) {
    // Read scenario parameters
     if (env.rank() == 0) {
     std::cout << "In atcQoiRoutine_DataClass(), reading file '"
               << scenarioFileName << "'\n"
               << std::endl;
     }
       inp = fopen(scenarioFileName,"r");    
       double tmpEQC,tmpA,tmpms,tmpTs;
       fscanf(inp,"%lf",&m_SystemTemperature); 
       for (int i=0; i<NO_STEPS; i++){
		fscanf(inp,"%s %s %s %s",dummyNAME1,dummyNAME2,dummyNAME3,dummyNAME4);
                fscanf(inp,"%lf %lf %lf %lf",&tmpEQC,&tmpA,&tmpms,&tmpTs);
                lkc[i] = tmpEQC;
                la[i]   = tmpA;
                p[i]   = tmpms;
                s[i]   = tmpTs;
       }

    // Close file
    fclose(inp);
  }

//   initial set up for concentration
    double tmpRG = 0.0;
    double universalGasConstant = 8.31451e3;
    for (int i=0; i < NSPECI; i++){
      tmpRG  +=  universalGasConstant / Species_mass[i] * Species_con[i];
    }
    RHOREF =  PREF / (tmpRG * TREF);

    for (int i=0; i < NSPECI; i++){
      Species_conm[i] = RHOREF * Species_con[i] / Species_mass[i]; // Kmol/m^3
    }
}

template<class P_V, class P_M,class Q_V, class Q_M>
    atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>::~atcQoiRoutine_DataClass()
{
}

// The actual (user defined) qoi routine
template<class P_V,class P_M,class Q_V,class Q_M>
void atcQoiRoutine(const P_V& paramValues, const void* functionDataPtr, Q_V& qoiValues)
{
  double la_param  = paramValues[0];    // 19th Reaction parameters  Log_10 (A)
  double p_param   = paramValues[1];    // 19th Reaction parameters  T^p
  double s_param   = paramValues[2];    // 19th Reaction parameters  exp(-s/T)
  double lkc_param = paramValues[3];    // 19th Reaction parameters  K_b=k_f/Kc

  double Ts                  = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->m_SystemTemperature;
  double criticalmaxOcon     = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->m_criticalmaxOcon;
  double criticalmaxOcontime = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->m_criticalmaxOcontime;

  double DTIME    = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->DTIME;
  int    NREST    = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->NREST;
  int    NSPECI   = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->NSPECI;
  int    NO_STEPS = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->NO_STEPS;
  const std::vector<double>& lkc = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->lkc;
  const std::vector<double>& la = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->la;
  const std::vector<double>& p = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->p;
  const std::vector<double>& s = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->s;
  const std::vector<double>& conmoInitial = ((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->Species_conm;
  int reac[NO_STEPS][NSPECI];
  int prod[NO_STEPS][NSPECI];

  for (int i = 0; i < NO_STEPS; ++i) {
     for (int j = 0; j < NSPECI; ++j) {
         reac[i][j] = (((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->reac)[i][j];
         prod[i][j] = (((atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M> *) functionDataPtr)->prod)[i][j];
     }
  }
  std::vector<double>      conmo(NSPECI,0.);
  std::vector<double>      conmo_old(NSPECI,0.);

  double WDOT[2][NSPECI];
  double WKIN[NO_STEPS];
  double RKF[NO_STEPS];
  double RKB[NO_STEPS];
  double CO,CN,CC,CARG,CO2,CN2,CNO,CCO,CCN,CNCO,CNCN,CN2O,CC2N2,time,numAvgad;
  double tmpmaxOcon,tmpmaxOcontime;
  tmpmaxOcon= 0.0;


  for (int i=0; i<NO_STEPS-1; i++){
     WKIN[i] = 0.0;    
     RKF[i]   = (pow(10.,la[i])) * (pow(Ts,p[i])) * exp(- s[i] / Ts); 
     RKB[i]   = RKF[i] / pow(10.,lkc[i]);
  }
  RKF[NO_STEPS-1]   = (pow(10.,la_param)) * (pow(Ts,p_param)) * exp(- s_param / Ts); 
  RKB[NO_STEPS-1]   = RKF[NO_STEPS-1] / pow(10.,lkc_param);
//  RKF[NO_STEPS-1]   = (pow(10.,12.66)) * (pow(Ts,0.0)) * exp(- 1500. / Ts); 
//  RKB[NO_STEPS-1]   = RKF[NO_STEPS-1] / pow(10.,0.5);


  for (int i=0; i<NSPECI; i++){
     conmo[i]  = conmoInitial[i] / 1e+3;       // kmol/m^3 -> mol/cm^3
  }
  for (int TSP=0; TSP<NREST; TSP++) {
     time = TSP * DTIME;

     for (int i=0; i<NSPECI; i++){
          conmo_old[i]  = conmo[i]; 
     }

     for (int NSP=0; NSP<2; NSP++) {
          CO    = conmo[0];
          CN    = conmo[1];
          CC    = conmo[2];
          CARG  = conmo[3];
          CO2   = conmo[4];
          CN2   = conmo[5];
          CNO   = conmo[6];
          CCO   = conmo[7];
          CCN   = conmo[8];
          CNCO  = conmo[9];;
          CNCN  = conmo[10];
          CN2O  = conmo[11];
          CC2N2 = conmo[12];

          WKIN[0]  = -RKF[0]  * CN2O  * CARG + RKB[0]  * CN2  * CO * CARG;
          WKIN[1]  = -RKF[1]  * CC2N2 * CO   + RKB[1]  * CCN  * CNCO;
          WKIN[2]  = -RKF[2]  * CNCO  * CO   + RKB[2]  * CCO  * CNO;
          WKIN[3]  = -RKF[3]  * CNCO  * CARG + RKB[3]  * CN   * CCO * CARG;
          WKIN[4]  = -RKF[4]  * CCN   * CO2  + RKB[4]  * CNCO * CO;
          WKIN[5]  = -RKF[5]  * CCN   * CO   + RKB[5]  * CCO  * CN;
          WKIN[6]  = -RKF[6]  * CN2O  * CO   + RKB[6]  * CNO  * CNO;
          WKIN[7]  = -RKF[7]  * CN2O  * CO   + RKB[7]  * CN2  * CO2;
          WKIN[8]  = -RKF[8]  * CN2   * CO   + RKB[8]  * CN   * CNO;
          WKIN[9]  = -RKF[9]  * CNO   * CO   + RKB[9]  * CN   * CO2;
          WKIN[10] = -RKF[10] * CNCO  * CN   + RKB[10] * CN2  * CCO;
          WKIN[11] = -RKF[11] * CNCO  * CN   + RKB[11] * CCN  * CNO;
          WKIN[12] = -RKF[12] * CNCO  * CC   + RKB[12] * CCN  * CCO;
          WKIN[13] = -RKF[13] * CCN   * CN   + RKB[13] * CC   * CN2;
          WKIN[14] = -RKF[14] * CN2O  * CCN  + RKB[14] * CNCN * CNO;
          WKIN[15] = -RKF[15] * CNCN  * CO   + RKB[15] * CCN  * CNO;
          WKIN[16] = -RKF[16] * CNCN  * CN   + RKB[16] * CCN  * CN2;
          WKIN[17] = -RKF[17] * CC2N2 * CARG + RKB[17] * CCN  * CCN * CARG;
          WKIN[18] = -RKF[18] * CC2N2 * CO   + RKB[18] * CCO  * CNCN;


          for(int NR = 0; NR < NSPECI; NR++) {
             WDOT[NSP][NR] = 0.0;
          }

          for(int NS = 0; NS < NSPECI; NS++){
             for(int NR = 0; NR < NO_STEPS; NR++){
                WDOT[NSP][NS] += (reac[NR][NS]-prod[NR][NS])*WKIN[NR];
             }
          }

          if( NSP == 0) {
             for(int NS = 0; NS < NSPECI; NS++){
               conmo[NS] = conmo_old[NS] + DTIME * WDOT[0][NS];
             }
          }
          else if (NSP == 1) {
             for(int NS = 0; NS < NSPECI; NS++){
               conmo[NS] = conmo_old[NS] + 0.5e0 *  DTIME *  ( WDOT[1][NS] + WDOT[1][NS] );
             }
          }
     }

    if(tmpmaxOcon < conmo[0]) 
    {
       tmpmaxOcon     = conmo[0];
       tmpmaxOcontime = time;
       if(criticalmaxOcon     > 0.)  qoiValues[0] = tmpmaxOcon * numAvgad / 1e+13;  //  (# / 1D+13)   /cm^3;  
       if(criticalmaxOcontime > 0.)  qoiValues[0] = tmpmaxOcontime;
    } 

  } 
	
  return;
}

//********************************************************
// Class "uqAtcValidation", instantiated by main()
//********************************************************
template <class P_V,class P_M,class Q_V,class Q_M>
class uqAtcValidationClass : public uqModelValidationClass<P_V,P_M,Q_V,Q_M>
{
public:
  uqAtcValidationClass(const uqBaseEnvironmentClass& env,
                       const char*               prefix);
 ~uqAtcValidationClass();

  void run();

private:
  void  runCalibrationStage();
  void  runValidationStage();
  void  runComparisonStage();

  using uqModelValidationClass<P_V,P_M,Q_V,Q_M>::m_env;
  using uqModelValidationClass<P_V,P_M,Q_V,Q_M>::m_prefix;
  using uqModelValidationClass<P_V,P_M,Q_V,Q_M>::m_cycle;

  uqAsciiTableClass<P_V,P_M>*               m_paramsTable;
  const EpetraExt::DistArray<std::string>*  m_paramNames;         // instantiated outside this class!!
  P_V*                                      m_paramMinValues;     // instantiated outside this class!!
  P_V*                                      m_paramMaxValues;     // instantiated outside this class!!
  P_V*                                      m_paramInitialValues; // instantiated outside this class!!
  uqVectorSpaceClass<P_V,P_M>*              m_paramSpace;
  uqVectorSetClass<P_V,P_M>*                m_paramDomain;

  uqAsciiTableClass<P_V,P_M>*               m_qoisTable;
  const EpetraExt::DistArray<std::string>*  m_qoiNames; // instantiated outside this class!!
  uqVectorSpaceClass<Q_V,Q_M>*              m_qoiSpace;

#if 1 // KENJI
  double                                    m_predCriticalmaxOcon;
  double                                    m_predCriticalmaxOcontime;
#endif

  uqBaseVectorRVClass<P_V,P_M>*             m_calPriorRv;
  atcLikelihoodRoutine_DataClass<P_V,P_M>*  m_calLikelihoodRoutine_Data;
  uqBaseScalarFunctionClass<P_V,P_M>*       m_calLikelihoodFunctionObj;
  atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>* m_calQoiRoutine_Data;

  atcLikelihoodRoutine_DataClass<P_V,P_M>*  m_valLikelihoodRoutine_Data;
  uqBaseScalarFunctionClass<P_V,P_M>*       m_valLikelihoodFunctionObj;
  atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>* m_valQoiRoutine_Data;
};

template <class P_V,class P_M,class Q_V,class Q_M>
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::uqAtcValidationClass(
  const uqBaseEnvironmentClass& env,
  const char*                   prefix)
  :
  uqModelValidationClass<P_V,P_M,Q_V,Q_M>(env,prefix),
  m_paramsTable              (NULL),
  m_paramNames               (NULL),
  m_paramMinValues           (NULL),
  m_paramMaxValues           (NULL),
  m_paramInitialValues       (NULL),
  m_paramSpace               (NULL),
  m_paramDomain              (NULL),
  m_qoisTable                (NULL),
  m_qoiNames                 (NULL),
  m_qoiSpace                 (NULL),
  m_calPriorRv               (NULL),
  m_calLikelihoodRoutine_Data(NULL),
  m_calLikelihoodFunctionObj (NULL),
  m_calQoiRoutine_Data       (NULL),
  m_valLikelihoodRoutine_Data(NULL),
  m_valLikelihoodFunctionObj (NULL),
  m_valQoiRoutine_Data       (NULL)
{
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::constructor()\n"
              << std::endl;
  }

  // Read Ascii file with information on parameters
  m_paramsTable = new uqAsciiTableClass<P_V,P_M> (m_env,
                                                  4,    // # of rows
                                                  3,    // # of cols after 'parameter name': min + max + initial value for Markov chain
                                                  NULL, // All extra columns are of 'double' type
                                                  "params.tab");

  m_paramNames = &(m_paramsTable->stringColumn(0));
  m_paramMinValues     = new P_V(m_paramsTable->doubleColumn(1));
  m_paramMaxValues     = new P_V(m_paramsTable->doubleColumn(2));
  m_paramInitialValues = new P_V(m_paramsTable->doubleColumn(3));

  m_paramSpace = new uqVectorSpaceClass<P_V,P_M>(m_env,
                                                 "param_", // Extra prefix before the default "space_" prefix
                                                 m_paramsTable->numRows(),
                                                 m_paramNames);

  m_paramDomain = new uqBoxSubsetClass<P_V,P_M>("param_",
                                                *m_paramSpace,
                                                *m_paramMinValues,
                                                *m_paramMaxValues);

  // Read Ascii file with information on qois
  m_qoisTable = new uqAsciiTableClass<P_V,P_M>(m_env,
                                               1,    // # of rows
                                               0,    // # of cols after 'parameter name': none
                                               NULL, // All extra columns are of 'double' type
                                               "qois.tab");

  m_qoiNames = &(m_qoisTable->stringColumn(0));

  m_qoiSpace = new uqVectorSpaceClass<Q_V,Q_M>(m_env,
                                               "qoi_", // Extra prefix before the default "space_" prefix
                                               m_qoisTable->numRows(),
                                               m_qoiNames);

  // Instantiate the validation cycle
  m_cycle = new uqValidationCycleClass<P_V,P_M,Q_V,Q_M>(m_env,
                                                        m_prefix.c_str(), // Use the prefix passed above
                                                        *m_paramSpace,
                                                        *m_qoiSpace);

#if 1 // KENJI
  m_predCriticalmaxOcon     = 0.;
  m_predCriticalmaxOcontime = 1.;
#endif // KENJI

  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::constructor()\n"
              << std::endl;
  }

  return;
}

template <class P_V,class P_M,class Q_V,class Q_M>
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::~uqAtcValidationClass()
{
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::destructor()"
              << std::endl;
  }

  if (m_valQoiRoutine_Data)        delete m_valQoiRoutine_Data;
  if (m_valLikelihoodFunctionObj)  delete m_valLikelihoodFunctionObj;
  if (m_valLikelihoodRoutine_Data) delete m_valLikelihoodRoutine_Data;
  if (m_calQoiRoutine_Data)        delete m_calQoiRoutine_Data;
  if (m_calLikelihoodFunctionObj)  delete m_calLikelihoodFunctionObj;
  if (m_calLikelihoodRoutine_Data) delete m_calLikelihoodRoutine_Data;
  if (m_calPriorRv)                delete m_calPriorRv;
  if (m_qoiSpace)                  delete m_qoiSpace;
//if (m_qoiNames)                  delete m_qoiNames; // instantiated outside this class!!
  if (m_qoisTable)                 delete m_qoisTable;
  if (m_paramDomain)               delete m_paramDomain;
  if (m_paramSpace)                delete m_paramSpace;
//if (m_paramInitialValues)        delete m_paramInitialValues; // instantiated outside this class!!
//if (m_paramMaxValues)            delete m_paramMaxValues;     // instantiated outside this class!!
//if (m_paramMinValues)            delete m_paramMinValues;     // instantiated outside this class!!
//if (m_paramNames)                delete m_paramNames;         // instantiated outside this class!!
  if (m_paramsTable)               delete m_paramsTable;

  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::destructor()"
              << std::endl;
  }
}

template <class P_V,class P_M,class Q_V,class Q_M>
void
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::run()
{
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::run()"
              << std::endl;
  }
  
  runCalibrationStage();
  runValidationStage();
  runComparisonStage();

  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::run()"
              << std::endl;
  }

  return;
}

template<class P_V,class P_M,class Q_V,class Q_M>
void 
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::runCalibrationStage()
{
  int iRC;
  struct timeval timevalRef;
  struct timeval timevalNow;

  iRC = gettimeofday(&timevalRef, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::runCalibrationStage() at " << ctime(&timevalRef.tv_sec)
              << std::endl;
  }

  // Deal with inverse problem
  m_calPriorRv = new uqUniformVectorRVClass<P_V,P_M> ("cal_prior_", // Extra prefix before the default "rv_" prefix
                                                      *m_paramDomain);

  m_calLikelihoodRoutine_Data = new atcLikelihoodRoutine_DataClass<P_V,P_M>(m_env,
                                                                            "input.dat",
                                                                            "reaction.dat", 
                                                                           "scenario_001.dat",
                                                                          "Experimental_001.dat");
  m_calLikelihoodFunctionObj = new uqGenericScalarFunctionClass<P_V,P_M>("cal_like_",
                                                                         *m_paramDomain,
                                                                         atcLikelihoodRoutine<P_V,P_M>,
                                                                         NULL,
                                                                         NULL,
                                                                         (void *) m_calLikelihoodRoutine_Data,
                                                                         true); // the routine computes [-2.*ln(function)]
  m_cycle->setCalIP(*m_calPriorRv,
                    *m_calLikelihoodFunctionObj);
  // Solve inverse problem = set 'pdf' and 'realizer' of 'postRv'
  P_M* calProposalCovMatrix = m_cycle->calIP().postRv().imageSet().vectorSpace().newGaussianMatrix(m_cycle->calIP().priorRv().pdf().domainVarVector(),
                                                                                               *m_paramInitialValues);
  m_cycle->calIP().solveWithBayesMarkovChain(*m_paramInitialValues,
                                             calProposalCovMatrix);
  delete calProposalCovMatrix;
  // Deal with forward problem
  m_calQoiRoutine_Data = new atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>(m_env,
                                                                      "input.dat",
                                                                      "reaction.dat", 
                                                                      "scenario_003.dat");
  m_calQoiRoutine_Data->m_criticalmaxOcon     = m_predCriticalmaxOcon;
  m_calQoiRoutine_Data->m_criticalmaxOcontime = m_predCriticalmaxOcontime;
  m_cycle->setCalFP(atcQoiRoutine<P_V,P_M,Q_V,Q_M>,
                    (void *) m_calQoiRoutine_Data);
  // Solve forward problem = set 'realizer' and 'cdf' of 'qoiRv'
  m_cycle->calFP().solveWithMonteCarlo(); // no extra user entities needed for Monte Carlo algorithm

  iRC = gettimeofday(&timevalNow, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::runCalibrationStage() at " << ctime(&timevalNow.tv_sec)
              << "Total uqAtcValidation::runCalibrationStage() run time = " << timevalNow.tv_sec - timevalRef.tv_sec
              << " seconds"
              << std::endl;
  }

  return;
}

template<class P_V,class P_M,class Q_V,class Q_M>
void 
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::runValidationStage()
{
  int iRC;
  struct timeval timevalRef;
  struct timeval timevalNow;

  iRC = gettimeofday(&timevalRef, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::runValidationStage() at " << ctime(&timevalRef.tv_sec)
              << std::endl;
  }

  // Deal with inverse problem
  m_valLikelihoodRoutine_Data = new atcLikelihoodRoutine_DataClass<P_V,P_M>(m_env,
                                                                            "input.dat",
                                                                            "reaction.dat",
                                                                            "scenario_002.dat",
                                                                            "Experimental_002.dat");

  m_valLikelihoodFunctionObj = new uqGenericScalarFunctionClass<P_V,P_M>("val_like_",
                                                                         *m_paramDomain,
                                                                         atcLikelihoodRoutine<P_V,P_M>,
                                                                         NULL,
                                                                         NULL,
                                                                         (void *) m_valLikelihoodRoutine_Data,
                                                                         true); // the routine computes [-2.*ln(function)]

  m_cycle->setValIP(*m_valLikelihoodFunctionObj);

  // Solve inverse problem = set 'pdf' and 'realizer' of 'postRv'
  P_M* valProposalCovMatrix = m_cycle->calIP().postRv().imageSet().vectorSpace().newGaussianMatrix(m_cycle->calIP().postRv().realizer().imageVarVector(),  // Use 'realizer()' because the posterior rv was computed with Markov Chain
                                                                                                   m_cycle->calIP().postRv().realizer().imageExpVector()); // Use these values as the initial values
  m_cycle->valIP().solveWithBayesMarkovChain(m_cycle->calIP().postRv().realizer().imageExpVector(),
                                             valProposalCovMatrix);
  delete valProposalCovMatrix;

  // Deal with forward problem
  m_valQoiRoutine_Data = new atcQoiRoutine_DataClass<P_V,P_M,Q_V,Q_M>(m_env,
                                                                      "input.dat",
                                                                      "reaction.dat", 
                                                                      "scenario_003.dat");
  m_valQoiRoutine_Data->m_criticalmaxOcon     = m_predCriticalmaxOcon;
  m_valQoiRoutine_Data->m_criticalmaxOcontime = m_predCriticalmaxOcontime;

  m_cycle->setValFP(atcQoiRoutine<P_V,P_M,Q_V,Q_M>,
                    (void *) m_valQoiRoutine_Data);

  // Solve forward problem = set 'realizer' and 'cdf' of 'qoiRv'
  m_cycle->valFP().solveWithMonteCarlo(); // no extra user entities needed for Monte Carlo algorithm

  iRC = gettimeofday(&timevalNow, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::runValidationStage() at " << ctime(&timevalNow.tv_sec)
              << "Total uqAtcValidation::runValidationStage() run time = " << timevalNow.tv_sec - timevalRef.tv_sec
              << " seconds"
              << std::endl;
  }

  return;
}

template<class P_V,class P_M,class Q_V,class Q_M>
void 
uqAtcValidationClass<P_V,P_M,Q_V,Q_M>::runComparisonStage()
{
  int iRC;
  struct timeval timevalRef;
  struct timeval timevalNow;

  iRC = gettimeofday(&timevalRef, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Entering uqAtcValidation::runComparisonStage() at " << ctime(&timevalRef.tv_sec)
              << std::endl;
  }

  if (m_cycle->calFP().computeSolutionFlag() &&
      m_cycle->valFP().computeSolutionFlag()) {
    Q_V* epsilonVec = m_cycle->calFP().qoiRv().imageSet().vectorSpace().newVector(0.02);
    Q_V cdfDistancesVec(m_cycle->calFP().qoiRv().imageSet().vectorSpace().zeroVector());
    horizontalDistances(m_cycle->calFP().qoiRv().cdf(),
                        m_cycle->valFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "    << *epsilonVec
                << ", cdfDistancesVec = " << cdfDistancesVec
                << std::endl;
    }

    // Test independence of 'distance' w.r.t. order of cdfs
    horizontalDistances(m_cycle->valFP().qoiRv().cdf(),
                        m_cycle->calFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "                             << *epsilonVec
                << ", cdfDistancesVec (switched order of cdfs) = " << cdfDistancesVec
                << std::endl;
    }

    // Epsilon = 0.04
    epsilonVec->cwSet(0.04);
    horizontalDistances(m_cycle->calFP().qoiRv().cdf(),
                        m_cycle->valFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "    << *epsilonVec
                << ", cdfDistancesVec = " << cdfDistancesVec
                << std::endl;
    }

    // Epsilon = 0.06
    epsilonVec->cwSet(0.06);
    horizontalDistances(m_cycle->calFP().qoiRv().cdf(),
                        m_cycle->valFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "    << *epsilonVec
                << ", cdfDistancesVec = " << cdfDistancesVec
                << std::endl;
    }

    // Epsilon = 0.08
    epsilonVec->cwSet(0.08);
    horizontalDistances(m_cycle->calFP().qoiRv().cdf(),
                        m_cycle->valFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "    << *epsilonVec
                << ", cdfDistancesVec = " << cdfDistancesVec
                << std::endl;
    }

    // Epsilon = 0.10
    epsilonVec->cwSet(0.10);
    horizontalDistances(m_cycle->calFP().qoiRv().cdf(),
                        m_cycle->valFP().qoiRv().cdf(),
                        *epsilonVec,
                        cdfDistancesVec);
    if (m_cycle->env().rank() == 0) {
      std::cout << "For epsilonVec = "    << *epsilonVec
                << ", cdfDistancesVec = " << cdfDistancesVec
                << std::endl;
    }

    delete epsilonVec;
  }

  iRC = gettimeofday(&timevalNow, NULL);
  if (m_env.rank() == 0) {
    std::cout << "Leaving uqAtcValidation::runComparisonStage() at " << ctime(&timevalNow.tv_sec)
              << "Total uqAtcValidation::runComparisonStage() run time = " << timevalNow.tv_sec - timevalRef.tv_sec
              << " seconds"
              << std::endl;
  }

  return;
}
#endif // __UQ_ATC_VALIDATION_H__
