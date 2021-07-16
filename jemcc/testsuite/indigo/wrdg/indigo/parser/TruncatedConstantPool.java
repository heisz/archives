// Indigo VM testsuite for testing truncated constant pool parsing failures.
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
 * Indigo test program to test the parsing of truncated constant
 * pool information.
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

public class TruncatedConstantPool extends ClassLoader {
    /* This is the correct class from which the test cases are derived */
    private static int[] properClassBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
            0x01, 0x00, 0x01, 0x44, /* "D" */
            0x01, 0x00, 0x01, 0x73, /* "s" */
            0x01, 0x00, 0x12, /* "Ljava/lang/String;" */
                0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 
                0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72, 0x69, 0x6e, 
                0x67, 0x3b,
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, 0x43, 0x6f, 0x64, 0x65, /* "Code" */
            0x01, 0x00, 0x0f, /* "LineNumberTable" */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x08, /* "<clinit>" */
                0x3c, 0x63, 0x6c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x19, 0x00, 0x1a, /* nameandtype */
            0x0c, 0x00, 0x0f, 0x00, 0x10, /* nameandtype */
            0x0c, 0x00, 0x11, 0x00, 0x12, /* nameandtype */
            0x0c, 0x00, 0x13, 0x00, 0x14, /* nameandtype */
            0x0c, 0x00, 0x15, 0x00, 0x16, /* nameandtype */
            0x01, 0x00, 0x07, /* "Testing" */
                0x54, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67, 
            0x0c, 0x00, 0x17, 0x00, 0x18, /* nameandtype */
            0x01, 0x00, 0x02, 0x63, 0x70, /* "cp" */
            0x01, 0x00, 0x10, /* "java/lang/Object" */
                0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 
                0x67, 0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
            0x03, 0x00, 0x01, 0x02, 0x03, /* integer */
        /* Remainder */ 
        0x00, 0x21, 0x00, 0x0d, 0x00,
        0x0e, 0x00, 0x00, 0x00, 0x05, 0x00, 0x09, 0x00,
        0x0f, 0x00, 0x10, 0x00, 0x00, 0x00, 0x09, 0x00,
        0x11, 0x00, 0x12, 0x00, 0x00, 0x00, 0x09, 0x00,
        0x13, 0x00, 0x14, 0x00, 0x00, 0x00, 0x09, 0x00,
        0x15, 0x00, 0x16, 0x00, 0x00, 0x00, 0x09, 0x00,
        0x17, 0x00, 0x18, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x01, 0x00, 0x19, 0x00, 0x1a, 0x00, 0x01, 0x00,
        0x1b, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x01, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x05, 0x2a, 0xb7, 0x00,
        0x01, 0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1c,
        0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x08, 0x00, 0x1d, 0x00, 0x1a,
        0x00, 0x01, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x49,
        0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d,
        0x11, 0x7d, 0x6d, 0xb3, 0x00, 0x02, 0x14, 0x00,
        0x03, 0xb3, 0x00, 0x05, 0x12, 0x06, 0xb3, 0x00,
        0x07, 0x14, 0x00, 0x08, 0xb3, 0x00, 0x0a, 0x12,
        0x0b, 0xb3, 0x00, 0x0c, 0xb1, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1a, 0x00,
        0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x06, 0x00,
        0x03, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x11, 0x00,
        0x05, 0x00, 0x17, 0x00, 0x06, 0x00, 0x1c, 0x00,
        0x02, 0x00, 0x01, 0x00, 0x1e, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x1f
    };

    /* INTERNAL - classparser[8] */
    /* Truncated data at boundary between constant pool entries */
    private static int[] boundaryTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
       /* SNIP */
    };

    /* INTERNAL - classparser[9] */
    /* Truncated data in middle of integer constant */
    private static int[] intTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
            0x01, 0x00, 0x01, 0x44, /* "D" */
            0x01, 0x00, 0x01, 0x73, /* "s" */
            0x01, 0x00, 0x12, /* "Ljava/lang/String;" */
                0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 
                0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72, 0x69, 0x6e, 
                0x67, 0x3b,
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, 0x43, 0x6f, 0x64, 0x65, /* "Code" */
            0x01, 0x00, 0x0f, /* "LineNumberTable" */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x08, /* "<clinit>" */
                0x3c, 0x63, 0x6c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x19, 0x00, 0x1a, /* nameandtype */
            0x0c, 0x00, 0x0f, 0x00, 0x10, /* nameandtype */
            0x0c, 0x00, 0x11, 0x00, 0x12, /* nameandtype */
            0x0c, 0x00, 0x13, 0x00, 0x14, /* nameandtype */
            0x0c, 0x00, 0x15, 0x00, 0x16, /* nameandtype */
            0x01, 0x00, 0x07, /* "Testing" */
                0x54, 0x65, 0x73, 0x74, 0x69, 0x6e, 0x67, 
            0x0c, 0x00, 0x17, 0x00, 0x18, /* nameandtype */
            0x01, 0x00, 0x02, 0x63, 0x70, /* "cp" */
            0x01, 0x00, 0x10, /* "java/lang/Object" */
                0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 
                0x67, 0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 
            0x03, 0x00, /* ERROR - integer truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in middle of float constant */
    private static int[] floatTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, /* ERROR - float truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in middle of UTF-8 constant length */
    private static int[] utfLengthTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, /* ERROR - UTF-8 length data truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[10] */
    /* Truncated data in middle of UTF-8 constant data */
    private static int[] utfDataTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
            0x01, 0x00, 0x01, 0x44, /* "D" */
            0x01, 0x00, 0x01, 0x73, /* "s" */
            0x01, 0x00, 0x12, /* "Ljava/la" ERROR - truncated */
                0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of long constant */
    private static int[] longATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x05, 0x00, 0x00, /* ERROR - long value truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of long constant */
    private static int[] longBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, /* ERROR - long truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of double constant */
    private static int[] doubleATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, /* ERROR - truncated double */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of double constant */
    private static int[] doubleBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 
                  0xe6, 0x3c, 0x53, /* ERROR - truncated double */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of name/type constant */
    private static int[] nameTypeATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
            0x01, 0x00, 0x01, 0x44, /* "D" */
            0x01, 0x00, 0x01, 0x73, /* "s" */
            0x01, 0x00, 0x12, /* "Ljava/lang/String;" */
                0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 
                0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72, 0x69, 0x6e, 
                0x67, 0x3b,
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, 0x43, 0x6f, 0x64, 0x65, /* "Code" */
            0x01, 0x00, 0x0f, /* "LineNumberTable" */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x08, /* "<clinit>" */
                0x3c, 0x63, 0x6c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x19, 0x00, 0x1a, /* nameandtype */
            0x0c, 0x00, 0x0f, 0x00, 0x10, /* nameandtype */
            0x0c, /* ERROR - truncate in first word of nameandtype */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of name/type constant */
    private static int[] nameTypeBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, 0x28, /* class */
            0x01, 0x00, 0x01, 0x69, /* "i" */
            0x01, 0x00, 0x01, 0x49, /* "I" */
            0x01, 0x00, 0x01, 0x6c, /* "l" */
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x01, 0x66, /* "f" */
            0x01, 0x00, 0x01, 0x46, /* "F" */
            0x01, 0x00, 0x01, 0x64, /* "d" */
            0x01, 0x00, 0x01, 0x44, /* "D" */
            0x01, 0x00, 0x01, 0x73, /* "s" */
            0x01, 0x00, 0x12, /* "Ljava/lang/String;" */
                0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 
                0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72, 0x69, 0x6e, 
                0x67, 0x3b,
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, 0x43, 0x6f, 0x64, 0x65, /* "Code" */
            0x01, 0x00, 0x0f, /* "LineNumberTable" */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x08, /* "<clinit>" */
                0x3c, 0x63, 0x6c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x19, 0x00, 0x1a, /* nameandtype */
            0x0c, 0x00, 0x0f, 0x00, 0x10, /* nameandtype */
            0x0c, 0x00, 0x11, 0x00, /* ERROR - truncate nameandtype 2nd word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in class constant */
    private static int[] classTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, 0x25, /* string */
            0x09, 0x00, 0x0d, 0x00, 0x26, /* fieldref */
            0x07, 0x00, 0x27, /* class */
            0x07, 0x00, /* ERROR - class data truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of fieldref constant */
    private static int[] fieldRefATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x09, 0x00, /* ERROR - fieldref truncated 1st word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of fieldref constant */
    private static int[] fieldRefBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x09, 0x00, 0x0d, 0x00, /* ERROR - fieldref truncated 1st word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of methodref constant */
    private static int[] methodRefATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x0a, /* ERROR - methodref truncated 1st word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of methodref constant */
    private static int[] methodRefBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x0a, 0x00, 0x0e, 0x00, /* ERROR - methodref truncated 2nd word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in first word of interfacemethodref constant */
    private static int[] ifMethodRefATruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x0b, 0x00, /* ERROR - ifmethodref truncated 1st word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in second word of interfacemethodref constant */
    private static int[] ifMethodRefBTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x0b, 0x00, 0x0e, 0x00, /* ERROR - ifmethodref truncated 2nd word */
       /* SNIP */
    };

    /* INTERNAL - classparser[9*] */
    /* Truncated data in middle of string constant */
    private static int[] stringTruncateBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x2a, 
            0x0a, 0x00, 0x0e, 0x00, 0x20, /* methodref */
            0x09, 0x00, 0x0d, 0x00, 0x21, /* fieldref */
            0x05, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5b, 0xcd, 0x15, /* long */
            0x09, 0x00, 0x0d, 0x00, 0x22, /* fieldref */
            0x04, 0x3f, 0x99, 0x99, 0x9a, /* float */
            0x09, 0x00, 0x0d, 0x00, 0x23, /* fieldref */
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* double */
            0x09, 0x00, 0x0d, 0x00, 0x24, /* fieldref */
            0x08, 0x00, /* ERROR - string value truncated */
       /* SNIP */
    };

    /* INTERNAL - classparser[12*] */
    /* Special case where long overlaps constant pool boundary */
    private static int[] longOverflowBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x14, 
            0x0a, 0x00, 0x06, 0x00, 0x0f, /* methodref */
            0x05, 0x00, 0x00, 0x70, 0x48, 0x86, 0x0d, 0xdf, 0x79, /* long */
            0x09, 0x00, 0x05, 0x00, 0x10, /* fieldref */
            0x07, 0x00, 0x11, /* class */
            0x07, 0x00, 0x12, /* class */
            0x01, 0x00, 0x05, /* "field" */
                0x66, 0x69, 0x65, 0x6c, 0x64, 
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e,
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, /* "Code" */
                0x43, 0x6f, 0x64, 0x65, 
            0x01, 0x00, 0x0f, /* LineNumberTable */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62,
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x09, 0x00, 0x0a, /* nameandtype */
            0x0c, 0x00, 0x07, 0x00, 0x08, /* nameandtype */
            0x01, 0x00, 0x02, 0x63, 0x70, /* "cp" */
            0x01, 0x00, 0x10, /* "java/lang/Object" */
                0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e,
                0x67, 0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74,
            0x05, 0x00, 0x00, 0x70, 0x48, 0x86, 0x0d, 0xdf, 0x79, /* ERROR */
        0x00, 0x21, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x07, 0x00, 0x08,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x09,
        0x00, 0x0a, 0x00, 0x01, 0x00, 0x0b, 0x00, 0x00,
        0x00, 0x2c, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x0c, 0x2a, 0xb7, 0x00, 0x01, 0x2a, 0x14,
        0x00, 0x02, 0xb5, 0x00, 0x04, 0xb1, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0e,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
        0x00, 0x02, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x0d, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0e
    };

    /* INTERNAL - classparser[12*] */
    /* Special case where double overlaps constant pool boundary */
    private static int[] doubleOverflowBytes = {
        /* Header */
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        /* Constant Pool */
        0x00, 0x14, 
            0x0a, 0x00, 0x06, 0x00, 0x0f, /* methodref */
            0x05, 0x00, 0x00, 0x70, 0x48, 0x86, 0x0d, 0xdf, 0x79, /* long */
            0x09, 0x00, 0x05, 0x00, 0x10, /* fieldref */
            0x07, 0x00, 0x11, /* class */
            0x07, 0x00, 0x12, /* class */
            0x01, 0x00, 0x05, /* "field" */
                0x66, 0x69, 0x65, 0x6c, 0x64, 
            0x01, 0x00, 0x01, 0x4a, /* "J" */
            0x01, 0x00, 0x06, /* "<init>" */
                0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e,
            0x01, 0x00, 0x03, 0x28, 0x29, 0x56, /* "()V" */
            0x01, 0x00, 0x04, /* "Code" */
                0x43, 0x6f, 0x64, 0x65, 
            0x01, 0x00, 0x0f, /* LineNumberTable */
                0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62,
                0x65, 0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 
            0x01, 0x00, 0x0a, /* "SourceFile" */
                0x53, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 
                0x6c, 0x65, 
            0x01, 0x00, 0x07, /* "cp.java" */
                0x63, 0x70, 0x2e, 0x6a, 0x61, 0x76, 0x61, 
            0x0c, 0x00, 0x09, 0x00, 0x0a, /* nameandtype */
            0x0c, 0x00, 0x07, 0x00, 0x08, /* nameandtype */
            0x01, 0x00, 0x02, 0x63, 0x70, /* "cp" */
            0x01, 0x00, 0x10, /* "java/lang/Object" */
                0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e,
                0x67, 0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74,
            0x06, 0x40, 0xc8, 0x1c, 0xd6, 0xe6, 0x3c, 0x53, 0xd7, /* ERROR */
        0x00, 0x21, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x07, 0x00, 0x08,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x09,
        0x00, 0x0a, 0x00, 0x01, 0x00, 0x0b, 0x00, 0x00,
        0x00, 0x2c, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00,
        0x00, 0x0c, 0x2a, 0xb7, 0x00, 0x01, 0x2a, 0x14,
        0x00, 0x02, 0xb5, 0x00, 0x04, 0xb1, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x0e,
        0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x04,
        0x00, 0x02, 0x00, 0x0b, 0x00, 0x01, 0x00, 0x01,
        0x00, 0x0d, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0e
    };

    /**
     * Locally run through the test cases.
     */
    public static void main(String argv[]) {
        /* Use an instance of myself for testing */
        TruncatedConstantPool tstInstance = new TruncatedConstantPool();

        /* Constant pool truncated on an entry boundary */
        try {
            tstInstance.localDefineClass("cp", boundaryTruncateBytes);
            throw new Error("Boundary truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within integer value */
        try {
            tstInstance.localDefineClass("cp", intTruncateBytes);
            throw new Error("Integer truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within float value */
        try {
            tstInstance.localDefineClass("cp", floatTruncateBytes);
            throw new Error("Float truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within UTF-8 length */ 
        try {
            tstInstance.localDefineClass("cp", utfLengthTruncateBytes);
            throw new Error("UTF length truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within UTF-8 data */ 
        try {
            tstInstance.localDefineClass("cp", utfDataTruncateBytes);
            throw new Error("UTF data truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within long data */ 
        try {
            tstInstance.localDefineClass("cp", longATruncateBytes);
            throw new Error("FW long truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", longBTruncateBytes);
            throw new Error("SW long truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within double data */ 
        try {
            tstInstance.localDefineClass("cp", doubleATruncateBytes);
            throw new Error("FW double truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", doubleBTruncateBytes);
            throw new Error("SW double truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within nameandtype data */ 
        try {
            tstInstance.localDefineClass("cp", nameTypeATruncateBytes);
            throw new Error("FW name/type truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", nameTypeBTruncateBytes);
            throw new Error("SW name/type truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within class data */ 
        try {
            tstInstance.localDefineClass("cp", classTruncateBytes);
            throw new Error("Class truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within fieldref data */ 
        try {
            tstInstance.localDefineClass("cp", fieldRefATruncateBytes);
            throw new Error("FW fieldref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", fieldRefBTruncateBytes);
            throw new Error("FW fieldref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within methodref data */ 
        try {
            tstInstance.localDefineClass("cp", methodRefATruncateBytes);
            throw new Error("FW methodref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", methodRefBTruncateBytes);
            throw new Error("SW methodref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within interface method ref data */ 
        try {
            tstInstance.localDefineClass("cp", ifMethodRefATruncateBytes);
            throw new Error("FW ifmethodref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", ifMethodRefBTruncateBytes);
            throw new Error("SW ifmethodref truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Constant pool truncated within string data */ 
        try {
            tstInstance.localDefineClass("cp", stringTruncateBytes);
            throw new Error("String truncated constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* Long and double overflows of constant pool */
        try {
            tstInstance.localDefineClass("cp", longOverflowBytes);
            throw new Error("Long overflow constant pool parsed");
        } catch (ClassFormatError ex) {}
        try {
            tstInstance.localDefineClass("cp", doubleOverflowBytes);
            throw new Error("Double overflow constant pool parsed");
        } catch (ClassFormatError ex) {}

        /* After all the failures, it should be usable */
        tstInstance.localDefineClass("cp", properClassBytes);
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
