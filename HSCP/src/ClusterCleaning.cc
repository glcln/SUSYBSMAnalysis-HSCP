#include "SUSYBSMAnalysis/HSCP/interface/ClusterCleaning.h"
#include <iostream>


ClusterCleaning::ClusterCleaning(std::vector<uint16_t> ampls) {
  amplitudes_ = ampls;
}

ClusterCleaning::~ClusterCleaning() {}


bool ClusterCleaning::passClusterCleaning(int crosstalkInv, uint16_t* exitCode){
  // ---------------- Count the number of maximas    --------------------------
  //----------------------------------------------------------------------------
  //std::cout<<"\t\t [+] Entering ClusterCleaning code using amplitudes:"<<std::endl; //EMERY//
  //for(unsigned int i=0;i<amplitudes_.size();i++) std::cout << "\t\t\t"<<i<<": "<<amplitudes_[i]<<std::endl;//EMERY
  Int_t NofMax = 0;
  Int_t recur255 = 1;
  Int_t recur254 = 1;
  bool MaxOnStart = false;
  bool MaxInMiddle = false, MaxOnEnd = false;
  Int_t MaxPos = 0;
  // Start with a max
  if (amplitudes_.size() != 1 &&
      ((amplitudes_[0] > amplitudes_[1]) ||
       (amplitudes_.size() > 2 && amplitudes_[0] == amplitudes_[1] && amplitudes_[1] > amplitudes_[2] && amplitudes_[0] != 254 && amplitudes_[0] != 255) ||
       (amplitudes_.size() == 2 && amplitudes_[0] == amplitudes_[1] && amplitudes_[0] != 254 && amplitudes_[0] != 255))) {
    NofMax = NofMax + 1;
    MaxOnStart = true;
  }
  //std::cout<<"\t\t [+] MaxOnStart: "<<MaxOnStart<<std::endl; //EMERY//

  // Max in the middle (strip between other ones)
  if (amplitudes_.size() > 2) {
    for (unsigned int i = 1; i < amplitudes_.size() - 1; i++) {
      if ((amplitudes_[i] > amplitudes_[i - 1] && amplitudes_[i] > amplitudes_[i + 1]) ||
          (amplitudes_.size() > 3 && i > 0 && i < amplitudes_.size() - 2 && amplitudes_[i] == amplitudes_[i + 1] && amplitudes_[i] > amplitudes_[i - 1] &&
           amplitudes_[i] > amplitudes_[i + 2] && amplitudes_[i] != 254 && amplitudes_[i] != 255)) {
        NofMax = NofMax + 1;
        MaxInMiddle = true;
        MaxPos = i;
      }
      if (amplitudes_[i] == 255 && amplitudes_[i] == amplitudes_[i - 1]) {
        recur255 = recur255 + 1;
        MaxPos = i - (recur255 / 2);
        if (amplitudes_[i] > amplitudes_[i + 1]) {
          NofMax = NofMax + 1;
          MaxInMiddle = true;
        }
      }
      if (amplitudes_[i] == 254 && amplitudes_[i] == amplitudes_[i - 1]) {
        recur254 = recur254 + 1;
        MaxPos = i - (recur254 / 2);
        if (amplitudes_[i] > amplitudes_[i + 1]) {
          NofMax = NofMax + 1;
          MaxInMiddle = true;
        }
      }
    }
  }
  // Max at the end of the cluster
  if (amplitudes_.size() > 1) {
    if (amplitudes_[amplitudes_.size() - 1] > amplitudes_[amplitudes_.size() - 2] ||
        (amplitudes_.size() > 2 && amplitudes_[amplitudes_.size() - 1] == amplitudes_[amplitudes_.size() - 2] &&
         amplitudes_[amplitudes_.size() - 2] > amplitudes_[amplitudes_.size() - 3]) ||
        amplitudes_[amplitudes_.size() - 1] == 255) {
      NofMax = NofMax + 1;
      MaxOnEnd = true;
    }
  }
  //std::cout<<"\t\t [+] MaxOnEnd: "<<MaxOnEnd<<std::endl; //EMERY//
  // If only one strip is hit
  if (amplitudes_.size() == 1) {
    NofMax = 1;
  }

  // --- SHAPE SELECTION
  //------------------------------------------------------------------------
  //
  //               ____
  //              |    |____
  //          ____|    |    |
  //         |    |    |    |____
  //     ____|    |    |    |    |
  //    |    |    |    |    |    |____
  //  __|____|____|____|____|____|____|__
  //    C_Mnn C_Mn C_M  C_D  C_Dn C_Dnn
  //
  //   bool shapetest=true;
  bool shapecdtn = false;
  if (exitCode)
    *exitCode = 255;

  if (crosstalkInv == 1) {
    if (NofMax == 1) {
      shapecdtn = true;
      if (exitCode)
        *exitCode = 0;
    }
    return shapecdtn;
  }

  //      Float_t C_M;    Float_t C_D;    Float_t C_Mn;   Float_t C_Dn;   Float_t C_Mnn;  Float_t C_Dnn;
  Float_t C_M = 0.0;
  Float_t C_D = 0.0;
  Float_t C_Mn = 10000;
  Float_t C_Dn = 10000;
  Float_t C_Mnn = 10000;
  Float_t C_Dnn = 10000;
  Int_t CDPos;
  Float_t coeff1 = 1.7;
  Float_t coeff2 = 2.0;
  Float_t coeffn = 0.10;
  Float_t coeffnn = 0.02;
  Float_t noise = 4.0;

  if (NofMax == 1) {
    if (MaxOnStart == true) {
      C_M = (Float_t)amplitudes_[0];
      C_D = (Float_t)amplitudes_[1];
      if (amplitudes_.size() < 3)
        shapecdtn = true;
      else if (amplitudes_.size() == 3) {
        C_Dn = (Float_t)amplitudes_[2];
        if (C_Dn <= coeff1 * coeffn * C_D + coeff2 * coeffnn * C_M + 2 * noise || C_D == 255)
          shapecdtn = true;
        else if (exitCode)
          *exitCode = 2;
      } else if (amplitudes_.size() > 3) {
        C_Dn = (Float_t)amplitudes_[2];
        C_Dnn = (Float_t)amplitudes_[3];
        if ((C_Dn <= coeff1 * coeffn * C_D + coeff2 * coeffnn * C_M + 2 * noise || C_D == 255) &&
            C_Dnn <= coeff1 * coeffn * C_Dn + coeff2 * coeffnn * C_D + 2 * noise) {
          shapecdtn = true;
        } else if (exitCode)
          *exitCode = 3;
      }
    }

    if (MaxOnEnd == true) {
      C_M = (Float_t)amplitudes_[amplitudes_.size() - 1];
      C_D = (Float_t)amplitudes_[amplitudes_.size() - 2];
      if (amplitudes_.size() < 3)
        shapecdtn = true;
      else if (amplitudes_.size() == 3) {
        C_Dn = (Float_t)amplitudes_[0];
        if (C_Dn <= coeff1 * coeffn * C_D + coeff2 * coeffnn * C_M + 2 * noise || C_D == 255)
          shapecdtn = true;
        else if (exitCode)
          *exitCode = 4;
      } else if (amplitudes_.size() > 3) {
        C_Dn = (Float_t)amplitudes_[amplitudes_.size() - 3];
        C_Dnn = (Float_t)amplitudes_[amplitudes_.size() - 4];
        if ((C_Dn <= coeff1 * coeffn * C_D + coeff2 * coeffnn * C_M + 2 * noise || C_D == 255) &&
            C_Dnn <= coeff1 * coeffn * C_Dn + coeff2 * coeffnn * C_D + 2 * noise) {
          shapecdtn = true;
        } else if (exitCode)
          *exitCode = 5;
      }
    }

    if (MaxInMiddle == true) {
      C_M = (Float_t)amplitudes_[MaxPos];
      int LeftOfMaxPos = MaxPos - 1;
      if (LeftOfMaxPos <= 0)
        LeftOfMaxPos = 0;
      int RightOfMaxPos = MaxPos + 1;
      if (RightOfMaxPos >= (int)amplitudes_.size())
        RightOfMaxPos = amplitudes_.size() - 1;
      //int after = RightOfMaxPos; int before = LeftOfMaxPos; if (after>=(int)amplitudes_.size() ||  before<0)  std::cout<<"invalid read MaxPos:"<<MaxPos <<"size:"<<amplitudes_.size() <<std::endl;
      if (amplitudes_[LeftOfMaxPos] < amplitudes_[RightOfMaxPos]) {
        C_D = (Float_t)amplitudes_[RightOfMaxPos];
        C_Mn = (Float_t)amplitudes_[LeftOfMaxPos];
        CDPos = RightOfMaxPos;
      } else {
        C_D = (Float_t)amplitudes_[LeftOfMaxPos];
        C_Mn = (Float_t)amplitudes_[RightOfMaxPos];
        CDPos = LeftOfMaxPos;
      }
      if (C_Mn < coeff1 * coeffn * C_M + coeff2 * coeffnn * C_D + 2 * noise || C_M == 255) {
        if (amplitudes_.size() == 3)
          shapecdtn = true;
        else if (amplitudes_.size() > 3) {
          if (CDPos > MaxPos) {
            if (amplitudes_.size() - CDPos - 1 == 0) {
              C_Dn = 0;
              C_Dnn = 0;
            }
            if (amplitudes_.size() - CDPos - 1 == 1) {
              C_Dn = (Float_t)amplitudes_[CDPos + 1];
              C_Dnn = 0;
            }
            if (amplitudes_.size() - CDPos - 1 > 1) {
              C_Dn = (Float_t)amplitudes_[CDPos + 1];
              C_Dnn = (Float_t)amplitudes_[CDPos + 2];
            }
            if (MaxPos >= 2) {
              C_Mnn = (Float_t)amplitudes_[MaxPos - 2];
            } else if (MaxPos < 2)
              C_Mnn = 0;
          }
          if (CDPos < MaxPos) {
            if (CDPos == 0) {
              C_Dn = 0;
              C_Dnn = 0;
            }
            if (CDPos == 1) {
              C_Dn = (Float_t)amplitudes_[0];
              C_Dnn = 0;
            }
            if (CDPos > 1) {
              C_Dn = (Float_t)amplitudes_[CDPos - 1];
              C_Dnn = (Float_t)amplitudes_[CDPos - 2];
            }
            if (amplitudes_.size() - LeftOfMaxPos > 1 && MaxPos + 2 < (int)(amplitudes_.size()) - 1) {
              C_Mnn = (Float_t)amplitudes_[MaxPos + 2];
            } else
              C_Mnn = 0;
          }
          if ((C_Dn <= coeff1 * coeffn * C_D + coeff2 * coeffnn * C_M + 2 * noise || C_D == 255) &&
              C_Mnn <= coeff1 * coeffn * C_Mn + coeff2 * coeffnn * C_M + 2 * noise &&
              C_Dnn <= coeff1 * coeffn * C_Dn + coeff2 * coeffnn * C_D + 2 * noise) {
            shapecdtn = true;
          }
        }
      } else if (exitCode)
        *exitCode = 6;
    }
  } else if (NofMax > 1 && exitCode)
    *exitCode = 1;  // more than one maximum
  if (amplitudes_.size() == 1) {
    shapecdtn = true;
  }
  if (shapecdtn && exitCode)
    *exitCode = 0;

    //std::cout<<"\t\t [+] shapecdtn: "<<shapecdtn<<std::endl; //EMERY//
  return shapecdtn;
}
