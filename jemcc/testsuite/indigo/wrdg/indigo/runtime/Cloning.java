//
// Indigo VM testsuite for testing object cloning operations.
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

package wrdg.indigo.runtime;

/**
 * <P>
 * Indigo test program to test object cloning operations.
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
 * @version $Revision: 1.2 $
 */

/** NOTE: the appear for documentation only (binary in main class) **/

/*
public class can implements Cloneable, Runnable {
    public int intField;
    public float fltField;
    public Object objField;

    public can() {
        intField = 12;
        fltField = (float) 24.0;
        objField = (Object) this;
    }

    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    public void run() {
        try {
            can o = (can) this.clone();
            if (o == this) {
                throw new Error("Clone returned original object");
            }
            if (o.intField != this.intField) {
                throw new Error("Clone shallow copy failed (int)");
            }
            if (o.fltField != this.fltField) {
                throw new Error("Clone shallow copy failed (float)");
            }
            if (o.objField != this.objField) {
                throw new Error("Clone shallow copy failed (object)");
            }
        } catch (CloneNotSupportedException ex) {
            throw new Error("Clone failed unexpectedly");
        }
    }
}

public class cant implements Runnable {
    public int intField;
    public float fltField;

    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    public void run() {
        try {
            Object o = this.clone();
            throw new Error("Clone successful on non-Cloneable");
        } catch (CloneNotSupportedException ex) {
        }
    }
}

*/

