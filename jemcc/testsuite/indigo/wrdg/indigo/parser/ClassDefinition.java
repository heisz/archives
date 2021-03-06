// Indigo VM testsuite for testing basic class definition parsing failures.
// Copyright (C) 1999-2004 J.M. Heisz 
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// See the file named COPYRIGHT in the root directory of the source
// distribution for specific references to the GNU General Public License,
// as well as further clarification on your rights to use this software.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

package wrdg.indigo.parser;

/**
 * <P>
 * Indigo test program to test the parsing of invalid class definition 
 * details (headers, cross-references, etc.).
 * </P>
 *
 * <P>Copyright (&#169;) 1999-2004 J.M. Heisz</P>
 *
 * <P>This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.</P>
 *
 * @author J.M. Heisz
 * @version $Revision: 1.3 $ $Date: 2004/01/05 12:42:13 $
 */

public class ClassDefinition extends ClassLoader {
    /* This is the correct class from which some test cases are derived */
    private static int[] properClassBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x00, 0x00, 0x2e,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[1] */
    /* Insufficient bytes to define a class */
    private static int[] tooSmallBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x00, 0x00, 0x2e,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, /* ERROR */
        /* SNIP */
    };

    /* INTERNAL - classparser[2] */
    /* Some bad magic happening here */
    private static int[] badMagicBytes = {
        /* Header */
        0xde, 0xad, 0xbe, 0xef, /* ERROR - should be "cafebabe" */
            0x00, 0x00, 0x00, 0x2e,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[3,4,5] */
    /* Invalid version for the VM */
    private static int[] invalidVersionBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x66, /* ERROR */
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[13] */
    /* Data truncated in class attribute flags */
    private static int[] classFlagTruncBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00  /* ERROR - truncated in class flags */
    };

    /* INTERNAL - classparser[13*] */
    /* Data truncated in class index */
    private static int[] classIdxTruncBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, /* ERROR - truncated in class instance */
    };

    /* INTERNAL - classparser[13*] */
    /* Data truncated in superclass index */
    private static int[] superIdxTruncBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, /* ERROR - truncated in superclass */
    };

    /* INTERNAL - undefined */
    /* Non-abstract interface class */
    private static int[] nonAbstractIFBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x02, 0x21, /* ERROR - non-abstract interface */
                0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[14] */
    /* Abstract final class */
    private static int[] abstractFinalBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x04, 0x31, /* ERROR - abstract final */
            0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[31] */
    /* Invalid class index */
    private static int[] invalidClassIdxBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x0a, /* ERROR - invalid class index */
                            0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[31*] */
    /* Really bad class index (out of range) */
    private static int[] badClassIdxBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x12, 0x34, /* ERROR - class index way out of range */
                            0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[32] */
    /* No superclass (not object) */
    private static int[] noSuperclassBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 
                0x00, 0x00, /* ERROR - only Object can have no superclass */
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[34] */
    /* Invalid superclass index */
    private static int[] invalidSuperclassIdxBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, 0x01, /* ERROR - invalid super index */
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[33] */
    /* Really bad superclass index (out of range) */
    private static int[] badSuperclassIdxBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02,
                0xef, 0xdc, /* ERROR - superclass index way out of range */
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c
    };

    /* INTERNAL - classparser[30] */
    /* Byte overrun (class too big) */
    private static int[] tooManyClassBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x10, 0x0a, 0x00, 0x03, 0x00, 0x0d, 0x07,
        0x00, 0x0e, 0x07, 0x00, 0x0f, 0x01, 0x00, 0x01,
        0x66, 0x01, 0x00, 0x01, 0x49, 0x01, 0x00, 0x06,
        0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01, 0x00,
        0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04, 0x43,
        0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c, 0x69,
        0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65, 0x72,
        0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x01,
        0x6d, 0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72,
        0x63, 0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00,
        0x08, 0x63, 0x6c, 0x73, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x06, 0x00, 0x07, 0x01, 0x00,
        0x03, 0x63, 0x6c, 0x73, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
        /* Class Details */
        0x00, 0x21, 0x00, 0x02, 0x00, 0x03, 
        /* Interfaces */
        0x00, 0x00, 
        /* Fields */
        0x00, 0x01, 
            0x00, 0x00, 0x00, 0x04, 0x00, 0x05, 0x00, 0x00, 
        /* Methods */
        0x00, 0x02, 
            0x00, 0x01, 0x00, 0x06, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x1d, 
                    0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 
                    0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00, 
                    0x01, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 
                    0x01, 0x00, 0x00, 0x00, 0x01, 
            0x00, 0x01, 0x00, 0x0a, 0x00, 0x07, 
                0x00, 0x01, 0x00, 0x08, 0x00, 0x00, 0x00, 0x19, 
                    0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 
                    0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x09, 0x00, 
                    0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 
                    0x05, 
        /* Class Attributes */
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0c,
 
        0x01 /* ERROR - one extra byte in the class */
    };

    /**
     * Locally run through the test cases.
     */
    public static void main(String argv[]) {
        /* Use an instance of myself for testing */
        ClassDefinition tstInstance = new ClassDefinition();

        /* Way too small */
        try {
            tstInstance.localDefineClass("cls", tooSmallBytes);
            throw new Error("ERROR: Small class instance parsed");
        } catch (ClassFormatError ex) {}

        /* Incorrect magic number */
        try {
            tstInstance.localDefineClass("cls", badMagicBytes);
            throw new Error("ERROR: Invalid magic number parsed");
        } catch (ClassFormatError ex) {}

        /* Invalid version information */
        try {
            tstInstance.localDefineClass("cls", invalidVersionBytes);
            throw new Error("ERROR: Invalid version number parsed");
        } catch (ClassFormatError ex) {}

        /* Truncated class access fields */
        try {
            tstInstance.localDefineClass("cls", classFlagTruncBytes);
            throw new Error("ERROR: Truncated class access flags parsed");
        } catch (ClassFormatError ex) {}

        /* Truncated class index */
        try {
            tstInstance.localDefineClass("cls", classIdxTruncBytes);
            throw new Error("ERROR: Truncated class index parsed");
        } catch (ClassFormatError ex) {}

        /* Truncated super class index */
        try {
            tstInstance.localDefineClass("cls", superIdxTruncBytes);
            throw new Error("ERROR: Truncated superclass index parsed");
        } catch (ClassFormatError ex) {}

        /* Non-abstract interface class */
        try {
            tstInstance.localDefineClass("cls", nonAbstractIFBytes);
            throw new Error("ERROR: Non-abstract interface parsed");
        } catch (ClassFormatError ex) {}

        /* Abstract final class */
        try {
            tstInstance.localDefineClass("cls", abstractFinalBytes);
            throw new Error("ERROR: Abstract final class parsed");
        } catch (ClassFormatError ex) {}

        /* Invalid class index */
        try {
            tstInstance.localDefineClass("cls", invalidClassIdxBytes);
            throw new Error("ERROR: Invalid class index parsed");
        } catch (ClassFormatError ex) {}

        /* Really invalid class index */
        try {
            tstInstance.localDefineClass("cls", badClassIdxBytes);
            throw new Error("ERROR: Really invalid class index parsed");
        } catch (ClassFormatError ex) {}

        /* No superclass index */
        try {
            tstInstance.localDefineClass("cls", noSuperclassBytes);
            throw new Error("ERROR: no superclass (non-Object) parsed");
        } catch (ClassFormatError ex) {}

        /* Invalid superclass index */
        try {
            tstInstance.localDefineClass("cls", invalidSuperclassIdxBytes);
            throw new Error("ERROR: Invalid superclass index parsed");
        } catch (ClassFormatError ex) {}

        /* Really invalid superclass index */
        try {
            tstInstance.localDefineClass("cls", badSuperclassIdxBytes);
            throw new Error("ERROR: Really invalid superclass index parsed");
        } catch (ClassFormatError ex) {}

        /* Too many bytes */
        try {
            tstInstance.localDefineClass("cls", tooManyClassBytes);
            throw new Error("ERROR: Oversized class parsed");
        } catch (ClassFormatError ex) {}

        /* After all the failures, it should be usable */
        tstInstance.localDefineClass("cls", properClassBytes);
    }

    /**
     * Local method to wrap class definition with integer byte arrays.
     */
    public void localDefineClass(String className, int[] byteData) {
        byte[] dataSet = new byte[2048];

        for (int i = 0; i < byteData.length; i++) {
            dataSet[i] = (byte) byteData[i];
        }
        defineClass(className, dataSet, 0, byteData.length);
    }

    /**
     * Method to provide support for JDK1.1 compilers.
     */
    public Class loadClass(String className, boolean resolve) 
            throws ClassNotFoundException {
        return Class.forName(className);
    }
}
