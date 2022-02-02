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
#include "helper.h"
#endif

using namespace o2;
//using namespace o2::quality_control::core;
using namespace std;

void testing_macro(){
    TPCQCVIS_HELPER::helper helper;
    //std::cout << "Checkpoint 1\n";
    ccdb::CcdbApi api;  // init
    map<string, string> metadata; // can be empty
    api.init("http://ccdb-test.cern.ch:8080");
    //api.init("http://ali-qcdb.cern.ch:8083"); //For the real QCDB access (doesn't work. Need to investigate Proxy Seetings)
    string path = "qc/TPC/MO/";
    string folder = "PID/.*";


    //std::cout << "Checkpoint 2\n";
    // Read a list of all files in directory
    string file_list = api.list(path+folder);
    vector<std::string> files_vector = helper::splitToVector(file_list,"\n\n"); //split different files information into vector
    files_vector.pop_back();
    std::sort(files_vector.begin(),files_vector.end(),helper::fileComparitor);
    //std::cout << "Checkpoint 3\n";


    vector<std::string> directoryTree;
    int file_count = 0;
    std::string current_file;
    for(const auto& file:files_vector) {
        if (file_count == 0) {
            current_file = helper::getPath(file);
            directoryTree.push_back(current_file);
        }
        if (current_file == helper:getPath(file)) {
            directoryTree.push_back("-->"+to_string(helper::getTimeStamp(file)));
        }
        else {
            current_file = helper::getPath(file);
            directoryTree.push_back(current_file);
            directoryTree.push_back("-->"+to_string(helper::getTimeStamp(file)));
        }
        file_count++;
    }
    //std::cout << "Checkpoint 4\n";
    std::ofstream output_file("./directoryTree.txt");
    std::ostream_iterator<std::string> helper:output_iterator(output_file, "\n");
    std::copy(directoryTree.begin(), directoryTree.end(), output_iterator);
}