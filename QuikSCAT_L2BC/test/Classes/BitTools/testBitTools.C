#include <iostream>
#include <bitset>
#include <string>
#include "BitTools.h"

using namespace std;

int main(int argc, char* argv[]) {

  bitset<16> b16(string("0000000000001010"));
  bitset<8> b8(string("00001010"));
  bitset<16> b88(string("0000101000001010"));

  cout<<b16<<"\t"<<bitset<16>(to_string(b16))<<endl;
  cout<<b8<<endl;

  short f16_10 = 10;
  bitset<16> b16_10 = size_t(f16_10);
  cout<<b16_10<<endl;

  cout<<b16_10<<" == "<<b16<<" = "<<int(b16_10 == b16)<<endl;

  cout<<int(bitpattern_equals(b16,b16_10,16))<<"\t"<<int(bitpattern_equals(b16,b16_10,16))<<"\t"
      <<int(bitpattern_equals(b16,b8,16))<<"\t"<<int(bitpattern_equals(b16,b8,8))
      <<"\t"<<int(b88==b16)<<"\t"<<int(bitpattern_equals(b16,b88,16))
      <<"\t"<<int(bitpattern_equals(b16,b88,8))<<"\t"<<int(bitpattern_equals(b16,b88,8,8,0))
      <<"\t"<<int(bitpattern_equals(b88,b8,8))<<"\t"<<int(bitpattern_equals(b88,b8,8,8,0))
      <<endl;

  cout<<int(bitpattern_equals(f16_10,b8,8))<<endl;
  return 0;
}
