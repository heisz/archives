This is Indigo, the JEMCC Virtual Machine Testsuite.
Copyright (C) 1999-2003, J.M. Heisz

This document provides details on the Indigo testsuite, in an FAQ style.

What Is Indigo?
---------------
  Indigo is a suite of programs and data conditions intended for testing
the implementation of a Java virtual machine, according to the information
provided in the Java Virtual Machine specification.

  This is not to be confused with Mauve (the Cygnus/RedHat Java testsuite)
which tests the associated classes and packages that are part of a Java
distribution.  Indigo tests the "internals" of the virtual machine, such as
how it handles corrupted class information, obscure linkage cases or 
verification failures.  It will indirectly test some of the core classes
of a Java distribution (like java.lang.Object or java.lang.Class) but that
is not its primary purpose.

  Of course, this becomes even more muddled by the fact that this testsuite
has been provided to the Mauve project and is included in the Mauve repository
(as another module).

What's With The Name?
---------------------
  Some people might think "Cute, another colour like Mauve.  How original".
Those people would be wrong...

  Actually, the testsuite is named for Inigo Montoya, one of the main
characters of the movie 'The Princess Bride'.  For those who haven't seen
the movie (you should!), Inigo has a 'severe case of vengeance' towards
the man who killed his father and has the following speech prepared for
when he meets him...

 'Hello, my name is Inigo Montoya.  You killed my father.  Prepare to die.'

 So, this testsuite is 'Indigo(sp)...the Java Virtual Machine killer'.  Here 
is a list of the casualties so far:

Sun JRE 1.3 [core dumps]
(Others to come, I'm sure)

[Besides, Tom Tromey once told me that Mauve is named after his dog...]
[And, of course, Indigo is another shade of blue/purple...]

What's Required To Run It?
--------------------------
  As little as possible.  Indigo is designed to test the virtual machine,
not the associated API's and packages.  The target VM requires support
for objects, arrays, classes, classloaders and the basic exception/error
instances/handling only.  No classes outside of java.lang.* are required
(and many within it aren't either) unless your virtual machine implementation
needs it.  

  Essentially, the testsuite runs against the JEMCC implementation without
any of the src/java packages.

How Do You Run Indigo/Detect Failures?
--------------------------------------
  The Indigo test classes should have been compiled as part of the standard
configure/make sequence for JEMCC.  If they have not been compiled, verify
that the --enable-testsuite option was specified when running configure and
that a JDK instance is available.  Or just manually invoke 'javac' against
each of the test classes in the 'wrdg' directory.

  In the JEMCC instance, simply perform a 'make check' in the testsuite/indigo
directory, which will run each of the Indigo test classes in sequence.  There
should be no output if the test cases complete successfully.  On an error,
an error/exception will be thrown which is not caught (and should be output
in some form at the VM level).  And, of course, if it core dumps...

  To utilize it with other virtual machines, just run each of the Indigo
test classes against the target virtual machine implementation.  The error
detection details already described apply as well (especially the part about
core dumps).

  Note that some of the Indigo test classes accept a '-sun' option.  These
bypass tests which comply with the contents of the Java Virtual Machine
specification but are not properly supported by the Sun VM implementation.

How To Debug (or Where Is The Source Code For The Test Classes)?
----------------------------------------------------------------
One of the elements in the Indigo testsuite that may cause difficulty is the
fact that the test cases utilize damaged class instances which appear
in binary (compiled) form.  Unfortunately, the source code which generated
the original/undamaged classes has been lost over time (alas, the author 
realized too late that it would be valuable to have those classes in the
source tree).

Technically, this source code is not really required, as in most cases
it is structure of the class, not the bytecode contents of the methods,
that is being tested.  However, it is possible to reconstruct the 'signature'
of the original class by taking the byte array for the 'normal' class
that appears at the beginning of the test program, write it to a binary
.class file and then use javap against it.  Or manually decompile it (a
good exercise for understanding class file formats).

The only exception to this are the class verification tests where specific
bytecode conditions are produced.  In these cases, there is no Java source
code and the bytecode has been produced by hand (with associated 'assembler'
comments).

What Is The INTERNAL Comment All About?
---------------------------------------
These comments are used to correlate the Indigo test cases against the
same test cases in the JEMCC testsuite/internals directory.  Why two identical
sets of test cases?  Well, the 'internals' tests were executable as the 
associated parsing/linking/verification components were completed.  The
Indigo test cases could only be run (against JEMCC) once the entire virtual
machine was somewhat operational.  In addition, the 'internals' tests include
arbitrary memory failure testing, something that is much more difficult
with the full virtual machine.
