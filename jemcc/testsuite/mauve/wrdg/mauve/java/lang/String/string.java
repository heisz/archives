// Mauve-compatible testlet instance for testing java.lang.String.
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

package wrdg.mauve.java.lang.String;

import gnu.testlet.TestHarness;
import gnu.testlet.Testlet;

public class string implements Testlet {

    public void testInit(TestHarness harness) {
    }

    public void testExtract(TestHarness harness) {
        String sa = "Testing";
        String sb = new String("\u1234\u4321ab");
        String sc = "Testing...1...2...3";
        String sd = "\u1111" + "\u2222\u3333abc";

        try {
            char ch = sa.charAt(-44);
            harness.check(false, "String::charAt(I) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            char ch = sb.charAt(88);
            harness.check(false, "String::charAt(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            char ch = sc.charAt(88);
            harness.check(false, "String::charAt(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sa.charAt(3) == 't'),
                      "String::charAt()... - invalid a result");
        harness.check((sb.charAt(1) == '\u4321'),
                      "String::charAt()... - invalid b result");
        harness.check((sc.charAt(0) == 'T'),
                      "String::charAt()... - invalid c result");
        harness.check((sd.charAt(2) == '\u3333'),
                      "String::charAt()... - invalid d result");

        char copy[] = new char[6];
        try {
            sc.getChars(-1, 5, copy, 0);
            harness.check(false, "String::getChars() - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(1, -1, copy, 0);
            harness.check(false, "String::getChars() - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(63, 64, copy, 0);
            harness.check(false, "String::getChars() - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(1, 64, copy, 0);
            harness.check(false, "String::getChars() - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(5, 1, copy, 0);
            harness.check(false, "String::getChars() - inv index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(0, 4, copy, -1);
            harness.check(false, "String::getChars() - -ve index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(0, 4, copy, 8);
            harness.check(false, "String::getChars() - bad index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(0, 7, copy, 0);
            harness.check(false, "String::getChars() - big index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sc.getChars(0, 2, null, 1);
            harness.check(false, "String::getChars() - null array ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }

        sa.getChars(0, 2, copy, 0);
        sa.getChars(1, 1, copy, 0);
        harness.check(((copy[0] == 'T') && (copy[1] == 'e')),
                      "String::getChars()... - invalid a result");
        sb.getChars(0, 2, copy, 2);
        harness.check(((copy[2] == '\u1234') && (copy[3] == '\u4321')),
                      "String::getChars()... - invalid b result");
        sc.getChars(3, 5, copy, 0);
        harness.check(((copy[0] == 't') && (copy[1] == 'i')),
                      "String::getChars()... - invalid c result");
        sd.getChars(0, 3, copy, 1);
        harness.check(((copy[1] == '\u1111') && (copy[3] == '\u3333')),
                      "String::getChars()... - invalid d result");

        try {
            String s = sc.substring(-1);
            harness.check(false, "String::substring(I) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sc.substring(44);
            harness.check(false, "String::substring(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sa.substring(4).equals("ing"),
                      "String::substring(I)... - invalid a result");
        harness.check(sb.substring(2).equals("ab"),
                      "String::substring(I)... - invalid b result");
        harness.check(sc.substring(0).equals("Testing...1...2...3"),
                      "String::substring(I)... - invalid c result");
        harness.check(sd.substring(1).equals("\u2222\u3333abc"),
                      "String::substring(I)... - invalid d result");

        try {
            String s = sc.substring(-1, 4);
            harness.check(false, "String::substring(II) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sc.substring(2, 88);
            harness.check(false, "String::substring(II) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sc.substring(4, 2);
            harness.check(false, "String::substring(II) - rev index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sa.substring(4, 6).equals("in"),
                      "String::substring(II)... - invalid a result");
        harness.check(sb.substring(2, 4).equals("ab"),
                      "String::substring(II)... - invalid b result");
        harness.check(sc.substring(0, 11).equals("Testing...1"),
                      "String::substring(II)... - invalid c result");
        harness.check(sd.substring(1, 3).equals("\u2222\u3333"),
                      "String::substring(II)... - invalid d result");

        try {
            CharSequence cs = sc.subSequence(-1, 4);
            harness.check(false, "String::subSeq(II) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            CharSequence cs = sc.subSequence(2, 88);
            harness.check(false, "String::subSeq(II) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            CharSequence cs = sc.subSequence(4, 2);
            harness.check(false, "String::subSeq(II) - rev index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sa.subSequence(4, 6).equals("in"),
                      "String::subSequence(II)... - invalid a result");
        harness.check(sb.subSequence(2, 4).equals("ab"),
                      "String::subSequence(II)... - invalid b result");
        harness.check(sc.subSequence(0, 11).equals("Testing...1"),
                      "String::subSequence(II)... - invalid c result");
        harness.check(sd.subSequence(1, 3).equals("\u2222\u3333"),
                      "String::subSequence(II)... - invalid d result");

        String ts = "one";
        char ct[] = ts.toCharArray();
        harness.check(((ct.length == 3) && (ct[0] == 'o') && 
                                    (ct[1] == 'n') && (ct[2] == 'e')),
                      "String::toCharArray()... - invalid result");
        String es = "";
        char ce[] = es.toCharArray();
        harness.check((ce.length == 0),
                      "String::toCharArray()... - invalid empty result");
    }

    public void testLookup(TestHarness harness) {
        String sa = "abcdeeeeeeeeeeeee";
        try {
            int x = sa.indexOf(null);
            harness.check(false, "String::indexOf(null) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        try {
            int x = sa.indexOf(null, 4);
            harness.check(false, "String::indexOf(null, I) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sa.indexOf("") == 0),
                      "String::indexOf(S) - invalid empty result");
        harness.check((sa.indexOf("\u2222\u3333") == -1),
                      "String::indexOf(S) - invalid a-u result");
        harness.check((sa.indexOf("bcdefghijlmnopqrst") == -1),
                      "String::indexOf(S) - invalid overflow result");
        harness.check((sa.indexOf("bcd") == 1),
                      "String::indexOf(S) - invalid a-a result");
        harness.check((sa.indexOf("efg") == -1),
                      "String::indexOf(S) - invalid a-mm result");
        harness.check((sa.indexOf("e") == 4),
                      "String::indexOf(S) - invalid a-single result");
        harness.check((sa.indexOf("cd", -10) == 2),
                      "String::indexOf(S) - invalid a-ve result");
        harness.check((sa.indexOf("eee", 555) == -1),
                      "String::indexOf(S) - invalid a-out result");
        harness.check((sa.indexOf("bcd", 5) == -1),
                      "String::indexOf(S) - invalid a-off result");
        harness.check((sa.indexOf("e", 10) == 10),
                      "String::indexOf(S) - invalid a-singleoff result");

        String sb = "\u1111\u2222ab\u3333\u4444";
        harness.check((sb.indexOf("\u4444\u5555") == -1),
                      "String::indexOf(S) - invalid uni over result");
        harness.check((sb.indexOf("\u1111\u2222") == 0),
                      "String::indexOf(S) - invalid u-u result");
        harness.check((sb.indexOf("ab") == 2),
                      "String::indexOf(S) - invalid u-a result");
        harness.check((sb.indexOf("\u2222ab") == 1),
                      "String::indexOf(S) - invalid u-ua result");
        harness.check((sb.indexOf("\u1111\u2222", -12) == 0),
                      "String::indexOf(S) - invalid u--ve result");
        harness.check((sb.indexOf("\u1111", 555) == -1),
                      "String::indexOf(S) - invalid u-singleoff result");
        harness.check((sb.indexOf("\u3333", 5) == -1),
                      "String::indexOf(S) - invalid u-off result");
        harness.check((sb.indexOf("\u3333\u4444", 2) == 4),
                      "String::indexOf(S) - invalid u-okoff result");

        String sc = "abcdeeeeeeeeeee";
        try {
            int x = sc.lastIndexOf(null);
            harness.check(false, "String::lastIndexOf(null) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        try {
            int x = sc.lastIndexOf(null, 4);
            harness.check(false, "String::lastIndexOf(null, I) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sc.lastIndexOf("") == 15),
                      "String::lastIndexOf(S) - invalid empty result");
        harness.check((sc.lastIndexOf("", 5555) == 15),
                      "String::lastIndexOf(S) - invalid big empty result");
        harness.check((sc.lastIndexOf("\u2222\u3333") == -1),
                      "String::lastIndexOf(S) - invalid a-u result");
        harness.check((sc.lastIndexOf("bcdefghijlmnopqrst") == -1),
                      "String::lastIndexOf(S) - invalid overflow result");
        harness.check((sc.lastIndexOf("bcd") == 1),
                      "String::lastIndexOf(S) - invalid a-a result");
        harness.check((sc.lastIndexOf("efg") == -1),
                      "String::lastIndexOf(S) - invalid a-mm result");
        harness.check((sc.lastIndexOf("e") == 14),
                      "String::lastIndexOf(S) - invalid a-single result");
        harness.check((sc.lastIndexOf("cd", 555) == 2),
                      "String::lastIndexOf(S) - invalid a-out result");
        harness.check((sc.lastIndexOf("eee", -12) == -1),
                      "String::lastIndexOf(S) - invalid a-ve result");
        harness.check((sc.lastIndexOf("bcd", 0) == -1),
                      "String::lastIndexOf(S) - invalid a-off result");
        harness.check((sc.lastIndexOf("e", 10) == 10),
                      "String::lastIndexOf(S) - invalid a-singoff result");

        String sd = "\u1111\u2222ab\u3333\u4444";
        harness.check((sd.lastIndexOf("\u4444\u5555") == -1),
                      "String::lastIndexOf(S) - invalid uni over result");
        harness.check((sd.lastIndexOf("\u1111\u2222") == 0),
                      "String::lastIndexOf(S) - invalid u-u result");
        harness.check((sd.lastIndexOf("ab") == 2),
                      "String::lastIndexOf(S) - invalid u-a result");
        harness.check((sd.lastIndexOf("\u2222ab") == 1),
                      "String::lastIndexOf(S) - invalid u-ua result");
        harness.check((sd.lastIndexOf("\u1111\u2222", 44) == 0),
                      "String::lastIndexOf(S) - invalid u-off result");
        harness.check((sd.lastIndexOf("\u1111", -12) == -1),
                      "String::lastIndexOf(S) - invalid u--ve result");
        harness.check((sd.lastIndexOf("\u3333", 3) == -1),
                      "String::lastIndexOf(S) - invalid u-singoff result");
        harness.check((sd.lastIndexOf("\u3333\u4444", 5) == 4),
                      "String::lastIndexOf(S) - invalid u-okoff result");

        String se = "abcdefg";
        harness.check((se.indexOf('b') == 1),
                      "String::indexOf(C) - invalid a single char result");
        harness.check((se.indexOf('\u1111') == -1),
                      "String::indexOf(C) - invalid a-u result");
        harness.check((se.indexOf('b', 4) == -1),
                      "String::indexOf(C) - invalid a-a over result");
        harness.check((se.indexOf('e', 555) == -1),
                      "String::indexOf(C) - invalid a-a wide result");

        String sf = "\u1234\u2222\u3333\u4321";
        harness.check((sf.indexOf('\u2222') == 1),
                      "String::indexOf(C) - invalid u single char result");
        harness.check((sf.indexOf('a') == -1),
                      "String::indexOf(C) - invalid u-a result");
        harness.check((sf.indexOf('\u1234', 2) == -1),
                      "String::indexOf(C) - invalid u-u over result");
        harness.check((sf.indexOf('\u3333', 555) == -1),
                      "String::indexOf(C) - invalid u-u wide result");

        String sg = "abcdefg";
        harness.check((sg.lastIndexOf('b') == 1),
                      "String::lastIndexOf(C) - invalid a single char result");
        harness.check((sg.lastIndexOf('\u1111') == -1),
                      "String::lastIndexOf(C) - invalid a-u result");
        harness.check((sg.lastIndexOf('e', 2) == -1),
                      "String::lastIndexOf(C) - invalid a-a over result");
        harness.check((sg.lastIndexOf('e', -12) == -1),
                      "String::lastIndexOf(C) - invalid a-a wide result");

        String sh = "\u1234\u2222\u3333\u4321";
        harness.check((sh.lastIndexOf('\u2222') == 1),
                      "String::lastIndexOf(C) - invalid u single char result");
        harness.check((sh.lastIndexOf('a') == -1),
                      "String::lastIndexOf(C) - invalid u-a result");
        harness.check((sh.lastIndexOf('\u4321', 2) == -1),
                      "String::lastIndexOf(C) - invalid u-u over result");
        harness.check((sh.lastIndexOf('\u3333', -24) == -1),
                      "String::lastIndexOf(C) - invalid u-u wide result");
    }

    /**
     * Method called to invoke the test sequence.
     */
    public void test(TestHarness harness) {
        testInit(harness);
        testExtract(harness);
        testLookup(harness);
    }

    /**
     * Define this to ensure that Object.toString() is consistent.
     */
    public String toString() {
        return "testToString";
    }
}
