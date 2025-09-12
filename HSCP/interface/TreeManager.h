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

// Static variables / functions

//____________________________________________________________________________________________________
std::map<std::string, std::map<std::string, std::any> > TreeManager::s_vars;

// Instance things

//____________________________________________________________________________________________________
TreeManager::TreeManager()
  : m_tree ( nullptr )
{}


//____________________________________________________________________________________________________
TreeManager::~TreeManager() {
  delete m_tree;
}

//____________________________________________________________________________________________________
void TreeManager::initialize(const std::string& treeName,  edm::Service<TFileService> fs ) {
  m_tree = fs->make<TTree>(treeName.c_str(), treeName.c_str());
  //m_tree->SetAutoFlush( 500. );
}

//____________________________________________________________________________________________________
void TreeManager::book( const char* name, std::any& var ) {
    //std::cout << "==> booking '"<<name<<"'"<<std::endl;
    // here, it's needed to book all possible types stored in any type

  if( nullptr == m_tree->GetBranch( name ) ) {
    
#define BRANCH( TYPE )     ( var.type() == typeid( TYPE ) ) {                               \
      auto* holder = std::any_cast<TYPE>(&var);                                             \
      auto* branch = m_tree->Branch( name, holder  );                                       \
      if( nullptr == branch ) {                                                             \
        throw cms::Exception("TreeManager::error::not_possible_to_branch")                  \
              << __PRETTY_FUNCTION__                                                        \
              << ": the type of variable " << name                                          \
              << "[ " << boost::core::demangle( typeid(TYPE).name() ) << " ] "              \
              << "is not possible to register to tree!";                                    \
      }                                                                                     \
      std::any tmp( var );                                                                  \
      var = std::numeric_limits<TYPE>::quiet_NaN();                                         \
      for( auto ientry = 0LL; ientry < m_tree->GetEntries(); ++ientry ) { branch->Fill(); } \
      var = *( std::any_cast<TYPE>( &tmp ) );                                               \
    }
    // end of #define

#define BRANCH_VEC( TYPE )     ( var.type() == typeid( TYPE ) ) {                           \
      auto* holder = std::any_cast<TYPE>(&var);                                             \
      auto* branch = m_tree->Branch( name, holder  );                                       \
      if( nullptr == branch ) {                                                             \
        throw cms::Exception("TreeManager::error::not_possible_to_branch")                  \
              << __PRETTY_FUNCTION__                                                        \
              << ": the type of variable " << name                                          \
              << "[ " << boost::core::demangle( typeid(TYPE).name() )  << " ] "             \
              << "is not possible to register to tree!";                                    \
      }                                                                                     \
      std::any tmp( var );                                                                  \
      var = std::numeric_limits<TYPE>::quiet_NaN();                                         \
      for( auto ientry = 0LL; ientry < m_tree->GetEntries(); ++ientry ) { branch->Fill(); } \
      var = *( std::any_cast<TYPE>( &tmp ) );                                               \
    }
    // end of #define

#define BRANCH_UINT32( TYPE )     ( var.type() == typeid( TYPE ) ) {                        \
      auto* holder = std::any_cast<TYPE>(&var);                                             \
      auto* branch = m_tree->Branch( name, holder, Form("%s/i",name) );                     \
      if( nullptr == branch ) {                                                             \
        throw cms::Exception("TreeManager::error::not_possible_to_branch")                  \
              << __PRETTY_FUNCTION__                                                        \
              << ": the type of variable " << name                                          \
              << "[ " << boost::core::demangle( typeid(TYPE).name() ) << " ] "              \
              << "is not possible to register to tree!";                                    \
      }                                                                                     \
      std::any tmp( var );                                                                  \
      var = std::numeric_limits<TYPE>::quiet_NaN();                                         \
      for( auto ientry = 0LL; ientry < m_tree->GetEntries(); ++ientry ) { branch->Fill(); } \
      var = *( std::any_cast<TYPE>( &tmp ) );                                               \
    }
    // end of #define