public class Cloning extends ClassLoader {
    /* Binary representations of the above classes (made public) */
    private static int[] canBytes = {
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        0x00, 0x36, 0x0a, 0x00, 0x11, 0x00, 0x24, 0x09,
        0x00, 0x08, 0x00, 0x25, 0x04, 0x41, 0xc0, 0x00,
        0x00, 0x09, 0x00, 0x08, 0x00, 0x26, 0x09, 0x00,
        0x08, 0x00, 0x27, 0x0a, 0x00, 0x11, 0x00, 0x28,
        0x0a, 0x00, 0x08, 0x00, 0x28, 0x07, 0x00, 0x29,
        0x07, 0x00, 0x2a, 0x08, 0x00, 0x2b, 0x0a, 0x00,
        0x09, 0x00, 0x2c, 0x08, 0x00, 0x2d, 0x08, 0x00,
        0x2e, 0x08, 0x00, 0x2f, 0x07, 0x00, 0x30, 0x08,
        0x00, 0x31, 0x07, 0x00, 0x32, 0x07, 0x00, 0x33,
        0x07, 0x00, 0x34, 0x01, 0x00, 0x08, 0x69, 0x6e,
        0x74, 0x46, 0x69, 0x65, 0x6c, 0x64, 0x01, 0x00,
        0x01, 0x49, 0x01, 0x00, 0x08, 0x66, 0x6c, 0x74,
        0x46, 0x69, 0x65, 0x6c, 0x64, 0x01, 0x00, 0x01,
        0x46, 0x01, 0x00, 0x08, 0x6f, 0x62, 0x6a, 0x46,
        0x69, 0x65, 0x6c, 0x64, 0x01, 0x00, 0x12, 0x4c,
        0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e,
        0x67, 0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74,
        0x3b, 0x01, 0x00, 0x06, 0x3c, 0x69, 0x6e, 0x69,
        0x74, 0x3e, 0x01, 0x00, 0x03, 0x28, 0x29, 0x56,
        0x01, 0x00, 0x04, 0x43, 0x6f, 0x64, 0x65, 0x01,
        0x00, 0x0f, 0x4c, 0x69, 0x6e, 0x65, 0x4e, 0x75,
        0x6d, 0x62, 0x65, 0x72, 0x54, 0x61, 0x62, 0x6c,
        0x65, 0x01, 0x00, 0x05, 0x63, 0x6c, 0x6f, 0x6e,
        0x65, 0x01, 0x00, 0x14, 0x28, 0x29, 0x4c, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x3b,
        0x01, 0x00, 0x0a, 0x45, 0x78, 0x63, 0x65, 0x70,
        0x74, 0x69, 0x6f, 0x6e, 0x73, 0x01, 0x00, 0x03,
        0x72, 0x75, 0x6e, 0x01, 0x00, 0x0a, 0x53, 0x6f,
        0x75, 0x72, 0x63, 0x65, 0x46, 0x69, 0x6c, 0x65,
        0x01, 0x00, 0x08, 0x63, 0x61, 0x6e, 0x2e, 0x6a,
        0x61, 0x76, 0x61, 0x0c, 0x00, 0x1a, 0x00, 0x1b,
        0x0c, 0x00, 0x14, 0x00, 0x15, 0x0c, 0x00, 0x16,
        0x00, 0x17, 0x0c, 0x00, 0x18, 0x00, 0x19, 0x0c,
        0x00, 0x1e, 0x00, 0x1f, 0x01, 0x00, 0x03, 0x63,
        0x61, 0x6e, 0x01, 0x00, 0x0f, 0x6a, 0x61, 0x76,
        0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x45,
        0x72, 0x72, 0x6f, 0x72, 0x01, 0x00, 0x1e, 0x43,
        0x6c, 0x6f, 0x6e, 0x65, 0x20, 0x72, 0x65, 0x74,
        0x75, 0x72, 0x6e, 0x65, 0x64, 0x20, 0x6f, 0x72,
        0x69, 0x67, 0x69, 0x6e, 0x61, 0x6c, 0x20, 0x6f,
        0x62, 0x6a, 0x65, 0x63, 0x74, 0x0c, 0x00, 0x1a,
        0x00, 0x35, 0x01, 0x00, 0x1f, 0x43, 0x6c, 0x6f,
        0x6e, 0x65, 0x20, 0x73, 0x68, 0x61, 0x6c, 0x6c,
        0x6f, 0x77, 0x20, 0x63, 0x6f, 0x70, 0x79, 0x20,
        0x66, 0x61, 0x69, 0x6c, 0x65, 0x64, 0x20, 0x28,
        0x69, 0x6e, 0x74, 0x29, 0x01, 0x00, 0x21, 0x43,
        0x6c, 0x6f, 0x6e, 0x65, 0x20, 0x73, 0x68, 0x61,
        0x6c, 0x6c, 0x6f, 0x77, 0x20, 0x63, 0x6f, 0x70,
        0x79, 0x20, 0x66, 0x61, 0x69, 0x6c, 0x65, 0x64,
        0x20, 0x28, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x29,
        0x01, 0x00, 0x22, 0x43, 0x6c, 0x6f, 0x6e, 0x65,
        0x20, 0x73, 0x68, 0x61, 0x6c, 0x6c, 0x6f, 0x77,
        0x20, 0x63, 0x6f, 0x70, 0x79, 0x20, 0x66, 0x61,
        0x69, 0x6c, 0x65, 0x64, 0x20, 0x28, 0x6f, 0x62,
        0x6a, 0x65, 0x63, 0x74, 0x29, 0x01, 0x00, 0x24,
        0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e,
        0x67, 0x2f, 0x43, 0x6c, 0x6f, 0x6e, 0x65, 0x4e,
        0x6f, 0x74, 0x53, 0x75, 0x70, 0x70, 0x6f, 0x72,
        0x74, 0x65, 0x64, 0x45, 0x78, 0x63, 0x65, 0x70,
        0x74, 0x69, 0x6f, 0x6e, 0x01, 0x00, 0x19, 0x43,
        0x6c, 0x6f, 0x6e, 0x65, 0x20, 0x66, 0x61, 0x69,
        0x6c, 0x65, 0x64, 0x20, 0x75, 0x6e, 0x65, 0x78,
        0x70, 0x65, 0x63, 0x74, 0x65, 0x64, 0x6c, 0x79,
        0x01, 0x00, 0x10, 0x6a, 0x61, 0x76, 0x61, 0x2f,
        0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x4f, 0x62, 0x6a,
        0x65, 0x63, 0x74, 0x01, 0x00, 0x13, 0x6a, 0x61,
        0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x2f,
        0x43, 0x6c, 0x6f, 0x6e, 0x65, 0x61, 0x62, 0x6c,
        0x65, 0x01, 0x00, 0x12, 0x6a, 0x61, 0x76, 0x61,
        0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x52, 0x75,
        0x6e, 0x6e, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00,
        0x15, 0x28, 0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f,
        0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72,
        0x69, 0x6e, 0x67, 0x3b, 0x29, 0x56, 0x00, 0x21,
        0x00, 0x08, 0x00, 0x11, 0x00, 0x02, 0x00, 0x12,
        0x00, 0x13, 0x00, 0x03, 0x00, 0x01, 0x00, 0x14,
        0x00, 0x15, 0x00, 0x00, 0x00, 0x01, 0x00, 0x16,
        0x00, 0x17, 0x00, 0x00, 0x00, 0x01, 0x00, 0x18,
        0x00, 0x19, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01,
        0x00, 0x1a, 0x00, 0x1b, 0x00, 0x01, 0x00, 0x1c,
        0x00, 0x00, 0x00, 0x3e, 0x00, 0x02, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x16, 0x2a, 0xb7, 0x00, 0x01,
        0x2a, 0x10, 0x0c, 0xb5, 0x00, 0x02, 0x2a, 0x12,
        0x03, 0xb5, 0x00, 0x04, 0x2a, 0x2a, 0xb5, 0x00,
        0x05, 0xb1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x1d,
        0x00, 0x00, 0x00, 0x16, 0x00, 0x05, 0x00, 0x00,
        0x00, 0x06, 0x00, 0x04, 0x00, 0x07, 0x00, 0x0a,
        0x00, 0x08, 0x00, 0x10, 0x00, 0x09, 0x00, 0x15,
        0x00, 0x0a, 0x00, 0x01, 0x00, 0x1e, 0x00, 0x1f,
        0x00, 0x02, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x1d,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05,
        0x2a, 0xb7, 0x00, 0x06, 0xb0, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x06, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x20, 0x00,
        0x00, 0x00, 0x04, 0x00, 0x01, 0x00, 0x0f, 0x00,
        0x01, 0x00, 0x21, 0x00, 0x1b, 0x00, 0x01, 0x00,
        0x1c, 0x00, 0x00, 0x00, 0xb2, 0x00, 0x03, 0x00,
        0x02, 0x00, 0x00, 0x00, 0x66, 0x2a, 0xb6, 0x00,
        0x07, 0xc0, 0x00, 0x08, 0x4c, 0x2b, 0x2a, 0xa6,
        0x00, 0x0d, 0xbb, 0x00, 0x09, 0x59, 0x12, 0x0a,
        0xb7, 0x00, 0x0b, 0xbf, 0x2b, 0xb4, 0x00, 0x02,
        0x2a, 0xb4, 0x00, 0x02, 0x9f, 0x00, 0x0d, 0xbb,
        0x00, 0x09, 0x59, 0x12, 0x0c, 0xb7, 0x00, 0x0b,
        0xbf, 0x2b, 0xb4, 0x00, 0x04, 0x2a, 0xb4, 0x00,
        0x04, 0x95, 0x99, 0x00, 0x0d, 0xbb, 0x00, 0x09,
        0x59, 0x12, 0x0d, 0xb7, 0x00, 0x0b, 0xbf, 0x2b,
        0xb4, 0x00, 0x05, 0x2a, 0xb4, 0x00, 0x05, 0xa5,
        0x00, 0x0d, 0xbb, 0x00, 0x09, 0x59, 0x12, 0x0e,
        0xb7, 0x00, 0x0b, 0xbf, 0xa7, 0x00, 0x0e, 0x4c,
        0xbb, 0x00, 0x09, 0x59, 0x12, 0x10, 0xb7, 0x00,
        0x0b, 0xbf, 0xb1, 0x00, 0x01, 0x00, 0x00, 0x00,
        0x57, 0x00, 0x5a, 0x00, 0x0f, 0x00, 0x01, 0x00,
        0x1d, 0x00, 0x00, 0x00, 0x32, 0x00, 0x0c, 0x00,
        0x00, 0x00, 0x12, 0x00, 0x08, 0x00, 0x13, 0x00,
        0x0d, 0x00, 0x14, 0x00, 0x17, 0x00, 0x16, 0x00,
        0x22, 0x00, 0x17, 0x00, 0x2c, 0x00, 0x19, 0x00,
        0x38, 0x00, 0x1a, 0x00, 0x42, 0x00, 0x1c, 0x00,
        0x4d, 0x00, 0x1d, 0x00, 0x57, 0x00, 0x1f, 0x00,
        0x5a, 0x00, 0x20, 0x00, 0x65, 0x00, 0x22, 0x00,
        0x01, 0x00, 0x22, 0x00, 0x00, 0x00, 0x02, 0x00,
        0x23
    };

