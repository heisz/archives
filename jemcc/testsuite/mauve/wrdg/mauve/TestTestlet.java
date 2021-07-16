// Basic testlet instance to validate the operation of the JEMCCTestHarness.
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

package wrdg.mauve;

import gnu.testlet.TestHarness;
import gnu.testlet.Testlet;

/**
 * <P>
 * Basic testlet instance used in the debugging of the JEMCCTestHarness and
 * the basic VM development.
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
 * @version $Revision: 1.2 $ $Date: 2004/01/05 12:42:15 $
 */
public class TestTestlet implements Testlet {
    /**
     * Method called to invoke the test sequence.
     */
    public void test(TestHarness harness) {
        harness.debug("TestTestlet has activated successfully.");
    }
}
