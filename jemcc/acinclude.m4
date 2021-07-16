dnl # Local m4 definitions to be inserted into aclocal.m4 (through aclocal)
dnl # Copyright (C) 1999-2004 J.M. Heisz 
dnl #
dnl # This library is free software; you can redistribute it and/or
dnl # modify it under the terms of the GNU Lesser General Public
dnl # License as published by the Free Software Foundation; either
dnl # version 2.1 of the License, or (at your option) any later version.
dnl #
dnl # See the file named COPYRIGHT in the root directory of the source
dnl # distribution for specific references to the GNU Lesser General Public
dnl # License, as well as further clarification on your rights to use this
dnl # software.
dnl #
dnl # This library is distributed in the hope that it will be useful,
dnl # but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl # Lesser General Public License for more details.
dnl #
dnl # You should have received a copy of the GNU Lesser General Public
dnl # License along with this library; if not, write to the Free Software
dnl # Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
dnl #

dnl #
dnl # These macros have seen a lot of changes.  Used to scan for installations
dnl # but too many variations of the Java distribution names now.  Essentially
dnl # relies on environment variable settings or the user's path.
dnl #

dnl #
dnl # Macro used to determine Java compilation environment.  This macro
dnl # will find a valid installation of a javac compiler and expand the
dnl # toolset from there.  Defines the JDK_HOME, JAVAC substitutions. If
dnl # other java programs are required (e.g. JAVAH), use the other macros
dnl # to validate/configure those substitutions - this has been split out
dnl # to support the use of programs like jikes or kaffe.
dnl #
dnl # NOTE: this macro also assumes that the JDK contains a JRE instance.  
dnl # As a result, this macro also defines the JRE_HOME and JAVA substitutions
dnl # (no need to use the AC_JAVA_RUNTIME macro below).
dnl #
AC_DEFUN([AC_JAVA_DEVKIT],
[
  AC_ARG_WITH(jdk-home,
  [  --with-jdk-home=DIR     specifies the Java Development Kit root directory],
  [ JDK_HOME=${withval}; JDK_CL="true" ])

  if test -n "${JDK_HOME}"; then
    if test -n "${JDK_CL}"; then
      AC_MSG_CHECKING(the development kit specified in --with-jdk-home)
    else
      AC_MSG_CHECKING(the development kit structure in JDK_HOME)
    fi
    if test -d "${JDK_HOME}/bin" && test -x "${JDK_HOME}/bin/javac"; then
      AC_MSG_RESULT(ok)
    else
      AC_MSG_RESULT(invalid)
      if test -n "${JDK_CL}"; then
        AC_MSG_ERROR([Location specified by --with-jdk-home is invalid.
           Please try again.  JDK home directory should contain bin/javac.])
      else
        AC_MSG_RESULT([Warning: JDK_HOME is invalid, searching elsewhere])
        JDK_HOME=""
      fi
    fi
  fi
  if test -z "${JDK_HOME}"; then
    AC_MSG_CHECKING(environment for a JDK installation)

    if test -n "${JAVA_HOME}" && \
         test -d "${JAVA_HOME}/bin" && \
           test -x "${JAVA_HOME}/bin/javac"; then
      JDK_HOME="${JAVA_HOME}"
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JDK_HOME} using JAVA_HOME)])
    elif test -n "${JAVAC}" && \
           test -x "${JAVAC}"; then
changequote(, )dnl
      # Remove prog and lib subdir (not all systems have dirname)
      j_dir=`echo ${JAVAC} | sed 's%/[^/][^/]*$%%'`
      j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
      JDK_HOME=$j_dir
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JDK_HOME} using JAVAC)])
    elif test -n "${JAVA}" && \
           test -x "${JAVA}"; then
changequote(, )dnl
      # Remove prog and lib subdir (not all systems have dirname)
      j_dir=`echo ${JAVA} | sed 's%/[^/][^/]*$%%'`
      j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
      JDK_HOME=$j_dir
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JDK_HOME} using JAVA)])
    else
      AC_MSG_RESULT([not found, checking path])
      AC_PATH_PROG(JAVA_PROG, javac)
      if test -n "${JAVA_PROG}"; then
changequote(, )dnl
        # Remove prog and lib subdir (not all systems have dirname)
        j_dir=`echo ${JAVA_PROG} | sed 's%/[^/][^/]*$%%'`
        j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
        JDK_HOME=$j_dir
        AC_MSG_RESULT(using JDK_HOME of ${JDK_HOME} (from path))
      else
        AC_PATH_PROG(JAVA_PROG, java)
        if test -n "${JAVA_PROG}"; then