    private static int[] cantBytes = {
        0xca, 0xfe, 0xba, 0xbe, 0x00, 0x03, 0x00, 0x2d,
        0x00, 0x23, 0x0a, 0x00, 0x09, 0x00, 0x19, 0x0a,
        0x00, 0x09, 0x00, 0x1a, 0x0a, 0x00, 0x08, 0x00,
        0x1a, 0x07, 0x00, 0x1b, 0x08, 0x00, 0x1c, 0x0a,
        0x00, 0x04, 0x00, 0x1d, 0x07, 0x00, 0x1e, 0x07,
        0x00, 0x1f, 0x07, 0x00, 0x20, 0x07, 0x00, 0x21,
        0x01, 0x00, 0x08, 0x69, 0x6e, 0x74, 0x46, 0x69,
        0x65, 0x6c, 0x64, 0x01, 0x00, 0x01, 0x49, 0x01,
        0x00, 0x08, 0x66, 0x6c, 0x74, 0x46, 0x69, 0x65,
        0x6c, 0x64, 0x01, 0x00, 0x01, 0x46, 0x01, 0x00,
        0x06, 0x3c, 0x69, 0x6e, 0x69, 0x74, 0x3e, 0x01,
        0x00, 0x03, 0x28, 0x29, 0x56, 0x01, 0x00, 0x04,
        0x43, 0x6f, 0x64, 0x65, 0x01, 0x00, 0x0f, 0x4c,
        0x69, 0x6e, 0x65, 0x4e, 0x75, 0x6d, 0x62, 0x65,
        0x72, 0x54, 0x61, 0x62, 0x6c, 0x65, 0x01, 0x00,
        0x05, 0x63, 0x6c, 0x6f, 0x6e, 0x65, 0x01, 0x00,
        0x14, 0x28, 0x29, 0x4c, 0x6a, 0x61, 0x76, 0x61,
        0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x2f, 0x4f, 0x62,
        0x6a, 0x65, 0x63, 0x74, 0x3b, 0x01, 0x00, 0x0a,
        0x45, 0x78, 0x63, 0x65, 0x70, 0x74, 0x69, 0x6f,
        0x6e, 0x73, 0x01, 0x00, 0x03, 0x72, 0x75, 0x6e,
        0x01, 0x00, 0x0a, 0x53, 0x6f, 0x75, 0x72, 0x63,
        0x65, 0x46, 0x69, 0x6c, 0x65, 0x01, 0x00, 0x09,
        0x63, 0x61, 0x6e, 0x74, 0x2e, 0x6a, 0x61, 0x76,
        0x61, 0x0c, 0x00, 0x0f, 0x00, 0x10, 0x0c, 0x00,
        0x13, 0x00, 0x14, 0x01, 0x00, 0x0f, 0x6a, 0x61,
        0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67, 0x2f,
        0x45, 0x72, 0x72, 0x6f, 0x72, 0x01, 0x00, 0x21,
        0x43, 0x6c, 0x6f, 0x6e, 0x65, 0x20, 0x73, 0x75,
        0x63, 0x63, 0x65, 0x73, 0x73, 0x66, 0x75, 0x6c,
        0x20, 0x6f, 0x6e, 0x20, 0x6e, 0x6f, 0x6e, 0x2d,
        0x43, 0x6c, 0x6f, 0x6e, 0x65, 0x61, 0x62, 0x6c,
        0x65, 0x0c, 0x00, 0x0f, 0x00, 0x22, 0x01, 0x00,
        0x24, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61,
        0x6e, 0x67, 0x2f, 0x43, 0x6c, 0x6f, 0x6e, 0x65,
        0x4e, 0x6f, 0x74, 0x53, 0x75, 0x70, 0x70, 0x6f,
        0x72, 0x74, 0x65, 0x64, 0x45, 0x78, 0x63, 0x65,
        0x70, 0x74, 0x69, 0x6f, 0x6e, 0x01, 0x00, 0x04,
        0x63, 0x61, 0x6e, 0x74, 0x01, 0x00, 0x10, 0x6a,
        0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61, 0x6e, 0x67,
        0x2f, 0x4f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x01,
        0x00, 0x12, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c,
        0x61, 0x6e, 0x67, 0x2f, 0x52, 0x75, 0x6e, 0x6e,
        0x61, 0x62, 0x6c, 0x65, 0x01, 0x00, 0x15, 0x28,
        0x4c, 0x6a, 0x61, 0x76, 0x61, 0x2f, 0x6c, 0x61,
        0x6e, 0x67, 0x2f, 0x53, 0x74, 0x72, 0x69, 0x6e,
        0x67, 0x3b, 0x29, 0x56, 0x00, 0x21, 0x00, 0x08,
        0x00, 0x09, 0x00, 0x01, 0x00, 0x0a, 0x00, 0x02,
        0x00, 0x01, 0x00, 0x0b, 0x00, 0x0c, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x0d, 0x00, 0x0e, 0x00, 0x00,
        0x00, 0x03, 0x00, 0x01, 0x00, 0x0f, 0x00, 0x10,
        0x00, 0x01, 0x00, 0x11, 0x00, 0x00, 0x00, 0x1d,
        0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x05,
        0x2a, 0xb7, 0x00, 0x01, 0xb1, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x12, 0x00, 0x00, 0x00, 0x06, 0x00,
        0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x13, 0x00, 0x14, 0x00, 0x02, 0x00, 0x11, 0x00,
        0x00, 0x00, 0x1d, 0x00, 0x01, 0x00, 0x01, 0x00,
        0x00, 0x00, 0x05, 0x2a, 0xb7, 0x00, 0x02, 0xb0,
        0x00, 0x00, 0x00, 0x01, 0x00, 0x12, 0x00, 0x00,
        0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00, 0x06,
        0x00, 0x15, 0x00, 0x00, 0x00, 0x04, 0x00, 0x01,
        0x00, 0x07, 0x00, 0x01, 0x00, 0x16, 0x00, 0x10,
        0x00, 0x01, 0x00, 0x11, 0x00, 0x00, 0x00, 0x3d,
        0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0x11,
        0x2a, 0xb6, 0x00, 0x03, 0x4c, 0xbb, 0x00, 0x04,
        0x59, 0x12, 0x05, 0xb7, 0x00, 0x06, 0xbf, 0x4c,
        0xb1, 0x00, 0x01, 0x00, 0x00, 0x00, 0x0f, 0x00,
        0x0f, 0x00, 0x07, 0x00, 0x01, 0x00, 0x12, 0x00,
        0x00, 0x00, 0x12, 0x00, 0x04, 0x00, 0x00, 0x00,
        0x0b, 0x00, 0x05, 0x00, 0x0c, 0x00, 0x0f, 0x00,
        0x0d, 0x00, 0x10, 0x00, 0x10, 0x00, 0x01, 0x00,
        0x17, 0x00, 0x00, 0x00, 0x02, 0x00, 0x18
    };

