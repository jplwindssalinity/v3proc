/*!
  \file Exception.h
  \brief A simple class to throw exceptions using strings.
  \author E. Rodriguez
*/

#ifndef _ER_EXCEPTION_H_
#define _ER_EXCEPTION_H_

#include <string>
#include <iostream>

/*!
  \brief An exception class based on std::string; It also has all members public.

  STD string based exception messages, rather than char*, makes it
  more convenient than xmsg. Also, the messages are not encapsulated
  for ease of concatenating error messages.

  Finally, if the verbose option is set (true by default), a cerr message
  is also passed before being thrown.
*/

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


#endif
