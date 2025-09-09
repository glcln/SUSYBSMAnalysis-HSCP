#include "SUSYBSMAnalysis/HSCP/interface/HSCPVertexSelector.h"

// ------------ HSCPVertexSelector  ------------

HSCPVertexSelector::HSCPVertexSelector(std::vector<reco::Vertex>  vertices) {
  vertices_ = vertices;

  process();
}

HSCPVertexSelector::~HSCPVertexSelector() {}

//
// member functions
//

void HSCPVertexSelector::process(){
  goodVertices_.clear();
  reco::Vertex vertex;

  int nvtx = 0;
  for (const auto &vtx: vertices_){

    if(!isGoodVertex(vtx)) continue;

    goodVertices_.push_back(vtx);

    if( nvtx==0 ) vertex = vtx;

    // Look for the best (sum pT squared) vertex, i.e hard-scattering vertex
    double sum = 0.;
    double pT;

    int ntrk = 0;
    for(auto it=vtx.tracks_begin(); it!=vtx.tracks_end(); it++){
      pT = (**it).pt();
      double epT=(**it).ptError();
      pT=pT>epT ? pT-epT : 0;

      sum += pT*pT;

      edm::LogVerbatim("")//edm::LogDebug("PrimaryVertices") 
      << Form("\t\t itrk = %d,\t pT = %.1f,\t epT = %.1f,\t epT - pT = %.1f,\t eta = %.2f,\t phi = %.2f \n",
      ntrk, (**it).pt(), (**it).ptError(), (**it).pt()-(**it).ptError(), (**it).eta(), (**it).phi());

      ntrk++;
    }

    nvtx++;  
  }

  bestGoodVertex_ = vertex;
}


bool HSCPVertexSelector::isGoodVertex(const reco::Vertex vtx){
  if (!vtx.isValid())   return false;
  if (vtx.isFake())     return false;
  //if (vtx.ndof() < 4)   return false;
  //if (fabs(vtx.position().Z()) > 24.)  return false;
  //if (fabs(vtx.position().Rho()) > 2.) return false;

	return true;
}

reco::Vertex HSCPVertexSelector::getBestVertex(){
  return bestGoodVertex_;
}

std::vector<reco::Vertex> HSCPVertexSelector::getGoodVertices(){
  return goodVertices_;
}