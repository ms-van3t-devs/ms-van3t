/*
 *  Extension module for asn1cpp, implementing the support to asn1c BIT_STRING_t
 *   Copyright (C) 2021 Francesco Raviglione (francescorav.es483@gmail.com)
 *                      Marco Malinverno (marco.malinverno1@gmail.com)
 *   asn1cpp project by Eugenio Bargiacchi: https://github.com/Svalorzen/asn1cpp 
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ASN1CPP_BITSTRING_HEADER_FILE
#define ASN1CPP_BITSTRING_HEADER_FILE

#include "BIT_STRING.h"
#include <cstring>

namespace asn1cpp {
    namespace bitstring {
        // This static inline function allocates, only if needeed, more space to the asn1c BIT_STRING buffer (buf)
        static inline bool bitstringAlloc(int requiredBytes, BIT_STRING_t & field) {
            if(requiredBytes > (int) field.size) {
                uint8_t * realloc_ptr;
                realloc_ptr = static_cast<uint8_t *>(realloc((void *)field.buf,requiredBytes));
                if(!realloc_ptr) return false;

                if(field.size == 0) {
                    memset(realloc_ptr,0,requiredBytes);
                } else {
                    // Set to 0 the additional portion of memory which was just allocated
                    memset(realloc_ptr+field.size,0,requiredBytes-field.size);
                }

                field.buf = realloc_ptr;
                field.size = requiredBytes;
            }

            return true;
        }

        inline bool setterBit(BIT_STRING_t & field, unsigned int bit_number) {
        	int requiredBytes = (bit_number / 8 + 1);

        	if(bitstringAlloc(requiredBytes,field) == false) return false;

        	field.buf[bit_number / 8] |= 1 << (7 - bit_number % 8);

        	return true;
        }

        inline bool setterBit(BIT_STRING_t *& field, unsigned int bit_number) {
        	if(!field) field = static_cast<BIT_STRING_t *>(calloc(1, sizeof(BIT_STRING_t)));
        	if(!field) return false;
        	return setterBit(*field,bit_number);
        }

        inline bool setterBit(BIT_STRING_t & field, uint8_t bytebitmask, unsigned int bytebitmask_pos) {
            if(bitstringAlloc(bytebitmask_pos+1,field) == false) return false;

            if(bytebitmask_pos >= field.size) {
                return false;
            }

            field.buf[bytebitmask_pos] = bytebitmask;

            return true;
        }

        inline bool setterBit(BIT_STRING_t *& field, uint8_t bytebitmask, unsigned int bytebitmask_pos) {
            if(!field) field = static_cast<BIT_STRING_t *>(calloc(1, sizeof(BIT_STRING_t)));
            if(!field) return false;
            return setterBit(*field,bytebitmask,bytebitmask_pos);
        }

        inline uint8_t getterByteMask(BIT_STRING_t & field, unsigned int byte_number, bool * ok = nullptr) {
            if(ok) *ok=true;
            return field.buf[byte_number];
        }


        inline uint8_t getterByteMask(BIT_STRING_t *& field, unsigned int byte_number, bool * ok = nullptr) {
          if(!field) {
              if(ok) *ok=false;
              return false;
          } else {
              if(ok) *ok=true;
              return field->buf[byte_number];
          }
        }

        inline bool checkerBit(BIT_STRING_t & field, unsigned int bit_number, bool * ok = nullptr) {
            if(ok) *ok=true;
            return field.buf[bit_number / 8] & (1 << (7 - bit_number % 8));
        }

        inline bool checkerBit(BIT_STRING_t *& field, unsigned int bit_number, bool * ok = nullptr) {
            if(!field) {
                if(ok) *ok=false;
                return false;
            } else {
                if(ok) *ok=true;
                return field->buf[bit_number / 8] & (1 << (7 - bit_number % 8));
            }

            return false;
        }

	inline bool clearerBit(BIT_STRING_t & field, unsigned int bit_number) {
	    	if(!field.buf) return false;

	    	if((bit_number / 8 + 1) > field.size) {
	    		return false;
	    	}

	    	field.buf[bit_number / 8] &= ~(1 << (7 - bit_number % 8));

	    	// Let's shrink the allocated buffer (if needed)
	    	int shrink_amount = 0;

	    	// Try to understand how much to shrink
	    	// We should never shrink the buffer to size=0, as it may be equivalent of doing free()
	    	// free(), however, should not be done here, as it would cause a double free when the Seq
	    	// object gets destroyed
	    	// This is why we loop until i>0 and not i>=0
	    	for(size_t i=field.size-1;i>0;i--) {
	    		if(field.buf[i] == 0x00) {
	    			shrink_amount++;
	    		} else {
	    			break;
	    		}
	    	}

	    	// If we can shrink the buffer, let's do so
		    if(shrink_amount>0) {
				uint8_t * realloc_ptr;
				realloc_ptr = static_cast<uint8_t *>(realloc((void *)field.buf,field.size-shrink_amount));
				if(realloc_ptr) {
					field.buf = realloc_ptr;
					field.size -= shrink_amount;
				}
	    	}

	    	return true;
    	}

        inline bool clearerBit(BIT_STRING_t *& field, unsigned int bit_number) {
    		if(!field) return false;
    		return clearerBit(*field,bit_number);
    	}
    }
}

// This macro can be used to set a specific bit inside a bit string
// Two options are possibile:
// asn1cpp::bitstring::setBit(<field>,<bit number>) to set a specific
// bit inside the given bit string field
// For instance, to set bit 1, you can use:
// asn1cpp::bitstring::setBit(<field>,1)
// The second option is to use:
// asn1cpp::bitstring::setBit(<field>,<bitmask>,<bitmask position>)
// to set a whole 8-bit bitmask in a given position of the bit string
// buffer
// For instance, if you have a 19-bit bit string:
// [........] [........] [...]
//   byte 0     byte 1    byte 2
// You can set the whole central byte (byte 1, starting from 0) with:
// asn1cpp::bitstring::setBit(<field>,<bitmask>,1)
// setBit() returns 'false' if an error occurred when setting the bit,
// 'true' otherwise
#define setBit(field, bit_number, ...) \
    setterBit(field, bit_number, ## __VA_ARGS__)

// This macro can be used to clear a specific bit inside a bit string
// It should be used by specifying the field and the bit number inside
// the bit string
// For instance, to clear bit 1: asn1cpp::bitstring::setBit(<field>,1)
// The function clearerBit(), wrapped by the macro, also manages memory
// and can free some memory when groups of 8-bits become all set to zero
// clearBit() returns 'false' if an error occurred when clearing the bit,
// 'true' otherwise
#define clearBit(field, bit_number, ...) \
    clearerBit(field, bit_number, ## __VA_ARGS__)

// This macro can be used to check if a bit has been set inside a bit
// string
// checkBit() returns 'true' is the specified bit has been set, 'false'
// otherwise
// Its main usage is to check which bits have been set in a bit string
// inside a decoded structure
#define checkBit(field, bit_number, ...) \
    checkerBit(field, bit_number, ## __VA_ARGS__)

#endif
