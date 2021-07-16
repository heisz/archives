// Test harness implementation for using static JEMCC VM against Mauve test lib.
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

import gnu.testlet.ResourceNotFoundException;
import gnu.testlet.TestHarness;
import gnu.testlet.Testlet;
import java.io.File;
import java.io.InputStream;
import java.io.Reader;

/**
 * <P>
 * Test program which replaces the Mauve SimpleTestHarness implementation
 * to provide more tightly integrated testing operations between the JEMCC
 * virtual machine and the Mauve Java testsuite.
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
 * @version $Revision: 1.3 $ $Date: 2004/01/05 12:42:15 $
 */
public class JEMCCTestHarness extends TestHarness {
    private boolean verboseEnabled = false, debugEnabled = false;

    /**
     * Constructor takes the argument flags from the main method.
     */
    public JEMCCTestHarness(boolean dbgEnabled, boolean verbEnabled) {
        /* Save the options */
        debugEnabled = dbgEnabled;
        verboseEnabled = verbEnabled;
    }

    /**
     * Harness method to obtain a Reader instance for a given resource name.
     *
     * @param name The name of the resource, using '#' characters as package
     *             separators.
     * @return A Reader instance which provides the contents of the named
     *         resource.
     * @exception ResourceNotFoundException The indicated resource name could
     *                                      not be found or read.
     */
    public Reader getResourceReader(String name) 
            throws ResourceNotFoundException {
        return null;
    }

    /**
     * Harness method to obtain an InputStream instance for a given resource.
     *
     * @param name The name of the resource, using '#' characters as package
     *             separators.
     * @return An InputStream instance which provides the contents of the named
     *         resource.
     * @exception ResourceNotFoundException The indicated resource name could
     *                                      not be found or read.
     */
    public InputStream getResourceStream(String name) 
            throws ResourceNotFoundException {
        return null;
    }

    /**
     * Harness method to obtain a File instance for a given resource.
     *
     * @param name The name of the resource, using '#' characters as package
     *             separators.
     * @return A File instance which refers to the requested resource.
     * @exception ResourceNotFoundException The indicated resource name could
     *                                      not be found or read.
     */
    public File getResourceFile(String name) 
            throws ResourceNotFoundException {
        return null;
    }

    /**
     * Harness method to obtain the directory to use for temporary files.
     *
     * @return The name of the temporary file directory.
     */
    public String getTempDirectory() {
        return "/tmp";
    }

    /**
     * Harness method to "check" the test result.  Allows for predetermined
     * failures of test methods.
     *
     * @param result The result of the test case - true if passed.
     */
    public void check(boolean result) {
        if (!result) VMTestInterface.die("Test has failed");
    }

    /**
     * Harness method which allows a testlet to "mark" a check point in
     * the test.  Allows tracking of exact failure locations during testing.
     *
     * @param name The name of the checkpoint the testlet is at.
     */
    public void checkPoint(String name) {
        VMTestInterface.print(name, true, true);
    }

    /**
     * Harness method to allow a testlet to send a message which should
     * only appear if the test programs are operating in "verbose" mode.
     *
     * @param message The message to appear.
     */
    public void verbose(String message) {
        if (verboseEnabled) {
            VMTestInterface.print(message, true, true);
        }
    }

    /**
     * Harness method to print a message which only appears when the test
     * programs are operating in "debug" mode.  For this method, a newline
     * is always appended to the message.
     *
     * @param message The message to be output.
     */
    public void debug(String message) {
        if (debugEnabled) {
            VMTestInterface.print(message, true, true);
        }
    }

    /**
     * Harness method to print a message which only appears when the test
     * programs are operating in "debug" mode.  Unlike the above method,
     * the newline output is controlled by the caller.
     *
     * @param message The message to be output.
     * @param newline A newline is only appended to the message output if this
     *                parameter is true.
     */
    public void debug(String message, boolean newline) {
        if (debugEnabled) {
            VMTestInterface.print(message, true, newline);
        }
    }

    /**
     * Harness method to print an exception stacktrace when the test
     * programs are operating in "debug" mode.
     *
     * @param message The message to be output.
     */
    public void debug(Throwable ex) {
    }

    /**
     * Harness method to print the contents of an object array (with a
     * leading descriptor) which only appears when the test programs are 
     * operating in "debug" mode.
     *
     * @param o The object array to output.
     * @param desc A leading description for the object array.
     */
    public void debug(Object[] o, String desc) {
    }

    /**
     * Main entry point to the test program.  Scans the Mauve class
     * instances and runs each testlet.
     *
     * @param args The command line arguments to the program.
     */
    public static void main(String argv[]) {
        boolean dbgEnabled = false, verbEnabled = false;

        /* Scan the argument list for options lists */
        String target = ".";
        for (int i = 0; i < argv.length; i++) {
            if (argv[i].equals("-d")) {
                dbgEnabled = true;
            } else if (argv[i].equals("-v")) {
                verbEnabled = true;
            } else {
                target = argv[i];
            }
        }

        /* Create the instance of the test harness */
        JEMCCTestHarness harness = new JEMCCTestHarness(dbgEnabled, 
                                                        verbEnabled);
        /* Try it! */
        harness.debug("JEMCCTestHarness starting");

        /* Find the target test classes */
        String testClassNames[] = VMTestInterface.scanTests(target);

        /* Run the tests! */
        for (int i = 0; i < testClassNames.length; i++) {
            try {
                Class tstClass = Class.forName(testClassNames[i]);
                if (Testlet.class.isAssignableFrom(tstClass)) {
                    VMTestInterface.print("*** Running test for class " +
                                                             testClassNames[i], 
                                          true, true);
                    Testlet test = (Testlet) tstClass.newInstance();
                    test.test(harness);
                }
            } catch (ClassNotFoundException ce) {
                VMTestInterface.print("*** Requested test class not found", 
                                      true, true);
            } catch (Throwable e) {
                VMTestInterface.print("*** Uncaught throwable " + e, 
                                      true, true);
            }
        }
    }
}
