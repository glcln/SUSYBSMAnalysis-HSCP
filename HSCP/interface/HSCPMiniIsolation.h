#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Math/interface/deltaR.h"

class HSCPMiniIsolation {
public:
    explicit HSCPMiniIsolation(const pat::PackedCandidateRef track,
                  const pat::PackedCandidateCollection& pfCands,
                  const pat::PackedCandidateCollection& lostTracks,
                  reco::Vertex vertex);
    virtual ~HSCPMiniIsolation();

    void computeMiniIsolation(float dZ_cut=0.1);
    
    bool isPF() {return track_isPF;}
    float getMiniRelIsoChg() {return miniRelIsoChg;}
    float getMiniRelIsoAll() {return miniRelIsoAll;}

    float getTrackIso_dr03() { return track_genTrackIsoSumPt_dr03; }
    void computeTrackIso_dr03();

private:
    pat::PackedCandidateRef track_;
    const pat::PackedCandidateCollection* pfCands_ = nullptr;
    const pat::PackedCandidateCollection* lostTracks_ = nullptr;
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

    float track_genTrackIsoSumPt_dr03 = 0;
};

HSCPMiniIsolation::HSCPMiniIsolation(const pat::PackedCandidateRef track,
                  const pat::PackedCandidateCollection& pfCands,
                  const pat::PackedCandidateCollection& lostTracks,
                  reco::Vertex vertex)
    : track_(track), pfCands_(&pfCands), lostTracks_(&lostTracks), vertex_(vertex) {}

HSCPMiniIsolation::~HSCPMiniIsolation() {}

void HSCPMiniIsolation::computeMiniIsolation(float dZ_cut)
{
    track_isPF = false;

    for (auto pfCand = pfCands_->begin(); pfCand != pfCands_->end(); ++pfCand)
    {
        // Matching track <-> PackedCandidate
        if (pfCand->hasTrackDetails() && 
            pfCand->charge() != 0 && 
            reco::deltaR(pfCand->eta(), pfCand->phi(), track_->eta(), track_->phi()) < 1e-5)
        {
            track_isPF = true;

            float dr = reco::deltaR(pfCand->eta(), pfCand->phi(), track_->eta(), track_->phi());
            bool fromPV = (fabs(track_->dz(vertex_.position())) < dZ_cut);
            float pt = pfCand->pt();

            float drForMiniIso = 0.0;
            if      (track_->pt() < 50 ) drForMiniIso = 0.2;
            else if (track_->pt() < 200) drForMiniIso = 10/track_->pt();
            else                         drForMiniIso = 0.05;

            if (dr < drForMiniIso)
            {
                int id = abs(pfCand->pdgId());
                bool isElectron   = (id == 11);
                bool isMuon       = (id == 13);
                bool isPhoton     = (id == 22);
                bool isChHadron   = (id == 211);
                bool isNeutHadron = (id == 130);

                if (isElectron || isMuon) track_PFMiniIso_sumLeptonPt += pt;
                if (isChHadron && fromPV) track_PFMiniIso_sumCharHadPt += pt;
                else if (isChHadron)      track_PFMiniIso_sumPUPt += pt;
                if (isNeutHadron)         track_PFMiniIso_sumNeutHadPt += pt;
                if (isPhoton)             track_PFMiniIso_sumPhotonPt  += pt;
                if (isMuon)               track_PFMiniIso_sumMuonPt    += pt;

                if (!isElectron && !isMuon && !isChHadron && !isNeutHadron && !isPhoton)
                    track_PFMiniIso_otherPt += pt;
            }
        }
    } // end loop

    // Iso definitions
    miniRelIsoAll = (track_PFMiniIso_sumLeptonPt + track_PFMiniIso_otherPt + track_PFMiniIso_sumCharHadPt +
                    std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5 * track_PFMiniIso_sumPUPt)) / track_->pt();

    miniRelIsoChg = track_PFMiniIso_sumCharHadPt / track_->pt();

    miniRelIsoAll_wMuon = (track_PFMiniIso_sumMuonPt + track_PFMiniIso_sumCharHadPt +
                          std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5 * track_PFMiniIso_sumPUPt)) / track_->pt();
}



void HSCPMiniIsolation::computeTrackIso_dr03()
{
    track_genTrackIsoSumPt_dr03 = 0.0;

    auto accumulateIsoFromColl = [&](const pat::PackedCandidateCollection* cands) {
        if (!cands) return;
        for (auto const& cand : *cands) {
            if (!cand.hasTrackDetails()) continue;       // must have one track
            if (cand.charge() == 0) continue;            // only charged tracks
            reco::Track const& pseudoTk = cand.pseudoTrack();

            // Do not take into account the HSCP candidate itself
            if (track_.isNonnull() && pseudoTk.extra().isNonnull() && pseudoTk.extra().key() == track_.key())
                continue;

            float pt = cand.pt();
            float dr = reco::deltaR(cand.eta(), cand.phi(), track_->eta(), track_->phi());

            if (dr < 0.3) {
                track_genTrackIsoSumPt_dr03 += pt;
            }
        }
    };

    // Add contribution from packedPFCands and lostTracks
    accumulateIsoFromColl(pfCands_);
    accumulateIsoFromColl(lostTracks_);
}