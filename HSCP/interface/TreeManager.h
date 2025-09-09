#ifndef TREEMANAGER_H
#define TREEMANAGER_H

/**  
   TTree manager
   Author: Emery Nibigira ( emery.nibigira@cern.ch )
   Created: 2024-Jan-11
 */

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "TTree.h"

#include <iostream>
#include <string>
#include <limits>
#include <map>
#include <unordered_map>
#include <any>
#include <typeinfo>
#include <boost/core/demangle.hpp>

class TFile;

// TTree manager
class TreeManager {
 protected:
  
  TTree* m_tree;
  
  static std::map<std::string, std::map<std::string, std::any> > s_vars;
  
 public:

  TreeManager();
  virtual ~TreeManager();
  
  //void initialize(const std::string&,  TFile* );
  void initialize(const std::string& treeName,  edm::Service<TFileService> fs );

  // template<class T>
  // void book( const char* name, T* var );
  
  virtual void book( const char* name, std::any& var );

  void clearVectorBranch(const char* name, std::any& var);

  void fill();

  Long64_t getEntries();
  
};


template<typename T>
void addToVectorBranch( std::map<std::string, std::any>& vars, std::string branchName, const T& value ){
  if( vars.find( branchName ) == vars.end() ){
    vars[branchName] = std::vector<T>{};
  }
  auto& var = vars[branchName.c_str()];
  // type check
  if( var.type() != typeid( std::vector<T> ) ) {
    throw cms::Exception("TreeManager::error::invalid_type") 
          << __PRETTY_FUNCTION__ 
          << ": the specified variable " 
          << branchName << " is not a type of " 
          << boost::core::demangle( typeid( std::vector<T> ).name() );
  }
  std::any_cast<std::vector<T>&>(var).emplace_back( value );
}


#endif /* TREEMANAGER_H */