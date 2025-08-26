#include <memory>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/Common/interface/View.h"

#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"
#include "SUSYBSMAnalysis/HSCP/interface/CandidateSelector.h"

class HSCParticleSelector : public edm::stream::EDFilter<> {
public:
    explicit HSCParticleSelector(const edm::ParameterSet&);
    ~HSCParticleSelector() override = default;

private:
    bool filter(edm::Event&, const edm::EventSetup&) override;

    edm::EDGetTokenT<susybsm::HSCParticleCollection> sourceToken_;
    bool Filter_;
    std::vector<CandidateSelector*> Selectors;
};

HSCParticleSelector::HSCParticleSelector(const edm::ParameterSet& iConfig) {
    produces<susybsm::HSCParticleCollection>();

    sourceToken_ = consumes<susybsm::HSCParticleCollection>(
        iConfig.getParameter<edm::InputTag>("source"));
    Filter_ = iConfig.getParameter<bool>("filter");

    auto SelectionParameters = iConfig.getParameter<std::vector<edm::ParameterSet>>("SelectionParameters");
    for (auto& pset : SelectionParameters) {
        Selectors.push_back(new CandidateSelector(pset));
    }
}

bool HSCParticleSelector::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    edm::Handle<susybsm::HSCParticleCollection> SourceHandle;
    if (!iEvent.getByToken(sourceToken_, SourceHandle)) {
        edm::LogError("") << ">>> HSCParticleCollection does not exist !!!";
        return false;
    }
    auto Source = *SourceHandle.product();

    auto output = std::make_unique<susybsm::HSCParticleCollection>();

    for (auto& hscpcandidate : Source) {
        bool decision = false;
        for (auto* sel : Selectors) decision |= sel->isSelected(hscpcandidate);
        if (decision) output->push_back(hscpcandidate);
    }

    bool filterResult = !Filter_ || (Filter_ && !output->empty());
    iEvent.put(std::move(output));

    return filterResult;
}

DEFINE_FWK_MODULE(HSCParticleSelector);
