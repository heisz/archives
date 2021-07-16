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

#ifndef JEM_NUMERICS_H
#define JEM_NUMERICS_H 1

/* <jemcc_start> */

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
JNIEXPORT void JNICALL JEMCC_IntegerToText(jint val, jint radix, char *buff);

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
JNIEXPORT void JNICALL JEMCC_LongToText(jlong val, jint radix, char *buff);

/**
 * Convert a float value into a character representation (for textual
 * output, for example).  Follows the ruleset and is essentially the same
 * as the Float.toString(val) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    buff - the character buffer to write into
 */
JNIEXPORT void JNICALL JEMCC_FloatToText(jfloat val, char *buff);

/**
 * Convert an double value into a character representation (for textual
 * output, for example).  Follows the ruleset and is essentially the same
 * as the Double.toString(val) method.
 *
 * Parameters:
 *    val - the value to be converted to text
 *    buff - the character buffer to write into
 */
JNIEXPORT void JNICALL JEMCC_DoubleToText(jdouble val, char *buff);

/* <jemcc_end> */

#endif
