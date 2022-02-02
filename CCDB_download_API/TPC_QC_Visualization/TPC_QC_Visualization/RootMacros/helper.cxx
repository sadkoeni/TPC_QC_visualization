// helper.cxx
// Helper functions for the CCDB interface

#include "helper.h"

using namespace TPCQCVIS_HELPER;

std::vector<string> helper::splitToVector(string str, string token){
  std::vector<string>result;
  while(str.size()){
    int index = str.find(token);
    if(index!=static_cast<int>(string::npos)){
        result.push_back(str.substr(0,index));
        str = str.substr(index+token.size());
        if(str.size()==0) result.push_back(str);
    }
    else{
        result.push_back(str);
        str = "";
    }
  }
  return result;
}

string helper::getPath(string str){
  string result;
  string token = "Path: ";
  int start = str.find(token)+token.size();
  int end = str.find("\n",start);
  result = str.substr(start,end-start);
  return result;
}

long helper::getTimeStamp(string str){
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

string helper::getType(string str){
  string result;
  string token = "ObjectType = ";
  int start = str.find(token)+token.size();
  int end = str.find("\n",start);
  result = str.substr(start,end-start);
  return result;
}