#define BRANCH_UINT64( TYPE )     ( var.type() == typeid( TYPE ) ) {                        \
      auto* holder = std::any_cast<TYPE>(&var);                                             \
      auto* branch = m_tree->Branch( name, holder, Form("%s/l",name) );                     \
      if( nullptr == branch ) {                                                             \
        throw cms::Exception("TreeManager::error::not_possible_to_branch")                  \
              << __PRETTY_FUNCTION__                                                        \
              << ": the type of variable " << name                                          \
              << "[ " << boost::core::demangle( typeid(TYPE).name() ) << " ] "              \
              << "is not possible to register to tree!";                                    \
      }                                                                                     \
      std::any tmp( var );                                                                  \
      var = std::numeric_limits<TYPE>::quiet_NaN();                                         \
      for( auto ientry = 0LL; ientry < m_tree->GetEntries(); ++ientry ) { branch->Fill(); } \
      var = *( std::any_cast<TYPE>( &tmp ) );                                               \
    }
    // end of #define

#define BRANCH_STRING( TYPE )     ( var.type() == typeid( TYPE ) ) {                        \
      auto* holder = std::any_cast<TYPE>(&var);                                             \
      auto* branch = m_tree->Branch( name, holder  );                                       \
      if( nullptr == branch ) {                                                             \
        throw cms::Exception("TreeManager::error::not_possible_to_branch")                  \
              << __PRETTY_FUNCTION__                                                        \
              << ": the type of variable " << name                                          \
              << "[ " << boost::core::demangle( typeid(TYPE).name() ) << " ] "              \
              << "is not possible to register to tree!";                                    \
      }                                                                                     \
      TYPE tmp( std::any_cast<TYPE&>(var) );                                                \
      var = TYPE{};                                                                         \
      for( auto ientry = 0LL; ientry < m_tree->GetEntries(); ++ientry ) { branch->Fill(); } \
      var = tmp;                                                                            \
    }
    // end of #define

    if BRANCH( bool )
    else if BRANCH( char )
    else if BRANCH( short )
    else if BRANCH( int )
    else if BRANCH( long )
    else if BRANCH( long long )
    else if BRANCH( unsigned char )
    else if BRANCH( unsigned short )
    else if BRANCH( unsigned int )
    else if BRANCH_UINT64( unsigned long )
    else if BRANCH( unsigned long long )
    else if BRANCH( float )
    else if BRANCH( double )
    else if BRANCH( uint8_t )
    else if BRANCH( int8_t )
    else if BRANCH( uint16_t )
    else if BRANCH( int16_t )
    else if BRANCH_UINT32( uint32_t )
    else if BRANCH( int32_t )
    else if BRANCH( uint64_t )
    else if BRANCH( int64_t )
    else if BRANCH_STRING( std::string )
    else if BRANCH_VEC( std::vector<bool> )
    else if BRANCH_VEC( std::vector<uint8_t> )
    else if BRANCH_VEC( std::vector<int8_t> )
    else if BRANCH_VEC( std::vector<uint16_t> )
    else if BRANCH_VEC( std::vector<int16_t> )
    else if BRANCH_VEC( std::vector<uint32_t> )
    else if BRANCH_VEC( std::vector<int32_t> )
    else if BRANCH_VEC( std::vector<uint64_t> )
    else if BRANCH_VEC( std::vector<int64_t> )
    else if BRANCH_VEC( std::vector<unsigned char> )
    else if BRANCH_VEC( std::vector<unsigned short> )
    else if BRANCH_VEC( std::vector<unsigned long> )
    else if BRANCH_VEC( std::vector<unsigned long long> )
    else if BRANCH_VEC( std::vector<char> )
    else if BRANCH_VEC( std::vector<std::string> )
    else if BRANCH_VEC( std::vector<short> )
    else if BRANCH_VEC( std::vector<long> )
    else if BRANCH_VEC( std::vector<long long> )
    else if BRANCH_VEC( std::vector<float> )
    else if BRANCH_VEC( std::vector<double> )
    else if BRANCH_VEC( std::vector<std::vector<bool>> )
    else if BRANCH_VEC( std::vector<std::vector<uint8_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int8_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint16_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int16_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint32_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int32_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint64_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int64_t>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned char>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned short>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned long>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned long long>> )
    else if BRANCH_VEC( std::vector<std::vector<char>> )
    else if BRANCH_VEC( std::vector<std::vector<short>> )
    else if BRANCH_VEC( std::vector<std::vector<long>> )
    else if BRANCH_VEC( std::vector<std::vector<long long>> )
    else if BRANCH_VEC( std::vector<std::vector<float>> )
    else if BRANCH_VEC( std::vector<std::vector<double>> )
    else {
      throw cms::Exception("TreeManager::error::unsupported_type")
            << __PRETTY_FUNCTION__ 
            << ": the type of variable " 
            << name << " [ " 
            << boost::core::demangle( var.type().name() ) 
            << " ] is not supported!";
    }
      
