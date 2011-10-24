#include <math.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <blitz/array.h>
#include "median.h"

using namespace std;
using namespace blitz;

int main(int argc, char* argv[]){

  int A[] = {1,4,2,8,5,7,9,10};
  int N = sizeof(A)/sizeof(int);
  partial_sort(A,A+(N+1)/2,A+N);
  copy(A,A+N,ostream_iterator<int>(cout," "));
  cout<<endl;
  int B[] = {1,4,2,8,5,7,9,10};
  N = sizeof(B)/sizeof(int);
  vector<int> V(B,B+N);
  int mid = median(V);
  copy(V.begin(),V.end(),ostream_iterator<int>(cout," "));
  cout<<endl;
  cout<<"Median: "<<mid<<endl;

  Array<float, 2> m(7,7);
  for(int i = 0; i < 7; i++){
    for(int j = 0; j < 7; j++){
      m(i,j) = int(100*sin(10*float(i*j)));
    }
  }

  cout<<"Input matrix"<<endl;
  cout<<m<<endl;

  Array<bool, 2> input_mask(m.rows(),m.cols());
  input_mask = ( m != 0 ) && (m != 21);
  cout<<"input mask"<<endl;
  cout<<input_mask<<endl;

  Array<float,2> M;
  Array<bool, 2> output_mask;
  median(m,input_mask,1,3,3,M,output_mask);
  cout<<"Median filtered matrix"<<endl;
  cout<<M<<endl;
  cout<<"Output mask"<<endl;
  cout<<output_mask<<endl;

  cout<<"Conservative mask"<<endl;
  cout<< Array<bool,2>(input_mask && output_mask) <<endl;

  return 0;
}
