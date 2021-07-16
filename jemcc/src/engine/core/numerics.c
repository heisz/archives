/**
 * Utility methods for performing various numeric (IEE754 and other) operations.
 * Copyright (C) 1999-2004 J.M. Heisz 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU Lesser General Public 
 * License, as well as further clarification on your rights to use this 
 * software.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "jeminc.h"
#include "jni.h"
#include "numerics.h"

static char digits[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                         'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                         'k', 'l', 'm' ,'n', 'o', 'p', 'q', 'r', 's', 't',
                         'u', 'v', 'w', 'x', 'y', 'z' };

/**
 * Convert an integer value into a character representation (for textual
 * output, for example). Essentially the Integer.toString(val, radix) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    radix - the radix to use in the text conversion
 *    buff - the character buffer to write into (should be at least 34 bytes
 *           for binary values)
 */
void JEMCC_IntegerToText(jint val, jint radix, char *buff) {
    char wrkBuff[33], *ptr = wrkBuff;

    /* Handle negative, watch out for that really minimal value */
    if (val < 0) {
        if (val == 0x80000000) {
            *(ptr++) = digits[-(val + radix) % radix];
            val = -(val / radix);
        } else {
            val = -val;
        }
        *(buff++) = '-';
    } else if (val == 0) {
        *(buff++) = '0';
        *buff = '\0';
        return;
    }

    /* Decompose the individual digits */
    while (val > 0) {
        *(ptr++) = digits[val % radix];
        val = val / radix;
    }

    /* Reverse the order into the final buffer */
    while (ptr > wrkBuff) {
        *(buff++) = *(--ptr);
    }
    *buff = '\0';
}

/**
 * Convert an long value into a character representation (for textual
 * output, for example). Essentially the Long.toString(val, radix) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    radix - the radix to use in the text conversion
 *    buff - the character buffer to write into (should be at least 67 bytes
 *           for binary values)
 */
void JEMCC_LongToText(jlong val, jint radix, char *buff) {
    char wrkBuff[66], *ptr = wrkBuff;

    /* Handle negative, watch out for that really minimal value */
    if (val < 0) {
        if (val == 0x8000000000000000) {
            *(ptr++) = digits[-(val + radix) % radix];
            val = -(val / radix);
        } else {
            val = -val;
        }
        *(buff++) = '-';
    } else if (val == 0) {
        *(buff++) = '0';
        *buff = '\0';
        return;
    }

    /* Decompose the individual digits */
    while (val > 0) {
        *(ptr++) = digits[val % radix];
        val = val / radix;
    }

    /* Reverse the order into the final buffer */
    while (ptr > wrkBuff) {
        *(buff++) = *(--ptr);
    }
    *buff = '\0';
}

/**
 * Convert a float value into a character representation (for textual
 * output, for example).  Follows the ruleset and is essentially the same
 * as the Float.toString(val) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    buff - the character buffer to write into
 */
void JEMCC_FloatToText(jfloat val, char *buff) {
    char *ptr;
    int foundDec;

    /* TODO - follow the JLS for representation modes */
    (void) sprintf(buff, "%f", val);

    /* Trim trailing precision zeros (rough approximation) */
    ptr = buff;
    foundDec = 0;
    while (*ptr != '\0') {
        if (*(ptr++) == '.') foundDec = 1;
    }
    if (foundDec != 0) {
        ptr--;
        while ((*ptr == '0') && (*(ptr - 1) != '.')) *(ptr--) = '\0';
    }
}

/**
 * Convert an double value into a character representation (for textual
 * output, for example).  Follows the ruleset and is essentially the same
 * as the Double.toString(val) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    buff - the character buffer to write into
 */
void JEMCC_DoubleToText(jdouble val, char *buff) {
    char *ptr;
    int foundDec;

    /* TODO - follow the JLS for representation modes */
    (void) sprintf(buff, "%f", val);

    /* Trim trailing precision zeros (rough approximation) */
    ptr = buff;
    foundDec = 0;
    while (*ptr != '\0') {
        if (*(ptr++) == '.') foundDec = 1;
    }
    if (foundDec != 0) {
        ptr--;
        while ((*ptr == '0') && (*(ptr - 1) != '.')) *(ptr--) = '\0';
    }
}