changequote(, )dnl
          # Remove prog and lib subdir (not all systems have dirname)
          j_dir=`echo ${JAVA_PROG} | sed 's%/[^/][^/]*$%%'`
          j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
          JDK_HOME=$j_dir
          AC_MSG_RESULT(using JDK_HOME of ${JDK_HOME} (from path))
        fi
      fi
    fi
  fi

  if test "${JDK_HOME}" = ""; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR([Unable to locate valid Java Development Kit installation.
           Use --with-jdk-home option to specify valid Java package location.])
  fi

  AC_SUBST(JDK_HOME)
  JRE_HOME="${JDK_HOME}"
  AC_SUBST(JRE_HOME)

  AC_MSG_CHECKING(for provided java executable)
  if test -n "${JAVA}" && \
       test -x "${JAVA}"; then
    AC_MSG_RESULT(found in environment)
  elif test -x "${JDK_HOME}/bin/java"; then
    AC_MSG_RESULT(found in JDK_HOME)
    JAVA="${JDK_HOME}/bin/java"
  else
    AC_MSG_RESULT(missing)
    AC_MSG_ERROR([Cannot find 'java' program.  Check installation or specify
                  location explicitly using JAVA environment variable.])
  fi
  j_ver=`$JAVA -version 2>&1`
  AC_MSG_RESULT(Using java version: $j_ver)
  AC_SUBST(JAVA)

  AC_MSG_CHECKING(for provided javac executable)
  if test -n "${JAVAC}" && \
       test -x "${JAVAC}"; then
    AC_MSG_RESULT(found in environment)
  elif test -x "${JDK_HOME}/bin/javac"; then
    AC_MSG_RESULT(found in JDK_HOME)
    JAVAC="${JDK_HOME}/bin/javac"
  else
    AC_MSG_RESULT(missing)
    AC_MSG_ERROR([Cannot find 'javac' program.  Check installation or specify
                  location explicitly using JAVAC environment variable.])
  fi
  AC_SUBST(JAVAC)
])

dnl #
dnl # Macro used to detect and validate the presence of the javah program.
dnl # Will define JAVAH substitution if found.  AC_JAVA_DEVKIT should be
dnl # defined first, but is not absolutely necessary if JDK_HOME or similar
dnl # definition is given.
dnl #
AC_DEFUN([AC_JAVA_H],
[
  if test -z "${JDK_HOME}"; then
    CJAVAH="C_JAVA_H"
    CDEVKIT="C_JAVA_DEVKIT"
    AC_MSG_ERROR([A${CJAVAH} called without JDK_HOME determined.
           Please use the A${CDEVKIT} macro or explicitly define JDK_HOME])
  fi

  AC_MSG_CHECKING(for provided javah executable)
  if test -n "${JAVAH}" && \
       test -x "${JAVAH}"; then
    AC_MSG_RESULT(found in environment)
  elif test -x "${JDK_HOME}/bin/javah"; then
    AC_MSG_RESULT(found in JDK_HOME)
    JAVAH="${JDK_HOME}/bin/javah"
  else
    AC_MSG_RESULT([not found, checking path])
    AC_PATH_PROG(JAVA_PROG, javah)
    if test -n "${JAVA_PROG}"; then
      JAVAH="$JAVA_PROG"
    else
      AC_MSG_ERROR([Cannot find 'javah' program.  Check installation or specify
                  location explicitly using JAVAH environment variable.])
    fi
  fi
  AC_SUBST(JAVAH)
])

dnl #
dnl # Macro used to detect and validate the presence of the javadoc program.
dnl # Will define JAVADOC substitution if found.  AC_JAVA_DEVKIT should be
dnl # defined first, but is not absolutely necessary if JDK_HOME or similar
dnl # definition is given.
dnl #
AC_DEFUN([AC_JAVA_DOC],
[
  if test -z "${JDK_HOME}"; then
    CJAVADOC="C_JAVA_DOC"
    CDEVKIT="C_JAVA_DEVKIT"
    AC_MSG_ERROR([A${CJAVADOC} called without JDK_HOME determined.
           Please use the A${CDEVKIT} macro or explicitly define JDK_HOME])
  fi

  AC_MSG_CHECKING(for provided javadoc executable)
  if test -n "${JAVADOC}" && \
       test -x "${JAVADOC}"; then
    AC_MSG_RESULT(found in environment)
  elif test -x "${JDK_HOME}/bin/javadoc"; then
    AC_MSG_RESULT(found in JDK_HOME)
    JAVAH="${JDK_HOME}/bin/javadoc"
  else
    AC_MSG_RESULT([not found, checking path])
    AC_PATH_PROG(JAVA_PROG, javadoc)
    if test -n "${JAVA_PROG}"; then
      JAVADOC="$JAVA_PROG"
    else
      AC_MSG_ERROR([Cannot find 'javadoc' program.  Check setup or specify
                  location explicitly using JAVAH environment variable.])
    fi
  fi
  AC_SUBST(JAVADOC)
])

