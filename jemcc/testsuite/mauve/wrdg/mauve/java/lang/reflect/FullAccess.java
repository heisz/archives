// Fully accessible test class for use with reflection testlets.
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

package wrdg.mauve.java.lang.reflect;

public class FullAccess {
    public boolean booleanField;
    public byte byteField;
    public transient char charField;
    public double doubleField;
    public float floatField;
    public int intField;
    public volatile long longField;
    public short shortField;
    public Object objectField;
    public Class classField;
    public int[] arrayField;

    public static boolean booleanStaticField = true;
    public static byte byteStaticField = 12;
    public static char charStaticField = 24;
    public static double doubleStaticField = 111111.0;
    public static float floatStaticField = (float) 111.0;
    public static int intStaticField = 16;
    public static long longStaticField = 12345;
    public static short shortStaticField = 123;
    public static Object objectStaticField = null;
    public static Class classStaticField = null;
    public static int[] arrayStaticField = new int[10];

    public FullAccess() {
        booleanField = false;
        byteField = 1;
        charField = 'a';
        doubleField = 54.0;
        floatField = (float) 22.0;
        intField = 99;
        longField = 8435345;
        shortField = 0;
        objectField = this;
        classField = null;
        arrayField = new int[6];

        classStaticField = this.getClass();
    }

    public FullAccess(int argi) {
    }

    public void empty() {}

    public int ifni(int i) { return 1; }

    public boolean bfnii(int i, int j) throws Exception { return false; }
}
