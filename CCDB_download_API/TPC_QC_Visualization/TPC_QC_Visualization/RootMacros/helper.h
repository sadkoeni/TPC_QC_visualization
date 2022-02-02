// helper.h
// Helper functions for the CCDB interface

#ifndef TPCQCVIS_HELPER_H
#define TPCQCVIS_HELPER_H

// Root includes
#include "TH2F.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TPad.h"
// O2 includes
#include "CCDB/CcdbApi.h"
#include "TPCQC/CalPadWrapper.h"
#include "TPCQC/Clusters.h"

namespace TPCQCVIS_HELPER
{
  class helper
  {
  public:

    helper() = default;

    struct datafile{
      std::string path;
      long timestamp;
      std::string type;
    };

    // Find files
    std::vector<std::string> splitToVector(std::string str, std::string token);

    std::string getPath(std::string str);

    long getTimeStamp(std::string str);

    std::string getType(std::string str);

  private:
    ClassDefNV(helper, 1)
  };
}

#endif