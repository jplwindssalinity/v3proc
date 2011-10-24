/*!
  \file BitTools.h
  \author Ernesto Rodriguez
  \brief Utility functions to deal with checking bit flags
*/
#ifndef _ER_BITTOOLS_H_
#define _ER_BITTOOLS_H_

#include <bitset>
#include <iostream>

/*!
  Print out a bitset into a stream as a string of 0/1. The bitset is
  printed out from top bit to bottom bit, to match the string initialization.
*/

template <size_t N1>
std::ostream& operator<<(std::ostream& os,    //!< Output stream
			 std::bitset<N1> b //!< Input bitset
			 ) {
  for(size_t i = 0; i < b.size(); i++) {
    if(b[b.size() - 1 - i] == true) os<<"1";
    else os <<"0";
  }
  return os;
}

/*!
  Print out a bitset into a string as a 0/1 sequence. The bitset is
  printed out from top bit to bottom bit, to match the string initialization.
*/

template <size_t N1>
std::string to_string(std::bitset<N1> b //!< Input bitset
		      ) {
  std::string s;
  for(size_t i = 0; i < b.size(); i++) {
    if(b[b.size() - 1 - i] == true) s += std::string("1");
    else s+= std::string("0");
  }
  return s;
}

/*!
  Test whether two bit patterns are equal over a set of bits. 

  If the two patterns are of size incompatible with the requested
  number of bits or offsets, the function returns false.

  \returns Returns true if the two patterns match over the selected bits,
  false otherwise.
*/

template <size_t N1, size_t N2>
bool
bitpattern_equals(std::bitset<N1> b1, //!< First bitset of size N1
		  std::bitset<N2> b2, //!< Second bitset of size N2
		  size_t nbits,        //!< Number of bits to check
		  size_t offset1 = 0,  //!< Offset into first bitset
		  size_t offset2 = 0   //!< Offset into second bitset
		  ) {
  if(b1.size() < offset1 + nbits) return false;
  if(b2.size() < offset2 + nbits) return false;
  for(size_t i = 0; i < nbits; i++) {
    if( b1[offset1 + i] != b2[offset2 + i] ) return false;
  }
  return true;
}

/*!
  Test whether two bit patterns are equal over a set of bits located
  at arbitrary bit locations. The bit locations are given by arrays of
  indexes into the pattern. 

  If the two patterns are of size incompatible with the requested
  number of indexes, the function returns false.

  \returns Returns true if the two patterns match over the selected bits,
  false otherwise.
*/

template <size_t N1, size_t N2>
bool
bitpattern_equals(std::bitset<N1> b1, //!< First bitset of size N1
		  std::bitset<N2> b2, //!< Second bitset of size N2
		  size_t nbits,        //!< Number of bits to check
		  const size_t* offset1,    //!< Index offsets into first bitset
		  const size_t* offset2     //!< Index offsets into second bitset
		  ) {
  for(size_t i = 0; i < nbits; i++) {
    if( offset1[i] >= b1.size() ) return false;
    if( offset2[i] >= b2.size() ) return false;
    if( b1[offset1[i]] != b2[offset2[i]] ) return false;
  }
  return true;
}


/*!
  Test whether a number (which can be cast to a size_t) has the same
  bit pattern as specified in a bitset

  \returns Returns true if the two patterns match over the selected bits,
  false otherwise.
*/

template <class T, size_t N2>
bool
bitpattern_equals(T flag,             //!< Flag value which can be cast to size_t
		  std::bitset<N2> b2, //!< Bitset to compare to
		  size_t nbits,        //!< Number of bits to check
		  size_t offset1 = 0,  //!< Offset into first bitset
		  size_t offset2 = 0   //!< Offset into second bitset
		  ) {
  std::bitset<8*sizeof(T)> b1 = size_t(flag);
  return bitpattern_equals(b1,b2,nbits,offset1,offset2);
}

#endif