#undef BRANCH
#undef BRANCH_VEC
    
    
  }
  
}


//____________________________________________________________________________________________________
void TreeManager::clearVectorBranch(const char* name, std::any& var){
#define BRANCH_VEC( TYPE ) ( var.type() == typeid( TYPE ) ) {std::any_cast<TYPE&>(var).clear();}
    if BRANCH_VEC( std::vector<bool> )
    else if BRANCH_VEC( std::vector<uint8_t> )
    else if BRANCH_VEC( std::vector<int8_t> )
    else if BRANCH_VEC( std::vector<uint16_t> )
    else if BRANCH_VEC( std::vector<int16_t> )
    else if BRANCH_VEC( std::vector<uint32_t> )
    else if BRANCH_VEC( std::vector<int32_t> )
    else if BRANCH_VEC( std::vector<uint64_t> )
    else if BRANCH_VEC( std::vector<int64_t> )
    else if BRANCH_VEC( std::vector<unsigned char> )
    else if BRANCH_VEC( std::vector<unsigned short> )
    else if BRANCH_VEC( std::vector<unsigned long> )
    else if BRANCH_VEC( std::vector<unsigned long long> )
    else if BRANCH_VEC( std::vector<char> )
    else if BRANCH_VEC( std::vector<std::string> )
    else if BRANCH_VEC( std::vector<short> )
    else if BRANCH_VEC( std::vector<long> )
    else if BRANCH_VEC( std::vector<long long> )
    else if BRANCH_VEC( std::vector<float> )
    else if BRANCH_VEC( std::vector<double> )
    else if BRANCH_VEC( std::vector<std::vector<bool>> )
    else if BRANCH_VEC( std::vector<std::vector<uint8_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int8_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint16_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int16_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint32_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int32_t>> )
    else if BRANCH_VEC( std::vector<std::vector<uint64_t>> )
    else if BRANCH_VEC( std::vector<std::vector<int64_t>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned char>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned short>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned long>> )
    else if BRANCH_VEC( std::vector<std::vector<unsigned long long>> )
    else if BRANCH_VEC( std::vector<std::vector<char>> )
    else if BRANCH_VEC( std::vector<std::vector<short>> )
    else if BRANCH_VEC( std::vector<std::vector<long>> )
    else if BRANCH_VEC( std::vector<std::vector<long long>> )
    else if BRANCH_VEC( std::vector<std::vector<float>> )
    else if BRANCH_VEC( std::vector<std::vector<double>> )
#undef BRANCH_VEC
}

//____________________________________________________________________________________________________
void TreeManager::fill() { 
#if 0
  auto* branches = m_tree->GetListOfBranches();
  for( int i=0; i< branches->GetEntries(); i++) {
    auto* branch = dynamic_cast<TBranch*>( branches->At(i) );
    auto* address = branch->GetAddress();
    std::cout << branch->GetName() << ": address = " << static_cast<void*>(address) << std::endl;
  }
  m_tree->Print();
#endif
  m_tree->Fill();
}

//____________________________________________________________________________________________________
Long64_t TreeManager::getEntries() {
  return m_tree->GetEntries();
}