// Mauve-compatible testlet instance for testing java.lang.reflect.Modifier.
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

package wrdg.mauve.java.lang.reflect.Modifier;

import gnu.testlet.Testlet;
import gnu.testlet.TestHarness;
import java.lang.reflect.Modifier;

public class modifier implements Testlet {
    public void test(TestHarness harness) {
        int tstMod = Modifier.PUBLIC | Modifier.STATIC | Modifier.SYNCHRONIZED |
                     Modifier.NATIVE | Modifier.FINAL;

        harness.check(!Modifier.isAbstract(tstMod),
                      "Modifier::isAbstract() method failed");
        harness.check(Modifier.isFinal(tstMod),
                      "Modifier::isFinal() method failed");
        harness.check(!Modifier.isInterface(tstMod),
                      "Modifier::isInterface() method failed");
        harness.check(Modifier.isNative(tstMod),
                      "Modifier::isNative() method failed");
        harness.check(!Modifier.isPrivate(tstMod),
                      "Modifier::isPrivate() method failed");
        harness.check(!Modifier.isProtected(tstMod),
                      "Modifier::isProtected() method failed");
        harness.check(Modifier.isPublic(tstMod),
                      "Modifier::isPublic() method failed");
        harness.check(Modifier.isStatic(tstMod),
                      "Modifier::isStatic() method failed");
        harness.check(!Modifier.isTransient(tstMod),
                      "Modifier::isTransient() method failed");
        harness.check(!Modifier.isVolatile(tstMod),
                      "Modifier::isVolatile() method failed");

        harness.check(Modifier.toString(tstMod).equals(
                           "public static final native synchronized"),
                      "Modifier::toString(partial) failed");

        /* Invalid case, but allowed... */
        tstMod = Modifier.ABSTRACT | Modifier.FINAL | Modifier.INTERFACE |
                 Modifier.NATIVE | Modifier.PRIVATE | Modifier.PROTECTED |
                 Modifier.PUBLIC | Modifier.STATIC | Modifier.SYNCHRONIZED |
                 Modifier.TRANSIENT | Modifier.VOLATILE;

        harness.check(Modifier.toString(tstMod).equals(
                         "public private protected abstract static final " +
                           "transient volatile native synchronized interface"),
                      "Modifier::toString(full) failed");
    }
}
