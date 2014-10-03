SD386 Version 5.00
------------------
  - This is the final SD386 release.
  - This release includes all source.
  - The primary improvements in this release are:

     - C++ support.
     - TCP/IP support.
     - support for multiple code segment source files.

 See the beta release info below for details.


SD386 Version 4.04
------------------
 Here's what you can now do with the addition of TCP/IP support:

 - Remote debug a process over a network TCP/IP connection.

 - Remote debug a child process over a network TCP/IP connection.

 - Remote debug the processes of a multiple process application
   using a remote TCP/IP connection. This is NOT an attach/detach
   methodology. You will get an independent debugger window for
   each process of the application. A debugger window will be
   automatically spawned each time a new process begins.

 - Ctrl-Break Support for all of the above configurations.

 - DosExecPgm() Support for all of the above connections.

 - Bug fixes.
   - fix for restoring breakpoint file on every step/go after a save.
   - fix for trap when setting breakpoint by name.

 TCP/IP Setup
 ------------

 There are some things you will need to do before using the
 TCP/IP support.

 - You need to have TCP/IP installed and running.

 - You need to specify a port number in your SERVICES file.  You
   should be able to find this file in the \MPTN\ETC subdirectory
   on the drive where OS/2 is installed.  The entry should look
   like:

    sd386     4321/tcp

   where 4321 is the port number and tcp is the protocol.  You can
   use whatever port number you wish as long as there is no
   conflict with other well-known services.  Simply browse
   SERVICES and find an available port number.  Use lower case.

   You need to do this on both the SD386 and ESP sides of the
   connection.


-  On the SD386 end of the connection, you need to specify a host
   name in your HOSTS file.  You should be able to find this file
   in the \MPTN\ETC subdirectory on the drive where OS/2 is
   installed.  The entry should look like:

   9.51.136.80  clements.cv.lexington.ibm.com  elvis

   where elvis is the alias for the foreign address specified by
   the dotted decimal/name addresses.  Use whatever alias name you
   wish.


 Example Invocation
 ------------------

 A typical invocation will look like so:

  On the SD386 side of               On the ESP side of
  the connection.                    the connection.

  sd386 /telvis mypgm.exe            esp /t

  where elvis is the name of the remote machine.


SD386 Version 4.03
------------------
 This README.DOC contains the information for the changes made for
 SD386 Version 4.03.  The SD386.INF and SD386 LIST3820 have not
 been updated from previous versions. The only files that have
 changed are SD386.EXE, ESP.EXE, and this README.DOC.

 This following enhancements provide some of the fundamental
 support needed for c++.  This version only explicitly supports
 VAC++ symbols.  (I have assumed that C-Set++ and VAC++ symbols
 are the same.  So you should be able to debug C-Set++ as well.)

 c vs c++ support.
 ----------------

 SD386 does not require any special options for c++. There are no
 additional dlls or installation required.  Simply toss it in your
 tools directory and use it the same way you did for c.

 c++ support.
 -----------

 Here's basically what you can do:

  - put your cursor on an object name(instance of a class) and hit
    the Ctrl-Ins to put the object into the data window.  (This is
    the same as structures in c.)  You can then go to the data
    window(F2) and move the cursor to a member and expand it again
    just as you did in c. Placing the cursor on a data member will
    expand the data and placing the cursor on a method will take
    you to the method.

  - In the source view, put your cursor on an object name(instance
    of a class) and hit the ENTER key to go into browse mode. You
    can then expand data members or go to methods just as you do
    in the data window.

  - When you are in the implementation file of a class put the
    cursor on a class member name hit ENTER(or INS) to view. The
    "this" ptr is implicit. To see the "this" ptr simply move the
    cursor to a blank space and hit ENTER. You will see the "this"
    ptr. Simply keep hitting ENTER to follow it wherever you want
    to go.

  - You can now use wide column vio modes such as 132x50.

  - You will now see unmangled names in the call stack.


 Here are some issues that haven't been resolved:

  - GetFunction, FindFunction, and SetFunctionNameBreak are not
    very functional due to the c++ name mangling. This issue will
    be addressed later if there is a need. At this point, expand
    the object and move your cursor to the method you want to go
    to and hit enter, or use GetFile.

 c++ object formatting.
 ---------------------

    When you show an object the object name will be formatted as
    follows:

    object_name   class : public derived_class
                        : public derived_class
                        : public derived_class

    Multiple derived classes will be shown for multiple
    inheritance. To browse a derived class, simply put the cursor
    on the derived class and hit ENTER.

 c++ member formatting.
 ---------------------

    c++ members will be formatted as follows:

    prefix    method/data member         data

    prefix
    ------

      The prefixes for data members are as follows:

       "Pri"  - Private
       "Pro"  - Protected
       "Pub"  - Public

       "St"   - Static
       "Vt"   - Virtual table ptr
       "Vb"   - Virtual base  ptr
       "Co"   - Const
       "Vo"   - Volatile
       "Se"   - Self-ptr

      The prefixes for methods are as follows:

       "Pri"  - Private
       "Pro"  - Protected
       "Pub"  - Public

       "Ct"   - Constructor
       "Dt"   - Destructor

       "St"   - Static
       "In"   - Inline
       "Co"   - Const
       "Vo"   - Volatile
       "Vir"  - Virtual

      The prefixes for friends are as follows:

       "FriendFnc" - Friend function
       "FriendCls" - Friend class

    method/data
    -----------

       Methods are formatted as prototypes. These can be very long
       which is the reason wide columns are now supported.

       Data members may have suffixes attached to the member names
       as follows:

         "...object"
         "...reference"
         "...typedef"
         "...enum"

       To expand an object or reference simply put the cursor on
       it and hit ENTER. The typedefs and enums let you see the
       names of these items in the class.

=======================================================================
SD386 Version 4.00/4.01
------------------------

 This following enhancements provide some of the fundamental
 support needed for c++ as well as improvements for c development.

Support for Source code in include files
----------------------------------------

 You can now use include files just as you would any other source
 file. You can get file views, set breakpoints, step into code
 blocks, save and restore breakpoints, etc. This support has been
 made backward compatible to all the toolsets that have been
 supported by SD386 including IBM C/2, Microsoft C6.00, C-Set/2,
 C-Set++, and the current VAC++.

 ( You will also be able to see some of your c++ variables in
   class files but they will be unformatted. Support for symbols
   will come later. )

Support for the Alloctext Pragma
--------------------------------

 The Alloctext Pragma is used to force functions into specific
 segments.  One of its primary uses is for page tuning
 applications.  Prior to this release SD386 only supported one
 code segment per compile unit.  Now, support is provided for
 multiple code sections.  This support has been made backward
 compatible to 16 bit tools including IBM C/2 and Microsoft C6.00
 and also to 32 bit tools including IBM C-Set/2 and IBM C-Set++.
 You can also debug assembler code at source level with multiple
 code segments.  This maybe be useful for anyone writing 32-16
 thunks.

 There are a couple of known problems:

  - If you're using IBM C/2 then use the LINK.EXE that comes with
    OS/2.  The one that comes with the toolset doesn't produce
    correct debug info.

  - C-Set/2 supports the alloctext pragma;however, the debug
    information generated when the alloctext is used is corrupt.
    The best thing to do in this case is move to the VAC++ level.


Breakpoint Pulldown Enhancements
--------------------------------

 Breakpoints set by function name will now be saved by function
 name.  Prior releases converted function names to a source
 file-line number format causing the breakpoint to go out of sync
 with the source if changes were made to the source file.

 There have been some additional miscellaneous fixes for
 breakpoint saving and restoring.
