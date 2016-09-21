/*
 * bitarray_extension.c
 *
 *  Created on: 20/9/2016
 *      Author: utnso
 */

#include <stdlib.h>
#include <commons/bitarray.h>

#define BIT_IN_CHAR_INVERSE(bit)      (0x80 >> (((bit)  % CHAR_BIT)))

bool bitarray_test_bit_inverse_char(t_bitarray *self, off_t bit_index) {
	return((self->bitarray[BIT_CHAR(bit_index)] & BIT_IN_CHAR_INVERSE(bit_index)) != 0);
}
