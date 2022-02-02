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
/// @file   runCCDBlocalAPI.C
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
//#include "QualityControl/TPC/ClustersData.h"
//#include "TPCBase/Painter.h"
//#include "DataFormatsTPC/TrackTPC.h"
//#include "DataFormatsTPC/ClusterNaitve.h"
//#include "TPCQC/Tracks.h"
//#include "TPCQC/Helpers.h"
#endif

using namespace o2;
//using namespace o2::quality_control::core;
using namespace std;

struct datafile{
    string path;
    long timestamp;
    string type;
    string name;
};

// Find files
vector<string> splitToVector(string str, string token){
    vector<string>result;
    while(str.size()){
        int index = str.find(token);
        if(index!=static_cast<int>(string::npos)){
            result.push_back(str.substr(0,index));
            str = str.substr(index+token.size());
            if(str.size()==0)result.push_back(str);
        }else{
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

string getPath(string str){
    string result;
    string token = "Path: ";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    result = str.substr(start,end-start);
    return result;
}

string getName(string str){
    string result;
    string token = "Path: qc/TPC/MO/Tracks/";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    result = str.substr(start,end-start);
    return result;
}


long getTimeStamp(string str){
    string result_str;
    long result;
    string token = "Validity: ";
    int start = str.find(token)+token.size();
    int end = str.find(" -",start);
    result_str = str.substr(start,end-start);
    string::size_type sz;
    result = stol(result_str,&sz);
    return result;
}

string getType(string str){
    string result;
    string token = "ObjectType = ";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    result = str.substr(start,end-start);
    return result;
}
void runCCDBlocalAPI(){
    std::cout << "Checkpoint 1\n";
    ccdb::CcdbApi api;  // init
    map<string, string> metadata; // can be empty
    api.init("http://ccdb-test.cern.ch:8080"); // or http://localhost:8080 for a local CCDB (doesn't work for some reason)
    string path = "qc/TPC/MO/";
    string folder = "Tracks/.*";

    std::cout << "Checkpoint 2\n";
    // Read a list of all files in directory
	string file_list = api.list(path+folder);
	vector<std::string> files_vector = splitToVector(file_list,"\n\n"); //split different files information into vector
	files_vector.pop_back();
    std::ofstream output_file("../../Data/UserFiles/list_of_all_files.txt");
    std::ostream_iterator<std::string> output_iterator(output_file, "\n");
    std::copy(files_vector.begin(), files_vector.end(), output_iterator);
	
	vector<datafile> files;
    std::cout << "Checkpoint 3\n";
    // Fill the structures
    for(const auto& file:files_vector) {
        //std::cout << "New file\n";
    	datafile current_file;
    	current_file.path = getPath(file);
    	current_file.timestamp = getTimeStamp(file);
    	current_file.type = getType(file);
        current_file.name = getName(file);
        files.push_back(current_file);
    }
    std::cout << "Checkpoint 4\n";

    // Create TFile and write objects
    TFile tf("../../Data/UserFiles/visClustersData.root","recreate");
    for (const auto& file : files) {
        if (file.type == "TH1F"){
            auto th1f = api.retrieveFromTFileAny<TH1F>(file.path,metadata,file.timestamp);
            tf.WriteObject(th1f, file.name.c_str());        }
        else if (file.type == "TH2F"){
            auto th2f = api.retrieveFromTFileAny<TH2F>(file.path,metadata,file.timestamp);
            tf.WriteObject(th2f, file.name.c_str());
        }
        else if (file.type == "TCanvas") {
            auto tcanvas = api.retrieveFromTFileAny<TCanvas>(file.path,metadata,file.timestamp);
            tf.WriteObject(tcanvas, file.name.c_str());   
        }
        else if (file.type == "o2::tpc::qc::CalPadWrapper") {
            auto calpad = api.retrieveFromTFileAny<o2::tpc::qc::CalPadWrapper>(file.path,metadata,file.timestamp);
            tf.WriteObject(calpad, file.name.c_str());
        }
        else {
            printf("Object %s has unknown file type %s.\n Skipping.\n", file.path.c_str(), file.type.c_str());
        }    
    }
    tf.Close();
}