// Mauve-compatible testlet instance for testing java.lang.reflect.Constructor.
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

package wrdg.mauve.java.lang.reflect.Constructor;

import gnu.testlet.Testlet;
import gnu.testlet.TestHarness;
import java.lang.reflect.Constructor;
import java.lang.reflect.Modifier;
import wrdg.mauve.java.lang.reflect.FullAccess;

public class constructor implements Testlet {
    public void testFullAccess(TestHarness harness) 
        throws IllegalArgumentException, IllegalAccessException {
        /* Note: Constructor access control tests are in Class */
        Class tstClass = FullAccess.class;
        FullAccess tst = new FullAccess();

        Constructor ctors[] = tstClass.getDeclaredConstructors();
        harness.check(ctors.length == 2,
                      "Class::getDeclaredConstructors() - wrong ctor count");

        for (int i = 0; i < ctors.length; i++) {
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
