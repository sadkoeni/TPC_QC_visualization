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
//using namespace o2::quality_control::core;
//using namespace std;

std::vector<std::string> splitString(std::string inString, const char* delimiter)
{
  std::vector<std::string> outVec;
  std::string placeholder;
  std::istringstream stream(inString);
  std::string token;
  while (std::getline(stream, token, *delimiter)) {
    if(token == "") {
        if(!placeholder.empty()){
            outVec.emplace_back(placeholder);
            placeholder.clear();
        }
    }
    else {
        placeholder.append(token+"\n");
    }
  }
  return std::move(outVec);
}

std::string getPath(std::string str){
    std::string result;
    std::string token = "Path: ";
    std::string token2 = "/";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    int slash = str.find_last_of("/",end)+token2.size();
    result = str.substr(start,slash-start);
    return result;
}

long getTimeStamp(std::string str){
    std::string result_str;
    long result;
    std::string token = "Validity: ";
    int start = str.find(token)+token.size();
    int end = str.find(" -",start);
    result_str = str.substr(start,end-start);
    std::string::size_type sz;
    result = stol(result_str,&sz);
    return result;
}

std::string getName(std::string str){
    std::string result;
    std::string token = "Path: ";
    std::string token2 = "/";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    int slash = str.find_last_of("/",end)+token2.size();
    result = str.substr(slash,end-slash);
    return result;
}

std::string getType(std::string str){
    std::string result;
    std::string token = "ObjectType = ";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    result = str.substr(start,end-start);
    return result;
}

std::string getTask(std::string str){
    std::string result;
    std::string token = "qc_task_name = ";
    int start = str.find(token)+token.size();
    int end = str.find("\n",start);
    result = str.substr(start,end-start);
    return result;
}

int getSize(std::string str){
    std::string result;
    std::string token = "size: ";
    int start = str.find(token)+token.size();
    int end = str.find(",",start);
    result = str.substr(start,end-start);
    std::string::size_type sz;
    return stoi(result,&sz);
}

bool fileComparitor (std::string i,std::string j) { return (getPath(i)<getPath(j)); }

void runCCDBItemList(){

    // Initialize CCDB API
    ccdb::CcdbApi api;
    map<std::string, std::string> metadata;
    api.init("http://ccdb-test.cern.ch:8080");

    // Choose which directory to list
    std::string path = "qc/TPC/MO/";
    std::string folder = ".*";
;
    // Read a list of all files in directory
    std::cout << "Getting list" << std::endl;
    std::string file_list = api.list(path+folder);

    std::cout << "Vectorizing" << std::endl;
    vector<std::string> files_vector = splitString(file_list,"\n"); //split different files information into vector
    files_vector.pop_back();

    std::cout << "Sorting" << std::endl;
    std::sort(files_vector.begin(),files_vector.end(),fileComparitor);

    std::cout << "Writing to file" << std::endl;
    // Extract relevant information and save to csv file
    FILE *output_file;
    output_file = fopen("../../Data/UserFiles/CCDB.csv", "w+");
    int file_count = 0, file_size;
    long file_timestamp;
    std::string file_path, file_name, file_type, file_task;
    std::string current_file;

    fprintf(output_file,"ID, Path, Name, TimeStamp, Type, Task, Size\n");
    for(const auto& file:files_vector) {
        file_count++;
        file_path = getPath(file);
        file_name = getName(file);
        file_timestamp = getTimeStamp(file);
        file_type = getType(file);
        file_task = getTask(file);
        file_size = getSize(file);
        // Write info to CSV
        fprintf(output_file,"%d, %s, %s, %ld, %s, %s, %d\n", file_count, file_path.c_str(), file_name.c_str(), file_timestamp, file_type.c_str(), file_task.c_str(), file_size);
    }
    fclose(output_file);
}