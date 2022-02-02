// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "CPOYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

///
/// @file   runCCDBItemList.C
/// @author Berkin Ulukutlu, berkin.ulukutlu@cern.ch
///

#if !defined(__CLING__) || defined(__ROOTCLING__)
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
// TPCQCVis includes
#include "helper.h"
//#include "QualityControl/TPC/ClustersData.h"
//#include "TPCBase/Painter.h"
//#include "DataFormatsTPC/TrackTPC.h"
//#include "DataFormatsTPC/ClusterNaitve.h"
//#include "TPCQC/Tracks.h"
//#include "TPCQC/Helpers.h"
#endif

using namespace o2;

bool fileComparitor (std::string i,std::string j) { 
    TPCQCVIS_HELPER::helper helper;
    return (helper.getPath(i)<helper.getPath(j));
}

vector<std::string> getCCDBItemList(){

    TPCQCVIS_HELPER::helper helper;
    ccdb::CcdbApi api;  // init

    map<std::string, std::string> metadata; // can be empty
    api.init("http://ccdb-test.cern.ch:8080");

    std::string path = "qc/TPC/MO/";
    std::string folder = "PID/.*";

    std::string file_list = api.list(path+folder);
    vector<std::string> files_vector = helper.splitToVector(file_list,"\n\n"); //split different files information into vector
    files_vector.pop_back();

    std::sort(files_vector.begin(),files_vector.end(),fileComparitor);
    //std::cout << "Checkpoint 3\n";


    vector<std::string> directoryTree;
    int file_count = 0;
    std::string current_file;
    for(const auto& file:files_vector) {
        if (file_count == 0) {
            current_file = helper.getPath(file);
            directoryTree.push_back(current_file);
        }
        if (current_file == helper.getPath(file)) {
            directoryTree.push_back("-->"+to_string(helper.getTimeStamp(file)));
        }
        else {
            current_file = helper.getPath(file);
            directoryTree.push_back(current_file);
            directoryTree.push_back("-->"+to_string(helper.getTimeStamp(file)));
        }
        file_count++;
    }
    //std::cout << "Checkpoint 4\n";
    std::ofstream output_file("./directoryTree.txt");
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(directoryTree.begin(), directoryTree.end(), output_iterator);
    return(directoryTree);
}