#ifndef HSCPMINIISOLATION_H
#define HSCPMINIISOLATION_H

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Math/interface/deltaR.h"

// -*- C++ -*-
// Class:      PFMiniIsolation
// Original Author:  Emery Nibigira

class HSCPMiniIsolation {
public:
    //explicit HSCPMiniIsolation(const reco::TrackRef track, std::vector<reco::PFCandidate> pfCands, reco::Vertex  vertex);
    explicit HSCPMiniIsolation(const reco::TrackRef track, const reco::PFCandidateCollection& pfCands, reco::Vertex  vertex);
    virtual ~HSCPMiniIsolation();

    void computeMiniIsolation(float dZ_cut=0.1);
    
    bool isPF() {return track_isPF;}
    float getMiniRelIsoChg() {return miniRelIsoChg;}
    float getMiniRelIsoAll() {return miniRelIsoAll;}
    float getPFEnergy() {return pf_energy;}
    float getPFEcalEnergy() {return pf_ecal_energy;}
    float getPFHcalEnergy() {return pf_hcal_energy;}


private:
    reco::TrackRef track_;
    //std::vector<reco::PFCandidate> pfCands_;
    const reco::PFCandidateCollection* pfCands_ = nullptr;
    reco::Vertex  vertex_;

    bool track_isPF;

    float miniRelIsoChg = 0; 
    float miniRelIsoAll = 0;
    float miniRelIsoAll_wMuon = 0;

    float track_PFMiniIso_sumCharHadPt = 0;
    float track_PFMiniIso_sumNeutHadPt = 0;
    float track_PFMiniIso_sumLeptonPt  = 0;
    float track_PFMiniIso_sumPhotonPt  = 0;
    float track_PFMiniIso_sumPUPt      = 0;
    float track_PFMiniIso_otherPt      = 0;
    float track_PFMiniIso_sumMuonPt    = 0;
    float pf_energy = 0;
    float pf_ecal_energy = 0;
    float pf_hcal_energy = 0;
};

#endif