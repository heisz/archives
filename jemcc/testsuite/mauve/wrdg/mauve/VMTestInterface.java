// Java skeleton definitions for the direct VM test interface methods.
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

/**
 * <P>
 * Static method definitions which provide direct access to test code
 * in the JEMCC VM (not requiring external Java libraries).  This Java code
 * only exists for the purposes of compilation - the static JEMCC VM instance
 * which is created in this directory contains a JEMCC implementation of
 * this class.
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
public class VMTestInterface {
    /**
     * Convenience method to handle a fatal test condition.
     *
     * @param msg The message to output.
     */
    public static void die(String msg) {
        System.err.println("FATAL ERROR " + msg);
        System.exit(1);
    }

    /**
     * Method to output (print) a string to the stdout/stderr channels.
     *
     * @param msg The message to output.
     * @param isError If true, use the stderr channel, otherwise use stdout.
     * @param newline If true, append a newline to the output string.
     */
    public static void print(String msg, boolean isError, boolean newline) {
        System.err.println(msg);
    }

    /**
     * Method to scan the specified target locating class names to be launched
     * as test cases.
     *
     * @param target The target directory to be scanned for test classes.
     */
    public static String[] scanTests(String target) {
        return new String[] { target };
    }
}
