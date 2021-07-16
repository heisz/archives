// Mauve-compatible testlet instance for testing java.lang.reflect.Field.
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

package wrdg.mauve.java.lang.reflect.Field;

import gnu.testlet.Testlet;
import gnu.testlet.TestHarness;
import java.lang.reflect.Field;
import java.lang.reflect.Modifier;
import wrdg.mauve.java.lang.reflect.FullAccess;

public class field implements Testlet {
    public void testFullAccess(TestHarness harness) 
        throws IllegalArgumentException, IllegalAccessException {
        /* Note: Field access control tests are in Class */
        Class tstClass = FullAccess.class;
        FullAccess tst = new FullAccess();

        Field fields[] = tstClass.getDeclaredFields();
        harness.check(fields.length == 22,
                      "Class::getDeclaredFields() - incorrect field count");

        for (int i = 0; i < fields.length; i++) {
            if (fields[i].getName().equals("booleanField")) {
                harness.check(fields[i].getModifiers() == Modifier.PUBLIC,
                              "Field::getModifiers() - boolean");

                try {
                    Object o = fields[i].get(null);
                    harness.check(false,
                                  "Field::get() null successful");
                } catch (NullPointerException ex) {
                    harness.check(true, "Expected result");
                }
                try {
                    boolean b = fields[i].getBoolean(null);
                    harness.check(false,
                                  "Field::getBoolean() null successful");
                } catch (NullPointerException ex) {
                    harness.check(true, "Expected result");
                }
                try {
                    fields[i].set(null, new Object());
                    harness.check(false,
                                  "Field::set() null successful");
                } catch (NullPointerException ex) {
                    harness.check(true, "Expected result");
                }
                try {
                    fields[i].setBoolean(null, false);
                    harness.check(false,
                                  "Field::setBoolean() null successful");
                } catch (NullPointerException ex) {
                    harness.check(true, "Expected result");
                }

                /* Add generic get/set when associated class available */
                
                harness.check(!fields[i].getBoolean(tst),
                              "Field::getBoolean(instance) incorrect");
                fields[i].setBoolean(tst, true);
                harness.check(fields[i].getBoolean(tst),
                              "Field::setBoolean(instance) incorrect");

                /* No widening cases for boolean */
                try {
                    fields[i].setInt(tst, 12);
                    harness.check(false,
                                  "Field::setInt() ok on boolean?");
                } catch (IllegalArgumentException ex) {
                    harness.check(true, "Expected result");
                }
                try {
                    harness.check(fields[i].getInt(tst) == 1,
                                  "Field::getInt() on boolean?");
                } catch (IllegalArgumentException ex) {
                    harness.check(true, "Expected result");
                }
            }
        }
    }

    public void test(TestHarness harness) {
        try {
            testFullAccess(harness);
        } catch (IllegalArgumentException ex) {
            harness.check(false, "Unexpected exception " + ex);
        } catch (IllegalAccessException ex) {
            harness.check(false, "Unexpected exception " + ex);
        }
    }
}
