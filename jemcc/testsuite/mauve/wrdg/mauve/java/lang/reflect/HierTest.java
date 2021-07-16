// Mauve-compatible testlet instance for testing java.lang.reflect hierarchy.
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

import gnu.testlet.TestHarness;
import gnu.testlet.Testlet;
import java.lang.reflect.*;

public class HierTest implements Testlet {
    protected static TestHarness harness;

    public void test (TestHarness the_harness) {
        Class testClass = null, interfaces[];

        harness = the_harness;

        try {
            testClass = Class.forName("java.lang.reflect.Array");
            harness.check(!testClass.isInterface(),
                          "Error: Array is not an object");
            harness.check(testClass.getSuperclass() == Object.class,
                          "Error: Array super not Object");
            interfaces = testClass.getInterfaces();
            harness.check(interfaces.length == 0,
                          "Error: Array has interfaces");

            testClass = Class.forName("java.lang.reflect.Constructor");
            harness.check(!testClass.isInterface(),
                          "Error: Constructor is not an object");
            harness.check(testClass.getSuperclass() == Object.class,
                          "Error: Constructor super not Object");
            interfaces = testClass.getInterfaces();
            harness.check((interfaces.length == 1) &&
                                       (interfaces[0] == Member.class),
                          "Error: Constructor is not Member");

            testClass = Class.forName("java.lang.reflect.Field");
            harness.check(!testClass.isInterface(),
                          "Error: Field is not an object");
            harness.check(testClass.getSuperclass() == Object.class,
                          "Error: Field super not Object");
            interfaces = testClass.getInterfaces();
            harness.check((interfaces.length == 1) &&
                                       (interfaces[0] == Member.class),
                          "Error: Field is not Member");

            testClass = Class.forName(
                                "java.lang.reflect.InvocationTargetException");
            harness.check(!testClass.isInterface(),
                          "Error: InvTargExc is not an object");
            harness.check(testClass.getSuperclass() == Exception.class,
                          "Error: InvTargExc super not Exception");
            interfaces = testClass.getInterfaces();
            harness.check(interfaces.length == 0,
                          "Error: InvTargExc has interfaces");

            testClass = Class.forName("java.lang.reflect.Member");
            harness.check(testClass.isInterface(),
                          "Error: Member is not an interface");
            harness.check(testClass.getSuperclass() == null,
                          "Error: Member super not null");

            testClass = Class.forName("java.lang.reflect.Method");
            harness.check(!testClass.isInterface(),
                          "Error: Method is not an object");
            harness.check(testClass.getSuperclass() == Object.class,
                          "Error: Method super not Object");
            interfaces = testClass.getInterfaces();
            harness.check((interfaces.length == 1) &&
                                       (interfaces[0] == Member.class),
                          "Error: Method is not Member");

            testClass = Class.forName("java.lang.reflect.Modifier");
            harness.check(!testClass.isInterface(),
                          "Error: Modifier is not an object");
            harness.check(testClass.getSuperclass() == Object.class,
                          "Error: Modifier super not Object");
            interfaces = testClass.getInterfaces();
            harness.check(interfaces.length == 0,
                          "Error: Modifier has interfaces");
        } catch (ClassNotFoundException ex) {
            harness.fail("Error: class was not found " + ex);
        }
    }
}
