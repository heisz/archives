# JEMCC, the Java Embedded Machine for Compiled Classes.
Copyright (C) 1999-2004 J.M. Heisz

Alas, this was a project that was sort of going somewhere (was built on a
dare/challenge from a former CEO who told me there was no way I could do what
I proposed...challenge accepted!).

And then Sun got sold to Oracle and Oracle stuck their heads up their
netherregions and created licensing conditions stating it was no longer legal
to develop a Java implementation without paying $$$ to license their
certification testsuite.  And that pretty much killed this project...

Ah well, a lot of the core code got mutated into structural elements that
were reused in parts of the xkoto codebase.  However, there as an independent
pthreads implementation for Windows that, well, don't get me started on the
legal corner our lame-duck US CEO put me in, which is why that file is
missing.  Oh well, the rewrite with more modern Windows API's can be found
in the toolkit repository...

# Notes on the JEM/JEMCC Differentiation
Anyone perusing the source code in this distribution will come across
a variation in the "naming" of functions and typedefs - while most use
'JEMCC' there are other locations where 'JEM' will be encountered.

This is a historical throwback.  Originally, this work was developed under
the name 'JEM' (for Java Embedded Machine).  The "special" interface for 
producing native class instances was called JEMINI (for JEM Integrated Native
Interface).  Of course, when a single person undertakes such a large project, 
it can take years to complete (as it has).  Unfortunately, during the same 
time period two other companies produced Java virtual machine related products
with associated trademarks for both of these names.

Hence, the new name JEMCC (if someone trademarks that name, well, then I
guess it's time to quit).  All externally exposed API's and executables use
this name as a prefix.  Internally, the name 'JEM' is still used to
differentiate the internal elements which are not intended for general
consumption.  Of course, it is possible due to the amount of renaming required
that the term 'JEM' has crept into the external components - if such a case
is found, please notify the author for correction.
