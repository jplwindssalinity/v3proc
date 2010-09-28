#include <stdlib.h>
#include "Options.h"
using namespace std;
using std::list;
void
Options::parseFile(const char* fileName){
  ifstream fin;
  fin.open(fileName,ios::in);
  if(!fin.is_open()){
    throw Exception(string("File open error"),
		    string("Options::parseFile"),
		    string("Cannot open file: ")+string(fileName));
  }

  // Read lines from the open file

  string line;
  string nextLine;
  do {
    getline(fin,line);
    if(fin.eof())break;
    if(line.size() < 1) continue;

    // Get all the continuation lines

    size_t i = line.find(continuation);
    while(i != string::npos){
      line = this->stripContinuation(line);
      getline(fin,nextLine);
      if(fin.eof())break;
      line += nextLine;
      i = line.find(continuation);
    }

    // Parse the line

    this->parseLine(line);

  } while(!fin.eof());
}


void 
Options::add(const std::string& _key, const std::string& _value,
	   const std::string& _units)
 { 
    if(!contains(toLower(_key))) keys.push_back(toLower(_key)); 
    value[toLower(_key)] = _value; 
    units[toLower(_key)] = _units; 
  }

void 
Options::add(const char* _key, const char* _value, const char* _units)
  { add(std::string(_key),std::string(_value),std::string(_units)); }

void
Options::parseLine(const string& s){

    if (isOption(s)){ // Make sure we are dealing with an option line
      string line = stripWS(s);        // strip white space
      line = stripComments(line);      // strip comments
      size_t i = line.find(separator); // find the separator
      string buffer = stripWS(line.substr(0,i-1)); // get keyword + units
      string keyword = getKeyword(buffer); //get just the keyword
      if(keyword.size() == 0) return;
      if(!contains(keyword)) keys.push_back(keyword); 
      if(hasUnits(buffer)) units[keyword] = getUnits(buffer); // add units
      else units[keyword] = string("");
      value[keyword] = stripWS(line.substr(i+1,line.size()-i-1)); // add value
    }
}

bool
Options::hasUnits(const string& s)const {
  size_t i = s.find(units_left);
  size_t j = s.find(units_right);
  if(i != string::npos && j != string::npos) return true;
  return false;
}

string
Options::getUnits(const string& s)const {
  size_t i = s.find(units_left);
  size_t j = s.find(units_right);
  if(i == string::npos || j == string::npos || i+1 >= j || j-1 <= i){
    throw Exception("Malformed units in option string",
		    "Options::getUnits");
  }
  return s.substr(i+1,j-i-1);
}

string
Options::getKeyword(const string& s)const {
  string keyword;
  
  // Strip off the units
  
  size_t i = s.find(units_left);
  if(i == string::npos){
    keyword = toLower(stripWS(s)); // no units found
  }
  else{
    keyword = toLower(stripWS(s.substr(0,i-1)));
  }
  return keyword;
}

string
Options::toLower(const string& s)const {
  string S(s);
  for(size_t i = 0; i < S.size(); i++)S[i] = tolower(S[i]);
  return S;
}

string
Options::toUpper(const string& s)const {
  string S(s);
  for(size_t i = 0; i < S.size(); i++)S[i] = toupper(S[i]);
  return S;
}

bool
Options::isOption(const string& s)const {
  size_t i = s.find(separator);
  if (i == string::npos) return false;
  return true;
}

string
Options::stripWS(const string& s)const {
  size_t i,j;
  if (s.size() == 0) return s;
  for(i = 0; i < s.size(); i++){
    if(s[i] != ' ' && s[i] != '\t' && s[i] != '\n' )break;
  }
  for(j = s.size()-1; j > 0; j--){
    if(s[j] != ' ' && s[j] != '\t' && s[j] != '\n' )break;
  }
  return s.substr(i,j-i+1);
}

string
Options::stripComments(const string& s)const {
  size_t i = s.find(comment);
  if (i == string::npos) return s;
  return s.substr(0,i);
}

string
Options::stripContinuation(const string& s)const {
  size_t i = s.find(continuation);
  if (i == string::npos) return s;
  return s.substr(0,i);
}

void
Options::print(ostream& os) {
  for(list<string>::iterator key = keys.begin(); key != keys.end(); key++){
    os<<(*key) <<"\t("<<units[*key]<<") = "<<value[*key]<<endl;
  }
}


bool 
Options::contains(const std::string& s){
  string aKey = toLower(s);
  for(list<string>::iterator key = keys.begin(); key != keys.end(); key++){
    if( (*key) == aKey) return true;
  }
  return false;
}

std::string& 
Options::operator[](const std::string& key){
  if(!contains(key)){
    throw Exception(string("Lookup error"),
		    string("Options::operator[]"),
		    string("Keyword: \"")+key+string("\" not found"));
  } 
  return value[toLower(key)]; 
}

std::string& 
Options::operator[](const char* key){
  if(!contains(key)){
    throw Exception(string("Lookup error"),
		    string("Options::operator[]"),
		    string("Keyword: \"")+key+string("\" not found"));
  } 
  return value[toLower(std::string(key))]; 
}

int 
Options::toInt(const std::string& key) {
  if(!contains(key)){
    throw Exception(string("Lookup error"),
		    string("Options::operator[]"),
		    string("Keyword: \"")+key+string("\" not found"));
  } 
  return atoi(value[toLower(std::string(key))].c_str());
} 

double 
Options::toFloat(const std::string& key) {
  if(!contains(key)){
    throw Exception(string("Lookup error"),
		    string("Options::operator[]"),
		    string("Keyword: \"")+key+string("\" not found"));
  } 
  return atof(value[toLower(key)].c_str()); 
}
 
const char* 
Options::toChar(const std::string& key) {
  if(!contains(key)){
    throw Exception(string("Lookup error"),
		    string("Options::operator[]"),
		    string("Keyword: \"")+key+string("\" not found"));
  } 
  return value[toLower(key)].c_str(); 
} 



