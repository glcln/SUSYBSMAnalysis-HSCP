#ifndef HSCPDEDXTOOL_H
#define HSCPDEDXTOOL_H

// -*- C++ -*-
// Class:      HSCPDeDxTool
// Original Author:  Emery Nibigira
//         Created:  Mon, 22 Jan 2024 17:47:22 GMT

// system include files
#include <memory>
#include <vector>

#include "TMatrix.h"
#include "TLorentzVector.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/VertexReco/interface/Vertex.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "RecoLocalTracker/ClusterParameterEstimator/interface/PixelClusterParameterEstimator.h"

#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

//#include "SaturationCorrection.h"  // New procedure for the correction of the saturation phenomena
//SaturationCorrection sc;

//
// class declaration
//

class HSCPDeDxTool {
public:
  explicit HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const pat::PackedCandidateRef track, const TrackerTopology* tkTopo = nullptr, const TrackerGeometry* tkGeometry = nullptr);
  explicit HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const reco::TrackRef track, const TrackerTopology* tkTopo = nullptr, const TrackerGeometry* tkGeometry = nullptr);
  virtual ~HSCPDeDxTool();

  //void process();

  float harmonicMean(std::vector<float> v, float expo = -2);
  float truncatedMean(std::vector<float> v, float rate = 0.40);
  float discriminatorIas(std::vector<float> v, bool symmetricSmirnov = false);
  unsigned int numberOfSaturatedClusters();
  std::vector<int> Convert(const std::vector<unsigned char>& input);
  std::vector<uint16_t> CrossTalkInv(const std::vector<uint16_t>& Q,
                                const float x1 = 0.10,
                                const float x2 = 0.04,
                                bool way = true,
                                float threshold = 20,
                                float thresholdSat = 25,
                                bool isClusterCleaning = false);
  std::vector<uint16_t> SaturationCorrection(const std::vector<uint16_t>&  Q, 
                                        const float x1, 
                                        const float x2, 
                                        bool way,
                                        float threshold,
                                        float thresholdSat);
  bool isHitInsideTkModule(const LocalPoint hitPos, const DetId& detid, const SiStripCluster* cluster = nullptr);

  void computedEdx(float* scaleFactors, 
                   bool useClusterCleaning = true,
                   bool mustBeInside = false, 
                   int crossTalkInvAlgo = 0, 
                   float dropLowerDeDxValue = 0.0,
                   TH3* templateHisto = nullptr,
                   bool addExtraDeDxEstimators = false);

  // Harmonic Mean
  reco::DeDxData dedxDataFullTracker() {return dedxDataFullTracker_;}
  reco::DeDxData dedxDataFullTrackerNoL1() {return dedxDataFullTrackerNoL1_;}
  reco::DeDxData dedxDataPixelOnly() {return dedxDataPixel_;}
  reco::DeDxData dedxDataPixelOnlyNoL1() {return dedxDataPixelNoL1_;}
  reco::DeDxData dedxDataStripOnly() {return dedxDataStrip_;}

  // Truncated Mean
  reco::DeDxData dedxTruncFullTracker() {return dedxTruncFullTracker_;}
  reco::DeDxData dedxTruncStripOnly() {return dedxTruncStrip_;}

  reco::DeDxData dedxIasFullTracker() {return dedxIasFullTracker_;}
  reco::DeDxData dedxIasFullTrackerNoL1() {return dedxIasFullTrackerNoL1_;}
  reco::DeDxData dedxIasPixelOnly() {return dedxIasPixel_;}
  reco::DeDxData dedxIasPixelOnlyNoL1() {return dedxIasPixelNoL1_;}
  reco::DeDxData dedxIasStripOnly() {return dedxIasStrip_;}

  //Extra Estimators
  std::vector<reco::DeDxData> dedxDataStripExtra() {return dedxDataStripExtra_;}
  std::vector<reco::DeDxData> dedxTruncStripExtra() {return dedxTruncStripExtra_;}

  std::vector<float> clusterCharge() {return clusterCharge_;}
  std::vector<uint32_t> clusterDetId() {return clusterDetId_;}
  std::vector<uint32_t> clusterSaturatingStrips() {return clusterSaturatingStrips_;}
  std::vector<float> clusterDeDxStrip() {return clusterDeDxStrip_;}

  void computeProbQ(const PixelClusterParameterEstimator* pixelCPE);
  float combineProbs(float probOnTrackWMulti, int numRecHits);

  float probQonTrack() {return probQonTrack_;}
  float probXYonTrack() {return probXYonTrack_;}
  float probQonTrackNoL1() {return probQonTrackNoL1_;}
  float probXYonTrackNoL1() {return probXYonTrackNoL1_;}

  uint32_t numberOfPixelNoL1Measurement() {return nonL1PixHits_;}


