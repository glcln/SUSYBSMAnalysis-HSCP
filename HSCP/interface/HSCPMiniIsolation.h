#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Math/interface/deltaR.h"


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

//HSCPMiniIsolation::HSCPMiniIsolation(const reco::TrackRef track, std::vector<reco::PFCandidate> pfCands, reco::Vertex  vertex)
HSCPMiniIsolation::HSCPMiniIsolation(const reco::TrackRef track, const reco::PFCandidateCollection& pfCands, reco::Vertex  vertex)
{
    track_   = track;
    pfCands_ = &pfCands;
    vertex_  = vertex;
}

HSCPMiniIsolation::~HSCPMiniIsolation() {}

void HSCPMiniIsolation::computeMiniIsolation(float dZ_cut)
{
    //bool pf_isChHadron(false), pf_isNeutHadron(false), pf_isElectron(false), pf_isMuon(false), pf_isPhoton(false);
    //bool pf_isUndefined(false);

    // https://github.com/cms-sw/cmssw/blob/72d0fc00976da53d1fb745eb7f37b2a4ad965d7e/
    // PhysicsTools/PatAlgos/plugins/PATIsolatedTrackProducer.cc#L555

    track_isPF = false;
    for (auto pfCand = pfCands_->begin(); pfCand != pfCands_->end(); pfCand++)
    {
        //const reco::PFCandidate* pfCand = &(*pfCands_)[i];

        bool pf_isElectronForIdx = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::e;
        bool pf_isMuonForIdx = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::mu;
        bool pf_isPhotonForIdx = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::gamma;
        bool pf_isChHadronForIdx = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::h;
        bool pf_isNeutHadronForIdx = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::h0;

        if (pfCand->trackRef().isNonnull() && pfCand->trackRef().key() == track_.key())
        {
            track_isPF = true;

            // bool pf_isElectron = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::e;
            // bool pf_isMuon = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::mu;
            // bool pf_isPhoton = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::gamma;

            // bool pf_isChHadron = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::h;
            // bool pf_isNeutHadron = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::h0;
            // bool pf_isUndefined = pfCand->translatePdgIdToType(pfCand->pdgId()) == reco::PFCandidate::ParticleType::X;

            pf_energy = pfCand->ecalEnergy() + pfCand->hcalEnergy();
            pf_ecal_energy = pfCand->ecalEnergy();
            pf_hcal_energy = pfCand->hcalEnergy();

            float dr = reco::deltaR(pfCand->eta(),pfCand->phi(),track_->eta(),track_->phi());

            bool fromPV = (fabs(track_->dz(vertex_.position())) < dZ_cut);

            float pt = pfCand->p4().pt();
            float drForMiniIso = 0.0;
            if      (track_->pt() < 50 ) drForMiniIso = 0.2;
            else if (track_->pt() < 200) drForMiniIso = 10/track_->pt();
            else                         drForMiniIso = 0.05;

            if (dr<drForMiniIso)
            {
                // Leptons get added to trackIso (this is not in the official definition)
                if (pf_isElectronForIdx || pf_isMuonForIdx) track_PFMiniIso_sumLeptonPt+=pt;
                // charged cands from PV get added to trackIso
                if(pf_isChHadronForIdx && fromPV) track_PFMiniIso_sumCharHadPt+=pt;
                // charged cands not from PV get added to pileup iso
                else if(pf_isChHadronForIdx) track_PFMiniIso_sumPUPt+=pt;
                // neutral hadron iso
                if(pf_isNeutHadronForIdx) track_PFMiniIso_sumNeutHadPt+=pt;
                // photon iso
                if(pf_isPhotonForIdx) track_PFMiniIso_sumPhotonPt+=pt;
                // muon iso
                if(pf_isMuonForIdx) track_PFMiniIso_sumMuonPt+=pt;
                if (!pf_isElectronForIdx && !pf_isMuonForIdx && !pf_isChHadronForIdx && !pf_isNeutHadronForIdx && !pf_isPhotonForIdx) {
                    track_PFMiniIso_otherPt+=pt;
                }
            }

            if (dr<0.05)
            {
                // // charged cands from PV get added to trackIso
                // if(pf_isChHadronForIdx && fromPV) track_PFIso005_sumCharHadPt+=pt;
                // // charged cands not from PV get added to pileup iso
                // else if(pf_isChHadronForIdx) track_PFIso005_sumPUPt+=pt;
                // // neutral hadron iso
                // if(pf_isNeutHadronForIdx) track_PFIso005_sumNeutHadPt+=pt;
                // // photon iso
                // if(pf_isPhotonForIdx) track_PFIso005_sumPhotonPt+=pt;
            }

            if(dr<0.1)
            {}

            if(dr<0.3)
            {}

            if(dr<0.5)
            {}
        }
    }// end loop PFCandidates

    // Calculate PF mini relative isolation
    // float miniRelIsoOfficial = (track_PFMiniIso_sumCharHadPt + std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5* track_PFMiniIso_sumPUPt))/track->pt();
    miniRelIsoAll = (track_PFMiniIso_sumLeptonPt + track_PFMiniIso_otherPt + track_PFMiniIso_sumCharHadPt + std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5* track_PFMiniIso_sumPUPt))/track_->pt();
    miniRelIsoChg = track_PFMiniIso_sumCharHadPt/track_->pt();
    miniRelIsoAll_wMuon = (track_PFMiniIso_sumMuonPt + track_PFMiniIso_sumCharHadPt + std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5* track_PFMiniIso_sumPUPt))/track_->pt();
    
}