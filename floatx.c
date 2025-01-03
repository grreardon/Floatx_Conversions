#include "floatx.h"
#include <assert.h>
#include <limits.h> // for CHAR_BIT - number of bits per byte
#include <math.h>	// for isinf and isnan
#include "bitFields.h"
#include <stdio.h>
#include <stdlib.h> 

void printBits(unsigned long n){
    unsigned long i;
    i = 1UL<<(sizeof(n)*CHAR_BIT-1);
    while(i>0){
         if(n&i)
              printf("1");
         else
              printf("0");
         i >>= 1;
    }
}

floatx doubleToFloatx(double val, int totBits, int expBits)
{
	
	assert(totBits > 3); 
      	assert(totBits <= 64); 
	assert(expBits > 1);
      	assert(expBits < (totBits - 2));

	union hexDouble
	{
		double dbl;
		unsigned long lng;
	} unionVal;

	unionVal.dbl = val;
	unsigned long bitRepresentation = unionVal.lng;
	unsigned long signBit = bitRepresentation >> 63; // 63 bit padding in front; 
	unsigned long biasedExponent = getBitFld(63 - 11, 11, bitRepresentation); //53 bit padding in front; 
	unsigned long mantissa =(bitRepresentation << 12) >> 12; // 12 bit padding in front  
	floatx new_format = signBit << (totBits - 1);
	
	long unbiasedExponent = labs(biasedExponent) - 1023;
	
	//SUBNORMAL CHECK 
	if(biasedExponent == 0) {
		int toSub = 0;
	       	for(int i = 51; i >= 0; i--) { 
			if(((mantissa >> i) & 1L) == 1) break;
			toSub++; 
		}

		//if Number inputted is too small to be computed, mantissa will be all zero 
		if(toSub >= totBits - expBits - 1) {
			if((signBit & 1L) == 1) return (1UL << (totBits - 1));
			else return 0;
		}	
		
		//Normalizing the subnormal unbiased exponent & mantissa bits
		unbiasedExponent -= toSub; 
		mantissa = mantissa << toSub;
	
	}
	if(((unbiasedExponent + (1L << (expBits - 1)) - 1) <= 0)) unbiasedExponent -= 1;   
	long rebiasedExponent =  ((1L << (expBits - 1)) -1) + unbiasedExponent;
	
	//Rebiased Exponent is too small too be normalized 
    	if(rebiasedExponent + (totBits - (expBits + 1)) <= 0) { 
		if((signBit & 1) == 1) return (1UL << (totBits -1)); 
		else return 0; 
	}

	//Rebiased exponent too large to be normal value, NAN or infinity
	if(rebiasedExponent >= ((1L << expBits) - 1)) {
		new_format = (new_format | (((1UL << expBits) - 1) << (totBits - expBits - 1))); 
		
	        if(rebiasedExponent > ((1L << expBits) - 1)) {
			return new_format; 
		} else {
			if(totBits - expBits - 1 <= 52) {
				new_format = (new_format | (mantissa >> (52 - (totBits - expBits - 1)))); 
			} else {
				new_format = (new_format | (mantissa << ( (totBits - expBits - 1) - 52))); 
			}
			return new_format; 
		}	
	}

	 if(rebiasedExponent < 0) {
		mantissa = (mantissa >> (labs(rebiasedExponent)));
		mantissa = mantissa | (1UL << 51);
		rebiasedExponent = 0; 
	 }

	 if(totBits - expBits - 1 <= 52) {
                mantissa = mantissa >> (52 - (totBits - expBits - 1));
          } else {
                mantissa = mantissa << ( (totBits - expBits - 1) - 52); 
         }

	new_format = (new_format | (rebiasedExponent) << (totBits - expBits - 1));
       	new_format = new_format | mantissa; 
	return new_format; 	
	/*----------------------------------
	-------------------------------------------------------------
		From the README: 	At a high level, doing the conversion requires several manipulations:
		1. Extracting the sign bit from the double value, and inserting it into the floatx value at
			the correct position.
		2. Extract the biased exponent from the double value.
				Check to see if the double value is sub-normal.
				Check to make sure the floatx exponent won't overflow or underflow).
						If so, handle as a special case.
				If not, rebias the exponent using the floatx bias (which depends on the number of exponent bits)
				and write the result to the correct location in the floatx result.
		3. Extract the fraction bits from the double value.
				Determine how many bits are available for the fraction in the floatx value,
				and truncate or extend the original value,
				and write the resulting bits to the floatx result.
		4. Handle special cases, such as infinity, or not-a-number.
		5. Return the floatx result.
	----------------------------------------------------------------------------------------------------*/

	// First, make some assertions to ensure the totBits and expBits parameters are OK
	// Then, implement the algorithm

}