private:
  unsigned long long eventNumber_    =  0ULL;
  uint32_t runNumber_                =  {0};
  const reco::DeDxHitInfo* dedxHits_ = nullptr;
  const TrackerTopology* tkTopo_     = nullptr;
  const TrackerGeometry* tkGeometry_ = nullptr;
  float track_eta        =  0;
  float track_pt         =  0;
  float track_px         =  0;
  float track_py         =  0;
  float track_pz         =  0;
  int track_charge       =  0;

  TH3* templateHisto_    = nullptr;

  reco::DeDxData  dedxDataFullTracker_        = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataFullTrackerNoL1_    = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataPixel_              = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataPixelNoL1_          = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataStrip_              = reco::DeDxData(-1, -1, -1);
  //
  reco::DeDxData dedxTruncFullTracker_       = reco::DeDxData(-1, -1, -1);
  reco::DeDxData dedxTruncStrip_             = reco::DeDxData(-1, -1, -1);
  //
  reco::DeDxData  dedxIasFullTracker_        = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasFullTrackerNoL1_    = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasPixel_              = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasPixelNoL1_          = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasStrip_              = reco::DeDxData(-1, -1, -1);
  //bool isStrip_;
  //bool isPixel_;
  //bool useClusterCleaning_;
  //bool mustBeInside_;
  //int crossTalkInvAlgo_;
  //float dropLowerDeDxValue_;

  //Extra Estimators
  std::vector<reco::DeDxData> dedxDataStripExtra_ = {reco::DeDxData(-1, -1, -1)};
  std::vector<reco::DeDxData> dedxTruncStripExtra_= {reco::DeDxData(-1, -1, -1)};

  //saveDeDxHitInfo
  std::vector<float> clusterCharge_;
  std::vector<uint32_t> clusterDetId_;
  std::vector<uint32_t> clusterSaturatingStrips_;
  std::vector<float> clusterDeDxStrip_;

  float probQonTrack_ = -1.f;
  float probXYonTrack_ = -1.f;
  float probQonTrackNoL1_ = -1.f;
  float probXYonTrackNoL1_ = -1.f;

  int nonL1PixHits_ = 0;

  float factorChargeToE_[2] = {3.61e-06, 3.61e-06 * 265};

  const float TkModGeomThickness[15] = {1,
                                     0.029000,
                                     0.029000,
                                     0.047000,
                                     0.047000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.047000,
                                     0.047000,
                                     0.047000};
  const float TkModGeomLength[15] = {1,
                                  5.844250,
                                  5.844250,
                                  9.306700,
                                  9.306700,
                                  5.542900,
                                  4.408000,
                                  5.533000,
                                  4.258000,
                                  4.408000,
                                  5.533000,
                                  5.758000,
                                  7.363125,
                                  9.204400,
                                  10.235775};
  const float TkModGeomWidthB[15] = {1,
                                  3.072000,
                                  3.072000,
                                  4.684800,
                                  4.684800,
                                  4.579445,
                                  5.502407,
                                  3.158509,
                                  4.286362,
                                  5.502407,
                                  4.049915,
                                  3.561019,
                                  6.002559,
                                  5.235483,
                                  3.574395};
  const float TkModGeomWidthT[15] = {1,
                                  3.072000,
                                  3.072000,
                                  4.684800,
                                  4.684800,
                                  3.095721,
                                  4.322593,
                                  4.049915,
                                  3.146580,
                                  4.322593,
                                  3.158509,
                                  2.898798,
                                  4.824683,
                                  4.177638,
                                  4.398049};
};

TH3F* loadDeDxTemplate(std::string path, bool splitByModuleType,bool puTreatment,int puBin);
int factorial(int n); //factorial function used to compute probQ

#endif