    /**
     * Locally run through the test cases.
     */
    public static void main(String argv[]) {
        Cloning tstInstance = new Cloning();

        /* First, try the non-cloneable case */
        Class cantClass = tstInstance.localDefineClass("cant", cantBytes);
        try {
            Runnable inst = (Runnable) cantClass.newInstance();
            inst.run();
        } catch (Exception ex) {
            throw new Error("Unexpected exception: " + ex);
        }

        /* Then, validate the basic shallow object clone */
        Class canClass = tstInstance.localDefineClass("can", canBytes);
        try {
            Runnable inst = (Runnable) canClass.newInstance();
            inst.run();
        } catch (Exception ex) {
            throw new Error("Unexpected exception: " + ex);
        }

        /* Arrays */
        boolean[] boolArr = new boolean[0];
        boolean[] boolComp = (boolean[]) boolArr.clone();
        if (boolComp.length != 0) {
            throw new Error("Failure on empty array clone");
        }

        byte[] byteArr = new byte[2];
        byteArr[0] = 1;
        byteArr[1] = 12;
        byte[] byteComp = (byte[]) byteArr.clone();
        byteArr[0] = byteArr[1] = 6;
        if ((byteComp.length != 2) ||
                (byteComp[0] != 1) || (byteComp[1] != 12)) {
            throw new Error("Failure on byte array clone");
        }

        char[] charArr = new char[10];
        charArr[0] = 'a';
        charArr[9] = 'b';
        char[] charComp = (char[]) charArr.clone();
        charArr[0] = charArr[9] = 'z';
        if ((charComp.length != 10) ||
                (charComp[0] != 'a') || (charComp[9] != 'b')) {
            throw new Error("Failure on char array clone");
        }

        int[] intArr = new int[1];
        intArr[0] = -101;
        int[] intComp = (int[]) intArr.clone();
        intArr[0] = 999;
        if ((intComp.length != 1) || (intComp[0] != -101)) {
            throw new Error("Failure on int array clone");
        }

        double[] doubleArr = new double[6];
        doubleArr[0] = 1.0;
        doubleArr[5] = 12.0;
        double[] doubleComp = (double[]) doubleArr.clone();
        if ((doubleComp.length != 6) ||
                (doubleComp[0] != doubleArr[0]) ||
                    (doubleComp[5] != doubleArr[5])) {
            throw new Error("Failure on double array clone");
        }

        Object[] objectArr = new Object[2];
        objectArr[0] = null;
        objectArr[1] = tstInstance;
        Object[] objectComp = (Object[]) objectArr.clone();
        objectArr[0] = objectArr[1] = objectArr;
        if ((objectComp.length != 2) ||
                (objectComp[0] != null) || (objectComp[1] != tstInstance)) {
            throw new Error("Failure on object array clone");
        }

        int[][] intintArr = new int[2][];
        intintArr[0] = new int[6];
        intintArr[1] = new int[12];
        int[][] intintComp = (int[][]) intintArr.clone();
        if ((intintComp.length != 2) ||
                (intintComp[0] != intintArr[0]) || 
                    (intintComp[1] != intintArr[1])) {
            throw new Error("Failure on multi-int array clone");
        }
    }

    /**
     * Local method to wrap class definition with integer byte arrays.
     */
    public Class localDefineClass(String className, int[] byteData) {
        byte[] dataSet = new byte[2048];

        for (int i = 0; i < byteData.length; i++) {
            dataSet[i] = (byte) byteData[i];
        }
        return defineClass(className, dataSet, 0, byteData.length);
    }

    /**
     * Method to provide support for JDK1.1 compilers.
     */
    public Class loadClass(String className, boolean resolve)
            throws ClassNotFoundException {
        return Class.forName(className);
    }
}