dnl #
dnl # Macro used to determine Java runtime environment.  This macro will
dnl # will find a valid installation of a java executable and expand the
dnl # toolset from there.  Defines the JRE_HOME and JAVA substitutions.
dnl #
AC_DEFUN([AC_JAVA_RUNTIME],
[
  AC_ARG_WITH(jre-home,
  [  --with-jre-home=DIR     specifies the Java installation root directory],
  [ JRE_HOME=${withval}; JRE_CL="true" ])

  if test -n "${JRE_HOME}"; then
    if test -n "${JRE_CL}"; then
      AC_MSG_CHECKING(the Java installation specified in --with-jre-home)
    else
      AC_MSG_CHECKING(the Java installation structure in JRE_HOME)
    fi
    if test -d "${JRE_HOME}/bin" && test -x "${JRE_HOME}/bin/java"; then
      AC_MSG_RESULT(ok)
    else
      AC_MSG_RESULT(invalid)
      if test -n "${JRE_CL}"; then
        AC_MSG_ERROR([Location specified by --with-jre-home is invalid.
           Please try again.  JRE home directory should contain bin/java.])
      else
        AC_MSG_RESULT([Warning: JRE_HOME is invalid, searching elsewhere])
        JRE_HOME=""
      fi
    fi
  fi
  if test -z "${JRE_HOME}"; then
    AC_MSG_CHECKING(environment for a JRE installation)

    if test -n "${JAVA_HOME}" && \
         test -d "${JAVA_HOME}/bin" && \
           test -x "${JAVA_HOME}/bin/java"; then
      JRE_HOME="${JAVA_HOME}"
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JRE_HOME} using JAVA_HOME)])
    elif test -n "${JAVA}" && \
           test -x "${JAVA}"; then
changequote(, )dnl
      # Remove prog and lib subdir (not all systems have dirname)
      j_dir=`echo ${JAVA} | sed 's%/[^/][^/]*$%%'`
      j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
      JRE_HOME=$j_dir
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JRE_HOME} using JAVA)])
    elif test -n "${JAVAC}" && \
           test -x "${JAVAC}"; then
changequote(, )dnl
      # Remove prog and lib subdir (not all systems have dirname)
      j_dir=`echo ${JAVAC} | sed 's%/[^/][^/]*$%%'`
      j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
      JRE_HOME=$j_dir
      AC_MSG_RESULT(found)
      AC_MSG_RESULT([      (in ${JRE_HOME} using JAVAC)])
    else
      AC_MSG_RESULT([not found, checking path])
      AC_PATH_PROG(JAVA_PROG, java)
      if test -n "${JAVA_PROG}"; then
changequote(, )dnl
        # Remove prog and lib subdir (not all systems have dirname)
        j_dir=`echo ${JAVA_PROG} | sed 's%/[^/][^/]*$%%'`
        j_dir=`echo $j_dir | sed 's%/bin*$%%'`
changequote([, ])dnl
        JRE_HOME=$j_dir
        JAVA="${JAVA_PROG}"
        AC_MSG_RESULT(using JRE_HOME of ${JRE_HOME} (from path))
      fi
    fi
  fi

  if test "${JRE_HOME}" = ""; then
    AC_MSG_RESULT(not found)
    AC_MSG_ERROR([Unable to locate valid Java runtime installation.
           Use --with-jre-home option to specify valid Java package location.])
  fi

  AC_SUBST(JRE_HOME)

  AC_MSG_CHECKING(for provided java executable)
  if test -n "${JAVA}" && \
       test -x "${JAVA}"; then
    AC_MSG_RESULT(found in environment)
  elif test -x "${JRE_HOME}/bin/java"; then
    AC_MSG_RESULT(found in JRE_HOME)
    JAVA="${JRE_HOME}/bin/java"
  else
    AC_MSG_RESULT(missing)
    AC_MSG_ERROR([Cannot find 'java' program.  Check installation or specify
                  location explicitly using JAVA environment variable.])
  fi
  j_ver=`$JAVA -version 2>&1`
  AC_MSG_RESULT(Using java version: $j_ver)
  AC_SUBST(JAVA)
])
