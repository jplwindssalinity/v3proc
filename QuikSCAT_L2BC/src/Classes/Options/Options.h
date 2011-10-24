/*!
  \file Options.h
  \author E. Rodriguez
  \brief Class to read option files. A replacement to RDF in C++ with
  greater ease of use and flexibility.
*/

#ifndef _ER_OPTIONS_H_
#define _ER_OPTIONS_H_

#include <string.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>

/*!
  \brief Class to read option files. A replacement to RDF in C++ with
  greater ease of use and flexibility.
*/

class Options {
public:

  ////////////////////////////////////////////////////////////////////////
  //
  // Start user interface.
  //
  ////////////////////////////////////////////////////////////////////////

  Options(const char* _separator = "=",
	  const char* _continuation = "\\",
	  const char* _comment = "!",
	  const char* _eof = "eof",
	  const char* _units_left = "(",
	  const char* _units_right = ")"
	  ):separator(_separator), continuation(_continuation),
    comment(_comment),eof(_eof),
    units_left(_units_left), units_right(_units_right) {}

  //! Parse a file and fill keywords

  void parseFile(const char* fileName);

  //! Parse a line and fill keywords

  void parseLine(const std::string& s);
  void parseLine(const char* s) { parseLine(std::string(s)); }

  //! Get the value (as a string) by left or right addressing

  std::string& operator[](const std::string& key);

  std::string& operator[](const char* key);

  //! Get the value as an integer

  int toInt(const std::string& key);

  //! Get the value as a float double

  double toFloat(const std::string& key);

  //! Get the value as a const char*

  const char* toChar(const std::string& key);

  // Add a new set of values

  void add(const std::string& _key, const std::string& _value,
	   const std::string& _units = std::string("")) { 
    if(!contains(toLower(_key))) keys.push_back(toLower(_key)); 
    value[toLower(_key)] = _value; 
    units[toLower(_key)] = _units; 
  }

  void add(const char* _key, const char* _value, const char* _units = "")
  { add(std::string(_key),std::string(_value),std::string(_units)); }

  //! Inquire if a keyword exists

  bool contains(const std::string& s);
  bool contains(const char* s) {return contains(std::string(s));}

  //! Print to an open ostream

  void print(std::ostream& os);

  ////////////////////////////////////////////////////////////////////////
  //
  // End user interface.
  //
  ////////////////////////////////////////////////////////////////////////

  // data members and helper functions (public, in case of future need)

  //! Used to order strings

  struct ltstr {
    bool operator()(std::string s1, std::string s2) const{
      return strcmp(s1.c_str(), s2.c_str()) < 0;
    }
  };

  std::map<std::string,std::string,ltstr> value;
  std::map<std::string,std::string,ltstr> units;
  std::list<std::string> keys;
  std::string separator;     //!< separates keyword and value (= in RDF)
  std::string continuation;  //!< continuation line character (\ in RDF)
  std::string comment;       //!< Anything after this is ignored (! in RDF)
  std::string eof;           //!< The parser stops after finding this.
  std::string units_left;    //!< Left container for units ( "(" in RDF )
  std::string units_right;   //!< Left container for units ( ")" in RDF )

  // Helper functions

  std::string stripWS(const std::string& s)const;
  std::string toLower(const std::string& s)const;
  std::string toUpper(const std::string& s)const;
  std::string stripComments(const std::string& s)const;
  std::string stripContinuation(const std::string& s)const;
  std::string getKeyword(const std::string& s)const;
  std::string getUnits(const std::string& s)const;
  bool isOption(const std::string& s)const;
  bool hasUnits(const std::string& s)const;

  //! The class to use for Options exceptions

  class Exception {
  public:

    Exception(const char* _what, 
	      const char* _where="", 
	      const char* _why="",
	      bool _verbose = true):
      what(_what), where(_where), why(_why), verbose(_verbose) {
      if(verbose) this->print(std::cerr);
    }

    Exception(const std::string& _what, 
	      const std::string& _where = std::string(""), 
	      const std::string& _why   = std::string(""),
	      bool _verbose = true):
      what(_what), where(_where), why(_why), verbose(_verbose) {
      if(verbose) this->print(std::cerr);
    }

    void raise(void){throw *this;}
    
    std::ostream& print(std::ostream& os){
      os<<" What: "<<what<<std::endl;
      os<<"Where: "<<where<<std::endl;
      os<<"  Why: "<<why<<std::endl;
      return os;
    }

    // These are the messages
    
    std::string what;    //!< What happened
    std::string where;   //!< Where did it happen (default NULL)
    std::string why;     //!< Why dit it happen (default NULL)
    
    bool verbose;        //!< If true, output to cerr
  };

};

#endif
