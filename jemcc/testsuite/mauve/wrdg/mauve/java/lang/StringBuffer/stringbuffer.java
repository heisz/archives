// Mauve-compatible testlet instance for testing java.lang.StringBuffer.
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

package wrdg.mauve.java.lang.StringBuffer;

import gnu.testlet.TestHarness;
import gnu.testlet.Testlet;

public class stringbuffer implements Testlet {

    public void testInit(TestHarness harness) {
        StringBuffer sba = new StringBuffer();
        harness.check((sba.capacity() == 16),
                      "StringBuffer::<init>() - invalid capacity");
        harness.check((sba.length() == 0),
                      "StringBuffer::<init>() - invalid length");

        try {
            StringBuffer sbb = new StringBuffer(-12);
            harness.check(false,
                          "StringBuffer::<init>(-) accepted -ve capacity");
        } catch (NegativeArraySizeException ex) {
            harness.check(true, "Expected result");
        }
        StringBuffer sbb = new StringBuffer(32);
        harness.check((sbb.capacity() == 32),
                      "StringBuffer::<init>(I) - invalid capacity");
        harness.check((sbb.length() == 0),
                      "StringBuffer::<init>(I) - invalid length");

        try {
            StringBuffer sbc = new StringBuffer((String) null);
            harness.check(false,
                          "StringBuffer::<init>(String) accepted null");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        StringBuffer sbc = new StringBuffer("Testing 1..2..3");
        harness.check((sbc.capacity() == 31),
                      "StringBuffer::<init>(I) - invalid capacity");
        harness.check((sbc.length() == 15),
                      "StringBuffer::<init>(I) - invalid length");

        StringBuffer sbd = new StringBuffer("\u4357\u1234\u4565\u7777");
        harness.check((sbd.capacity() == 20),
                      "StringBuffer::<init>(I) - invalid capacity");
        harness.check((sbd.length() == 4),
                      "StringBuffer::<init>(I) - invalid length");

        /* Special JEMCC-specific test */
        String s = "testing";
        StringBuffer sb = new StringBuffer(s);
        harness.check((sb.toString() == s),
                      "StringBuffer::<init>(String) - didn't copy properly");
    }

    public void testAppendExpand(TestHarness harness) {
        String ascii = "abcdefghijklmnopqrstuvwxyz";
        String uni = "\u1234ab\u4321cd\u2453ef\u6578gh\u3322ij\u4398kl\u3245";

        /* Note: these tests don't cover content, just resize and ASCII-Uni */

        StringBuffer sba = new StringBuffer();
        sba.append(ascii);
        sba.append(ascii);
        /* 16; 16 * 2 + 2 > 26; 34 * 2 + 2 > 52 */
        harness.check((sba.capacity() == 70),
                      "StringBuffer::append.append - invalid a-a capacity");
        harness.check((sba.length() == 52),
                      "StringBuffer::append.append - invalid a-a length");

        StringBuffer sbb = new StringBuffer(10);
        sbb.append(ascii);
        sbb.append(uni);
        /* 2 * 10 + 2 < 26; 26 * 2 + 2 > 45 */
        harness.check((sbb.capacity() == 54),
                      "StringBuffer::append.append - invalid a-u capacity");
        harness.check((sbb.length() == 45),
                      "StringBuffer::append.append - invalid a-u length");

        StringBuffer sbc = new StringBuffer();
        sbc.append(uni);
        sbc.append(ascii);
        /* 16; 16 * 2 + 2 > 19; 34 * 2 + 2 > 45 */
        harness.check((sbc.capacity() == 70),
                      "StringBuffer::append.append - invalid u-a capacity");
        harness.check((sbc.length() == 45),
                      "StringBuffer::append.append - invalid u-a length");

        StringBuffer sbd = new StringBuffer(24);
        sbd.append(uni);
        sbd.append(uni);
        /* 24; 24 > 19; 2 * 24 + 2 > 38 */
        harness.check((sbd.capacity() == 50),
                      "StringBuffer::append.append - invalid u-u capacity");
        harness.check((sbd.length() == 38),
                      "StringBuffer::append.append - invalid u-u length");


        StringBuffer sbe = new StringBuffer(ascii);
        sbe.append(ascii);
        /* 26 + 16; 42 * 2 + 2 > 52 */
        harness.check((sbe.capacity() == 86),
                      "StringBuffer::<init>(S).append - invalid a-a capacity");
        harness.check((sbe.length() == 52),
                      "StringBuffer::<init>(S).append - invalid a-a length");


        StringBuffer sbf = new StringBuffer(ascii);
        sbf.append(uni);
        /* 26 + 16; 42 * 2 + 2 > 45 */
        harness.check((sbf.capacity() == 86),
                      "StringBuffer::<init>(S).append - invalid a-u capacity");
        harness.check((sbf.length() == 45),
                      "StringBuffer::<init>(S).append - invalid a-u length");

        StringBuffer sbg = new StringBuffer(uni);
        sbg.append(ascii);
        /* 19 + 16; 35 * 2 + 2 > 45 */
        harness.check((sbg.capacity() == 72),
                      "StringBuffer::<init>(S).append - invalid u-a capacity");
        harness.check((sbg.length() == 45),
                      "StringBuffer::<init>(S).append - invalid u-a length");

        StringBuffer sbh = new StringBuffer(uni);
        sbh.append(uni);
        /* 19 + 16; 35 * 2 + 2 > 38 */
        harness.check((sbh.capacity() == 72),
                      "StringBuffer::<init>(S).append - invalid u-u capacity");
        harness.check((sbh.length() == 38),
                      "StringBuffer::<init>(S).append - invalid u-u length");

        StringBuffer sbi = new StringBuffer(64);
        harness.check((sbi.capacity() == 64),
                      "StringBuffer::<init>(I) - invalid capacity");
        sbi.ensureCapacity(92);
        /* 64 * 2 + 2 > 92 */
        harness.check((sbi.capacity() == 130),
                      "StringBuffer::ensureCapacity() - invalid high capacity");
        sbi.ensureCapacity(512);
        /* 130 * 2 + 2 > 92 */
        harness.check((sbi.capacity() == 512),
                      "StringBuffer::ensureCapacity() - invalid low capacity");
        sbi.ensureCapacity(-1);
        sbi.ensureCapacity(32);
        harness.check((sbi.capacity() == 512),
                      "StringBuffer::ensureCapacity() - invalid non-capacity");
    }

    public void testAppend(TestHarness harness) {
        String ascii = "abcd";
        String uni = "\u1234\u5678\u8765\u4321";

        StringBuffer sba = new StringBuffer();
        sba.append(ascii);
        sba.append(ascii);
        harness.check(sba.toString().equals("abcdabcd"),
                      "StringBuffer::append.append - invalid a-a result");

        StringBuffer sbb = new StringBuffer(3);
        sbb.append(ascii);
        sbb.append(uni);
        harness.check(sbb.toString().equals("abcd\u1234\u5678\u8765\u4321"),
                      "StringBuffer::append.append - invalid a-u result");

        StringBuffer sbc = new StringBuffer();
        sbc.append(uni);
        sbc.append(ascii);
        harness.check(sbc.toString().equals("\u1234\u5678\u8765\u4321abcd"),
                      "StringBuffer::append.append - invalid u-a result");

        StringBuffer sbd = new StringBuffer(24);
        sbd.append(uni);
        sbd.append(uni);
        harness.check(sbd.toString().equals(
                           "\u1234\u5678\u8765\u4321\u1234\u5678\u8765\u4321"),
                      "StringBuffer::append.append - invalid u-a result");

        StringBuffer sbe = new StringBuffer(ascii);
        sbe.append(ascii);
        harness.check(sbe.toString().equals("abcdabcd"),
                      "StringBuffer::<init>(S).append - invalid a-a result");

        StringBuffer sbf = new StringBuffer(ascii);
        sbf.append(uni);
        harness.check(sbf.toString().equals("abcd\u1234\u5678\u8765\u4321"),
                      "StringBuffer::<init>(S).append - invalid a-u result");

        StringBuffer sbg = new StringBuffer(uni);
        sbg.append(ascii);
        harness.check(sbg.toString().equals("\u1234\u5678\u8765\u4321abcd"),
                      "StringBuffer::<init>(S).append - invalid u-a result");

        StringBuffer sbh = new StringBuffer(uni);
        sbh.append(uni);
        harness.check(sbh.toString().equals(
                           "\u1234\u5678\u8765\u4321\u1234\u5678\u8765\u4321"),
                      "StringBuffer::<init>(S).append - invalid u-u result");

        StringBuffer sbi = new StringBuffer("");
        sbi.append('T').append('e').append('s').append('t');
        harness.check(sbi.toString().equals("Test"),
                      "StringBuffer::append(C)... - invalid a-a result");
        sbi.append('\u1234').append('\u4321');
        harness.check(sbi.toString().equals("Test\u1234\u4321"),
                      "StringBuffer::append(C)... - invalid a-u result");

        StringBuffer sbj = new StringBuffer();
        sbj.append((double) 0.0);
        sbj.append(':');
        sbj.append((double) 34.0);
        harness.check(sbj.toString().equals("0.0:34.0"),
                      "StringBuffer::append(D)... - invalid result");

        StringBuffer sbk = new StringBuffer();
        sbk.append((float) 0.0);
        sbk.append(':');
        sbk.append((float) -34.0);
        harness.check(sbk.toString().equals("0.0:-34.0"),
                      "StringBuffer::append(F)... - invalid result");

        StringBuffer sbl = new StringBuffer();
        sbl.append((int) -12);
        sbl.append(':');
        sbl.append((int) 4567435);
        sbl.append(':');
        sbl.append((int) 0);
        sbl.append(':');
        sbl.append((int) -2147483648);
        harness.check(sbl.toString().equals("-12:4567435:0:-2147483648"),
                      "StringBuffer::append(I)... - invalid result");

        StringBuffer sbm = new StringBuffer();
        sbm.append(-1234567L);
        sbm.append(':');
        sbm.append(4567435345645L);
        sbm.append(':');
        sbm.append(0L);
        sbm.append(':');
        sbm.append(-9223372036854775808L);
        harness.check(sbm.toString().equals(
                               "-1234567:4567435345645:0:-9223372036854775808"),
                      "StringBuffer::append(J)... - invalid result");

        StringBuffer sbn = new StringBuffer();
        Object o = null;
        sbn.append(o);
        harness.check(sbn.toString().equals("null"),
                      "StringBuffer::append((Object) null) - invalid result");
        sbn.append(':');
        sbn.append((Object) this);
        harness.check(sbn.toString().equals("null:testToString"),
                      "StringBuffer::append(Object) - invalid result");

        StringBuffer sbo = new StringBuffer();
        String s = null;
        sbo.append(s);
        harness.check(sbo.toString().equals("null"),
                      "StringBuffer::append((String) null) - invalid result");

        StringBuffer sbp = new StringBuffer();
        StringBuffer sb = null;
        sbp.append(s);
        harness.check(sbp.toString().equals("null"),
                      "StringBuffer::append((SB) null) - invalid result");
        sb = new StringBuffer("Test");
        sbp.append(':');
        sbp.append(sb);
        sb.append("...1...2...3");
        sbp.append(':');
        sbp.append(sb);
        harness.check(sbp.toString().equals("null:Test:Test...1...2...3"),
                      "StringBuffer::append((SB))... - invalid result");

        StringBuffer sbq = new StringBuffer();
        sbq.append(true);
        sbq.append(':');
        sbq.append(false);
        harness.check(sbq.toString().equals("true:false"),
                      "StringBuffer::append(Z)... - invalid result");

        StringBuffer sbr = new StringBuffer();
        char ca[] = { 'T', 'e', 's', 't' };
        char cb[] = { '\u1234', '\u4321', '\u7777' };
        char cc[] = new char[0];
        sbr.append(ca);
        sbr.append(':');
        sbr.append(cb);
        sbr.append(cc);
        harness.check(sbr.toString().equals("Test:\u1234\u4321\u7777"),
                      "StringBuffer::append([C)... - invalid result");

        StringBuffer sbs = new StringBuffer();
        try {
            sbs.append(ca, 12, 4);
            harness.check(false, "StringBuffer::append([CII) - bad offset ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.append(ca, -1, 4);
            harness.check(false, "StringBuffer::append([CII) - -ve offset ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.append(ca, 1, 12);
            harness.check(false, "StringBuffer::append([CII) - bad length ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbs.append(ca, 1, 3);
        sbs.append(':');
        sbs.append(cb, 1, 1);
        sbs.append(ca, 1, 0);
        harness.check(sbs.toString().equals("est:\u4321"),
                      "StringBuffer::append([CII)... - invalid result");
    }

    public void testExtract(TestHarness harness) {
        StringBuffer sba = new StringBuffer("Testing");
        StringBuffer sbb = new StringBuffer("\u1234\u4321ab");
        StringBuffer sbc = new StringBuffer("Testing");
        sbc.append("...1...2...3");
        StringBuffer sbd = new StringBuffer("\u1111");
        sbd.append("\u2222\u3333abc");

        try {
            char ch = sba.charAt(-44);
            harness.check(false, "StringBuffer::charAt(I) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            char ch = sbb.charAt(88);
            harness.check(false, "StringBuffer::charAt(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            char ch = sbc.charAt(88);
            harness.check(false, "StringBuffer::charAt(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sba.charAt(3) == 't'),
                      "StringBuffer::charAt()... - invalid a result");
        harness.check((sbb.charAt(1) == '\u4321'),
                      "StringBuffer::charAt()... - invalid b result");
        harness.check((sbc.charAt(0) == 'T'),
                      "StringBuffer::charAt()... - invalid c result");
        harness.check((sbd.charAt(2) == '\u3333'),
                      "StringBuffer::charAt()... - invalid d result");

        char copy[] = new char[6];
        try {
            sbc.getChars(-1, 5, copy, 0);
            harness.check(false, "StringBuffer::getChars() - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(1, -1, copy, 0);
            harness.check(false, "StringBuffer::getChars() - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(63, 64, copy, 0);
            harness.check(false, "StringBuffer::getChars() - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(1, 64, copy, 0);
            harness.check(false, "StringBuffer::getChars() - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(5, 1, copy, 0);
            harness.check(false, "StringBuffer::getChars() - inv index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(0, 4, copy, -1);
            harness.check(false, "StringBuffer::getChars() - -ve index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(0, 4, copy, 8);
            harness.check(false, "StringBuffer::getChars() - bad index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(0, 7, copy, 0);
            harness.check(false, "StringBuffer::getChars() - big index ok?");
        } catch (ArrayIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbc.getChars(0, 2, null, 1);
            harness.check(false, "StringBuffer::getChars() - null array ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }

        sba.getChars(0, 2, copy, 0);
        sba.getChars(1, 1, copy, 0);
        harness.check(((copy[0] == 'T') && (copy[1] == 'e')),
                      "StringBuffer::getChars()... - invalid a result");
        sbb.getChars(0, 2, copy, 2);
        harness.check(((copy[2] == '\u1234') && (copy[3] == '\u4321')),
                      "StringBuffer::getChars()... - invalid b result");
        sbc.getChars(3, 5, copy, 0);
        harness.check(((copy[0] == 't') && (copy[1] == 'i')),
                      "StringBuffer::getChars()... - invalid c result");
        sbd.getChars(0, 3, copy, 1);
        harness.check(((copy[1] == '\u1111') && (copy[3] == '\u3333')),
                      "StringBuffer::getChars()... - invalid d result");

        try {
            String s = sbc.substring(-1);
            harness.check(false, "StringBuffer::substring(I) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sbc.substring(44);
            harness.check(false, "StringBuffer::substring(I) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sba.substring(4).equals("ing"),
                      "StringBuffer::substring(I)... - invalid a result");
        harness.check(sbb.substring(2).equals("ab"),
                      "StringBuffer::substring(I)... - invalid b result");
        harness.check(sbc.substring(0).equals("Testing...1...2...3"),
                      "StringBuffer::substring(I)... - invalid c result");
        harness.check(sbd.substring(1).equals("\u2222\u3333abc"),
                      "StringBuffer::substring(I)... - invalid d result");
        harness.check(sbd.substring(4).equals("bc"),
                      "StringBuffer::substring(I)... - invalid a-d result");

        try {
            String s = sbc.substring(-1, 4);
            harness.check(false, "StringBuffer::substring(II) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sbc.substring(2, 88);
            harness.check(false, "StringBuffer::substring(II) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            String s = sbc.substring(4, 2);
            harness.check(false, "StringBuffer::substring(II) - rev index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sba.substring(4, 6).equals("in"),
                      "StringBuffer::substring(II)... - invalid a result");
        harness.check(sbb.substring(2, 4).equals("ab"),
                      "StringBuffer::substring(II)... - invalid b result");
        harness.check(sbc.substring(0, 11).equals("Testing...1"),
                      "StringBuffer::substring(II)... - invalid c result");
        harness.check(sbd.substring(1, 3).equals("\u2222\u3333"),
                      "StringBuffer::substring(II)... - invalid d result");

        try {
            CharSequence cs = sbc.subSequence(-1, 4);
            harness.check(false, "StringBuffer::subSeq(II) - -ve index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            CharSequence cs = sbc.subSequence(2, 88);
            harness.check(false, "StringBuffer::subSeq(II) - bad index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            CharSequence cs = sbc.subSequence(4, 2);
            harness.check(false, "StringBuffer::subSeq(II) - rev index ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        harness.check(sba.subSequence(4, 6).equals("in"),
                      "StringBuffer::subSequence(II)... - invalid a result");
        harness.check(sbb.subSequence(2, 4).equals("ab"),
                      "StringBuffer::subSequence(II)... - invalid b result");
        harness.check(sbc.subSequence(0, 11).equals("Testing...1"),
                      "StringBuffer::subSequence(II)... - invalid c result");
        harness.check(sbd.subSequence(1, 3).equals("\u2222\u3333"),
                      "StringBuffer::subSequence(II)... - invalid d result");
    }

    public void testInsert(TestHarness harness) {
        String ascii = "abcd";
        String uni = "\u1234\u5678\u8765\u4321";

        StringBuffer sb = new StringBuffer();
        try {
            sb.insert(-2, "oops");
            harness.check(false, "StringBuffer::insert() - -ve length ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sb.insert(22, "oops");
            harness.check(false, "StringBuffer::insert() - bad length ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }

        StringBuffer sba = new StringBuffer();
        sba.append(ascii);
        sba.insert(2, ascii);
        harness.check(sba.toString().equals("ababcdcd"),
                      "StringBuffer::append.insert - invalid a-a result");

        StringBuffer sbb = new StringBuffer(3);
        sbb.append(ascii);
        sbb.insert(0, uni);
        harness.check(sbb.toString().equals("\u1234\u5678\u8765\u4321abcd"),
                      "StringBuffer::append.insert - invalid a-u result");

        StringBuffer sbc = new StringBuffer();
        sbc.insert(0, uni);
        sbc.insert(1, ascii);
        harness.check(sbc.toString().equals("\u1234abcd\u5678\u8765\u4321"),
                      "StringBuffer::insert.insert - invalid u-a result");

        StringBuffer sbd = new StringBuffer(24);
        sbd.append(uni);
        sbd.insert(3, uni);
        harness.check(sbd.toString().equals(
                           "\u1234\u5678\u8765\u1234\u5678\u8765\u4321\u4321"),
                      "StringBuffer::append.insert - invalid u-a result");

        StringBuffer sbe = new StringBuffer(ascii);
        sbe.insert(0, ascii);
        harness.check(sbe.toString().equals("abcdabcd"),
                      "StringBuffer::<init>(S).insert - invalid a-a result");

        StringBuffer sbf = new StringBuffer(ascii);
        sbf.insert(1, uni);
        harness.check(sbf.toString().equals("a\u1234\u5678\u8765\u4321bcd"),
                      "StringBuffer::<init>(S).insert - invalid a-u result");

        StringBuffer sbg = new StringBuffer(uni);
        sbg.insert(3, ascii);
        harness.check(sbg.toString().equals("\u1234\u5678\u8765abcd\u4321"),
                      "StringBuffer::<init>(S).insert - invalid u-a result");

        StringBuffer sbh = new StringBuffer(uni);
        sbh.insert(1, uni);
        harness.check(sbh.toString().equals(
                           "\u1234\u1234\u5678\u8765\u4321\u5678\u8765\u4321"),
                      "StringBuffer::<init>(S).insert - invalid u-u result");


        StringBuffer sbi = new StringBuffer("");
        sbi.insert(0, 'T').insert(1, 't').insert(1, 's').insert(1, 'e');
        harness.check(sbi.toString().equals("Test"),
                      "StringBuffer::insert(IC)... - invalid a-a result");
        sbi.insert(2, '\u1234').insert(4, '\u4321');
        harness.check(sbi.toString().equals("Te\u1234s\u4321t"),
                      "StringBuffer::insert(C)... - invalid a-u result");

        StringBuffer sbj = new StringBuffer();
        sbj.append("aa");
        sbj.insert(1, (double) 34.0);
        harness.check(sbj.toString().equals("a34.0a"),
                      "StringBuffer::insert(D) - invalid result");

        StringBuffer sbk = new StringBuffer();
        sbk.append("::");
        sbk.insert(1, (float) -34.0);
        harness.check(sbk.toString().equals(":-34.0:"),
                      "StringBuffer::append(F) - invalid result");

        StringBuffer sbl = new StringBuffer();
        sbl.insert(0, (int) -2147483648);
        sbl.insert(0, ':');
        sbl.insert(0, (int) 0);
        sbl.insert(0, ':');
        sbl.insert(0, (int) 4567435);
        sbl.insert(0, ':');
        sbl.insert(0, (int) -12);
        harness.check(sbl.toString().equals("-12:4567435:0:-2147483648"),
                      "StringBuffer::insert(I)... - invalid result");

        StringBuffer sbm = new StringBuffer();
        sbm.insert(0, -9223372036854775808L);
        sbm.insert(0, ':');
        sbm.insert(0, 0L);
        sbm.insert(0, ':');
        sbm.insert(0, 4567435345645L);
        sbm.insert(0, ':');
        sbm.insert(0, -1234567L);
        harness.check(sbm.toString().equals(
                               "-1234567:4567435345645:0:-9223372036854775808"),
                      "StringBuffer::insert(J)... - invalid result");

        StringBuffer sbn = new StringBuffer();
        Object o = null;
        sbn.insert(0, o);
        sbn.insert(1, o);
        harness.check(sbn.toString().equals("nnullull"),
                      "StringBuffer::insert((Object) null) - invalid result");
        sbn.insert(1, (Object) this);
        harness.check(sbn.toString().equals("ntestToStringnullull"),
                      "StringBuffer::insert(Object) - invalid result");

        StringBuffer sbo = new StringBuffer();
        String s = null;
        sbo.insert(0, s);
        sbo.insert(2, s);
        harness.check(sbo.toString().equals("nunullll"),
                      "StringBuffer::insert((String) null).. - invalid result");

        StringBuffer sbq = new StringBuffer();
        sbq.insert(0, true);
        sbq.insert(2, false);
        harness.check(sbq.toString().equals("trfalseue"),
                      "StringBuffer::insert(Z)... - invalid result");

        StringBuffer sbr = new StringBuffer();
        char ca[] = { 'T', 'e', 's', 't' };
        char cb[] = { '\u1234', '\u4321', '\u7777' };
        char cc[] = new char[0];
        sbr.insert(0, ca);
        sbr.insert(2, cb);
        sbr.insert(0, cc);
        harness.check(sbr.toString().equals("Te\u1234\u4321\u7777st"),
                      "StringBuffer::insert([C)... - invalid result");

        StringBuffer sbs = new StringBuffer("abcd");
        try {
            sbs.insert(2, ca, 12, 4);
            harness.check(false, "StringBuffer::insert([CII) - bad offset ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.insert(2, ca, -1, 4);
            harness.check(false, "StringBuffer::insert([CII) - -ve offset ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.insert(2, ca, 1, 12);
            harness.check(false, "StringBuffer::insert([CII) - bad length ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbs.insert(2, ca, 1, 3);
        sbs.insert(1, cb, 1, 1);
        sbs.insert(3, ca, 1, 0);
        harness.check(sbs.toString().equals("a\u4321bestcd"),
                      "StringBuffer::insert([CII)... - invalid result");
    }

    public void testMisc(TestHarness harness) {
        StringBuffer sba = new StringBuffer();
        sba.append("1234");
        harness.check(sba.reverse().toString().equals("4321"),
                      "StringBuffer::reverse() - invalid Ascii result");

        StringBuffer sbb = new StringBuffer();
        sbb.append("\u1111\u2222\u3333");
        harness.check(sbb.reverse().toString().equals("\u3333\u2222\u1111"),
                      "StringBuffer::reverse() - invalid Unicode result");

        StringBuffer sbc = new StringBuffer("abcd");
        harness.check(sbc.reverse().toString().equals("dcba"),
                      "StringBuffer::reverse() - invalid Ascii result");


        StringBuffer sbd = new StringBuffer();
        sbd.append("eeee");
        try {
            sbd.setCharAt(-12, 'c');
            harness.check(false, "StringBuffer::setCharAt() - -ve idx ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbd.setCharAt(44, 'c');
            harness.check(false, "StringBuffer::setCharAt() - bad idx ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbd.setCharAt(0, 'a');
        sbd.setCharAt(1, 'b');
        sbd.setCharAt(2, 'c');
        sbd.setCharAt(3, 'd');
        harness.check(sbd.toString().equals("abcd"),
                      "StringBuffer::setCharAt()... - invalid Ascii result");
        sbd.setCharAt(0, '\u1234');
        harness.check(sbd.toString().equals("\u1234bcd"),
                      "StringBuffer::setCharAt()... - invalid mixed result");

        StringBuffer sbe = new StringBuffer("\u3333\u3333\u3333");
        try {
            sbe.setCharAt(-12, 'c');
            harness.check(false, "StringBuffer::setCharAt() - -ve idx ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbe.setCharAt(44, 'c');
            harness.check(false, "StringBuffer::setCharAt() - bad idx ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbe.setCharAt(0, '\u1111');
        sbe.setCharAt(1, 'x');
        harness.check(sbe.toString().equals("\u1111x\u3333"),
                      "StringBuffer::setCharAt()... - invalid Unicode result");

        StringBuffer sbf = new StringBuffer("a\u1111c");
        sbf.setCharAt(1, 'b');
        harness.check(sbf.toString().equals("abc"),
                      "StringBuffer::setCharAt()... - invalid u-a result");
        sbf.setCharAt(2, '\u3333');
        harness.check(sbf.toString().equals("ab\u3333"),
                      "StringBuffer::setCharAt()... - invalid u-a result");

        StringBuffer sbg = new StringBuffer(5);
        sbg.append("abcdefg");
        try {
            sbg.setLength(-6);
            harness.check(false, "StringBuffer::setLength() - -ve len ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbg.setLength(4);
        harness.check(sbg.toString().equals("abcd"),
                      "StringBuffer::setLength() - invalid ascii result");
        sbg.setLength(7);
        harness.check(sbg.toString().equals("abcd\u0000\u0000\u0000"),
                      "StringBuffer::setLength() - invalid aexpand result");
        sbg.setLength(3);
        harness.check(sbg.toString().equals("abc"),
                      "StringBuffer::setLength() - invalid u-a result");

        StringBuffer sbh = new StringBuffer();
        sbh.append("\u1111\u2222\u3333\u4444");
        sbh.setLength(2);
        harness.check(sbh.toString().equals("\u1111\u2222"),
                      "StringBuffer::setLength() - invalid unicode result");
        sbh.setLength(3);
        harness.check(sbh.toString().equals("\u1111\u2222\u0000"),
                      "StringBuffer::setLength() - invalid uexpand result");

        StringBuffer sbi = new StringBuffer("abcd");
        sbi.setLength(2);
        harness.check(sbi.toString().equals("ab"),
                      "StringBuffer::setLength() - invalid raw result");
        sbi.setLength(0);
        harness.check(sbi.toString().equals(""),
                      "StringBuffer::setLength() - invalid zero result");
        sbi.setLength(2);
        harness.check(sbi.toString().equals("\u0000\u0000"),
                      "StringBuffer::setLength() - invalid 0 expand result");

        StringBuffer sbj = new StringBuffer();
        sbj.append("abcd");
        try {
            sbj.delete(-1, 4);
            harness.check(false, "StringBuffer::delete() - -ve start ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbj.delete(3, 2);
            harness.check(false, "StringBuffer::delete() - bad ends ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbj.delete(2, 555);
        harness.check(sbj.toString().equals("ab"),
                      "StringBuffer::delete() - invalid ascii result");
        sbj.append("cdefghijk");
        sbj.delete(3, 8);
        harness.check(sbj.toString().equals("abcijk"),
                      "StringBuffer::delete()... - invalid ascii result");

        StringBuffer sbk = new StringBuffer("\u1111\u2222\u3333");
        sbk.delete(2, 5);
        harness.check(sbk.toString().equals("\u1111\u2222"),
                      "StringBuffer::delete() - invalid uni result");
        sbk.delete(0, 2);
        harness.check(sbk.toString().equals(""),
                      "StringBuffer::delete() - invalid zero result");
        sbk.append("abc\u1111\u2222\u3333");
        sbk.delete(3, 1111);
        harness.check(sbk.toString().equals("abc"),
                      "StringBuffer::delete() - invalid a-u-a result");

        StringBuffer sbl = new StringBuffer("1234");
        try {
            sbl.deleteCharAt(-1);
            harness.check(false, "StringBuffer::deleteCharAt() - -ve ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbl.deleteCharAt(12);
            harness.check(false, "StringBuffer::deleteCharAt() - bad ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        sbl.deleteCharAt(2);
        harness.check(sbl.toString().equals("124"),
                      "StringBuffer::deleteCharAt() - invalid ascii result");
        sbl.deleteCharAt(0);
        harness.check(sbl.toString().equals("24"),
                      "StringBuffer::deleteCharAt() - invalid zero asc result");
        sbl.deleteCharAt(1);
        harness.check(sbl.toString().equals("2"),
                      "StringBuffer::deleteCharAt() - invalid end asc result");
        sbl.deleteCharAt(0);
        harness.check(sbl.toString().equals(""),
                      "StringBuffer::deleteCharAt() - invalid empty result");

        StringBuffer sbm = new StringBuffer("\u1111\u2222\u3333\u4444");
        sbm.deleteCharAt(2);
        harness.check(sbm.toString().equals("\u1111\u2222\u4444"),
                      "StringBuffer::deleteCharAt() - invalid uni result");
        sbm.deleteCharAt(0);
        harness.check(sbm.toString().equals("\u2222\u4444"),
                      "StringBuffer::deleteCharAt() - invalid zero uni result");
        sbm.deleteCharAt(1);
        harness.check(sbm.toString().equals("\u2222"),
                      "StringBuffer::deleteCharAt() - invalid end uni result");
        sbm.deleteCharAt(0);
        harness.check(sbm.toString().equals(""),
                      "StringBuffer::deleteCharAt() - invalid empty result");

        StringBuffer sbo = new StringBuffer();
        sbo.append("abcdeeeeeeeeeeeee");
        try {
            int x = sbo.indexOf(null);
            harness.check(false, "StringBuffer::indexOf(null) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        try {
            int x = sbo.indexOf(null, 4);
            harness.check(false, "StringBuffer::indexOf(null, I) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sbo.indexOf("") == 0),
                      "StringBuffer::indexOf() - invalid empty result");
        harness.check((sbo.indexOf("\u2222\u3333") == -1),
                      "StringBuffer::indexOf() - invalid a-u result");
        harness.check((sbo.indexOf("bcdefghijlmnopqrst") == -1),
                      "StringBuffer::indexOf() - invalid overflow result");
        harness.check((sbo.indexOf("bcd") == 1),
                      "StringBuffer::indexOf() - invalid a-a result");
        harness.check((sbo.indexOf("efg") == -1),
                      "StringBuffer::indexOf() - invalid a-mm result");
        harness.check((sbo.indexOf("e") == 4),
                      "StringBuffer::indexOf() - invalid a-single result");
        harness.check((sbo.indexOf("cd", -10) == 2),
                      "StringBuffer::indexOf() - invalid a-ve result");
        harness.check((sbo.indexOf("eee", 555) == -1),
                      "StringBuffer::indexOf() - invalid a-out result");
        harness.check((sbo.indexOf("bcd", 5) == -1),
                      "StringBuffer::indexOf() - invalid a-off result");
        harness.check((sbo.indexOf("e", 10) == 10),
                      "StringBuffer::indexOf() - invalid a-singleoff result");

        StringBuffer sbp = new StringBuffer("\u1111\u2222ab\u3333\u4444");
        harness.check((sbp.indexOf("\u4444\u5555") == -1),
                      "StringBuffer::indexOf() - invalid uni over result");
        harness.check((sbp.indexOf("\u1111\u2222") == 0),
                      "StringBuffer::indexOf() - invalid u-u result");
        harness.check((sbp.indexOf("ab") == 2),
                      "StringBuffer::indexOf() - invalid u-a result");
        harness.check((sbp.indexOf("\u2222ab") == 1),
                      "StringBuffer::indexOf() - invalid u-ua result");
        harness.check((sbp.indexOf("\u1111\u2222", -12) == 0),
                      "StringBuffer::indexOf() - invalid u--ve result");
        harness.check((sbp.indexOf("\u1111", 555) == -1),
                      "StringBuffer::indexOf() - invalid u-singleoff result");
        harness.check((sbp.indexOf("\u3333", 5) == -1),
                      "StringBuffer::indexOf() - invalid u-off result");
        harness.check((sbp.indexOf("\u3333\u4444", 2) == 4),
                      "StringBuffer::indexOf() - invalid u-okoff result");

        StringBuffer sbq = new StringBuffer();
        sbq.append("abcdeeeeeeeeeee");
        try {
            int x = sbq.lastIndexOf(null);
            harness.check(false, "StringBuffer::lastIndexOf(null) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        try {
            int x = sbq.lastIndexOf(null, 4);
            harness.check(false, "StringBuffer::lastIndexOf(null, I) - ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        harness.check((sbq.lastIndexOf("") == 15),
                      "StringBuffer::lastIndexOf() - invalid empty result");
        harness.check((sbq.lastIndexOf("", 5555) == 15),
                      "StringBuffer::lastIndexOf() - invalid big empty result");
        harness.check((sbq.lastIndexOf("\u2222\u3333") == -1),
                      "StringBuffer::lastIndexOf() - invalid a-u result");
        harness.check((sbq.lastIndexOf("bcdefghijlmnopqrst") == -1),
                      "StringBuffer::lastIndexOf() - invalid overflow result");
        harness.check((sbq.lastIndexOf("bcd") == 1),
                      "StringBuffer::lastIndexOf() - invalid a-a result");
        harness.check((sbq.lastIndexOf("efg") == -1),
                      "StringBuffer::lastIndexOf() - invalid a-mm result");
        harness.check((sbq.lastIndexOf("e") == 14),
                      "StringBuffer::lastIndexOf() - invalid a-single result");
        harness.check((sbo.lastIndexOf("cd", 555) == 2),
                      "StringBuffer::lastIndexOf() - invalid a-out result");
        harness.check((sbo.lastIndexOf("eee", -12) == -1),
                      "StringBuffer::lastIndexOf() - invalid a-ve result");
        harness.check((sbo.lastIndexOf("bcd", 0) == -1),
                      "StringBuffer::lastIndexOf() - invalid a-off result");
        harness.check((sbo.lastIndexOf("e", 10) == 10),
                      "StringBuffer::lastIndexOf() - invalid a-singoff result");

        StringBuffer sbr = new StringBuffer("\u1111\u2222ab\u3333\u4444");
        harness.check((sbr.lastIndexOf("\u4444\u5555") == -1),
                      "StringBuffer::lastIndexOf() - invalid uni over result");
        harness.check((sbr.lastIndexOf("\u1111\u2222") == 0),
                      "StringBuffer::lastIndexOf() - invalid u-u result");
        harness.check((sbr.lastIndexOf("ab") == 2),
                      "StringBuffer::lastIndexOf() - invalid u-a result");
        harness.check((sbr.lastIndexOf("\u2222ab") == 1),
                      "StringBuffer::lastIndexOf() - invalid u-ua result");
        harness.check((sbr.lastIndexOf("\u1111\u2222", 44) == 0),
                      "StringBuffer::lastIndexOf() - invalid u-off result");
        harness.check((sbr.lastIndexOf("\u1111", -12) == -1),
                      "StringBuffer::lastIndexOf() - invalid u--ve result");
        harness.check((sbr.lastIndexOf("\u3333", 3) == -1),
                      "StringBuffer::lastIndexOf() - invalid u-singoff result");
        harness.check((sbr.lastIndexOf("\u3333\u4444", 5) == 4),
                      "StringBuffer::lastIndexOf() - invalid u-okoff result");

        StringBuffer sbs = new StringBuffer();
        sbs.append("abcdefg");
        try {
            sbs.replace(-1, 4, "a");
            harness.check(false, "StringBuffer::replace() - -ve start ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.delete(3, 2);
            harness.check(false, "StringBuffer::replace() - bad ends ok?");
        } catch (StringIndexOutOfBoundsException ex) {
            harness.check(true, "Expected result");
        }
        try {
            sbs.replace(3, 4, null);
            harness.check(false, "StringBuffer::replace() - null ok?");
        } catch (NullPointerException ex) {
            harness.check(true, "Expected result");
        }
        sbs.replace(2, 3, "xyz");
        harness.check(sbs.toString().equals("abxyzdefg"),
                      "StringBuffer::replace() - invalid a-a result");
        sbs.replace(0, 3, "\u1111\u2222");
        harness.check(sbs.toString().equals("\u1111\u2222yzdefg"),
                      "StringBuffer::replace() - invalid a-u result");
        sbs.replace(6, 44, "\u4444");
        harness.check(sbs.toString().equals("\u1111\u2222yzde\u4444"),
                      "StringBuffer::replace() - invalid u-u result");
        sbs.replace(5, 7, "");
        harness.check(sbs.toString().equals("\u1111\u2222yzd"),
                      "StringBuffer::replace() - invalid u-empty result");
        sbs.replace(0, 2, "a");
        harness.check(sbs.toString().equals("ayzd"),
                      "StringBuffer::replace() - invalid u-a result");
    }

    /**
     * Method called to invoke the test sequence.
     */
    public void test(TestHarness harness) {
        testInit(harness);
        testAppendExpand(harness); 
        testAppend(harness);
        testExtract(harness);
        testInsert(harness);
        testMisc(harness);
    }

    /**
     * Define this to ensure that Object.toString() is consistent.
     */
    public String toString() {
        return "testToString";
    }
}
