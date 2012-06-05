/*!
  \file HDF42Blitz.h
  \author E. Rodriguez
  \brief A template function to median filter a 2D Blitz array containing flagged data.
*/

#ifndef _ER_MEDIAN_H_
#define _ER_MEDIAN_H_

#include <iostream>
#include <vector>
#include <algorithm>
#include <blitz/array.h>

/*! 
  Return the median of the elements stored in a vector. The vector
  is rearranged.
*/

template <class T>
T median(std::vector<T>& v,
	 int n = -1
	 ){
  if(n == -1) n = v.size();
  int mid = (n+1)/2;
  partial_sort(v.begin(),v.begin()+mid,v.begin()+n);
  return v[mid-1];
}

/*!
    Median filter of a matrix whose type allows the use of an ordering operator.
    Only unmasked points are used to form the result. The output will have the
    filtered values, when valid, or the original values, when not valid. These
    can be differentiated using the output mask.
*/

template<class T>
void
median(const blitz::Array<T, 2> & input_array, //!< Matrix to smooth (not changed)
       const blitz::Array<bool, 2>& input_mask, //!< when false, do not use these points
       int minGood,   //!< Minimum number of good points in the window
       int nsmooth,   //!< smoothing size for rows
       int msmooth,  //!< smoothing size for columns
       blitz::Array<T, 2> & output_array, //!< Median filtered result (resized to match input array)
       blitz::Array<bool, 2>& output_mask //!< These are the filtered points mask
       ){

  int i,j,n,m;

  // resize result arrays
  
  output_array.resize(input_array.rows(),input_array.cols());
  output_mask.resize(input_array.rows(),input_array.cols());

  // Calculate the smoothing radii, and return if there is nothing to do.

  if( msmooth==0 ) msmooth = nsmooth;
  int rowRadius = (nsmooth-1)/2;
  int colRadius = (msmooth-1)/2;
  if((rowRadius<=0)||(input_array.rows()<=nsmooth) ||
     (colRadius<=0)||(input_array.cols()<=msmooth)) {
    output_array = input_array;
    output_mask = input_mask;
    return;
  }

  // Fill the edges

  blitz::Range all = blitz::Range::all();
  blitz::Range row_range_low = blitz::Range(0,rowRadius-1);
  blitz::Range row_range_high = blitz::Range(input_array.rows()-rowRadius,input_array.rows()-1);
  blitz::Range col_range_low = blitz::Range(0,colRadius-1);
  blitz::Range col_range_high = blitz::Range(input_array.cols()-colRadius,input_array.cols()-1);

  output_mask(row_range_low,all) = false;
  output_mask(row_range_high,all) = false;
  output_mask(all,col_range_low) = false;
  output_mask(all,col_range_high) = false;

  output_array(row_range_low,all) = input_array(row_range_low,all);
  output_array(row_range_high,all) = input_array(row_range_high,all);
  output_array(all,col_range_low) = input_array(all,col_range_low);
  output_array(all,col_range_high) = input_array(all,col_range_high);

  // This vector holds the data temprarily
  
  std::vector<T> buffer(nsmooth*msmooth);
  
  //Process one row at a time
  
  for(i = rowRadius; i< input_array.rows()-rowRadius; i++){
    for(j = colRadius; j < input_array.cols()-colRadius; j++){
    
        // select the good points to filter
        
        int good = 0;
	// AHChau 6/3/12, add this if statement, so that no filtering will be performed if the center value is not good.  
	if( input_mask(i,j) ){ 
	  for(n = i-rowRadius; n <= i+rowRadius; n++){
            for(m = j - colRadius; m <= j + colRadius; m++){
	      if( input_mask(n,m) ) buffer[good++] = input_array(n,m);
            }
	  }
	}

        // Fill in the result
        
        if(good > minGood){
  	  output_array(i,j) = median(buffer,good);
  	  output_mask(i,j) = true;
  	}
  	else {
  	  output_array(i,j) = input_array(i,j);
  	  output_mask(i,j) = false;
  	}
    }
  }

}


#endif
