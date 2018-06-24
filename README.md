# Rebours

Rebours is an atrificial computer platform designed for automated analysis
of executable programs of real computer platforms. The main focus is on
control-flow recovery in self-rewriting executables.

## Install guide

### License

Rebours is a freely available open source software. You can use source code,
built binaries, and produced outputs at will. In particular, you can copy,
modify, sell, or otherwise distribute source code, binaries, and output from
Rebours. You can even claim that you are the author of the possesed software.
On the other hand, you take ALL responsibilities. It in particular means that
it is only YOU to blaim if ANYTHING unwanted or unexpected happens while ANYHOW
using the original or modified source code, binaries, or produced outputs.

### Minimal system requirements 

The computer with a 64bit processor, 4GB free RAM, 20GB free disk space. 

Rebours can be built and run on MS Windows 7, Ubuntu 16.04, or Linux Mint
18.03, or later versions.

### Software depndences

Development toolkits:
    - C++ compiler supporting C++11 standard; namely,
        - MS Visual Studio 14 2015 or later on MS Windows
        - GCC 5.0 or later on Linux
    - CMake build system 2.8 or later
    - Git version control system
    - Graphviz
    - Gnuplot
    - (optional) LaTeX system (for building documentation)
    - (optional) Python 3 (for automated running of tests)

3rd party libraries:
    - Boost C++ libraries (http://www.boost.org)
    - Capstone (https://github.com/aquynh/capstone.git ; branch 'next')
    - Z3 SMT solver
    - (optional) MATHSAT5 SMT solver (for boosting SAT/SMT queries)
    - (optional) Boolector SAT solver (for boosting SAT queries)

### Downloading Rebours

Type the folling commands into a console: 

    mkdir $REBOURS-INSTALL-DIR
    cd $REBOURS-INSTALL-DIR
    git clone https://github.com/trtikm/Rebours.git .

where $REBOURS-INSTALL-DIR is a symbol we used to denote a directory, where
you want to install Rebours.

### Building Rebourse on MS Windows

Type the folling commands into a console of MS Visual Studio x64: 

    cd $REBOURS-INSTALL-DIR
    mkdir build
    cd build
    mkdir $BUILD-TYPE
    cd $BUILD-TYPE
    cmake ../.. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=$BUILD-TYPE -DBOOST_INSTALL_DIR=$BOOST-INSTALL-DIR -DCAPSTONE_NEXT_ROOT=$CAPSTONE-NEXT-ROOT -DZ3_ROOT=$Z3-ROOT
    nmake install

where we used the following symbols (they must be  substituted by proper values):
    - $REBOURS-INSTALL-DIR: The install directory of the Capstone framework.
    - $BUILD-TYPE: Can be one of these keywords: Debug, RelWithDebInfo, Release.
    - $BOOST-INSTALL-DIR: The install directory of Boost C++ libraries.
        - NOTE: In case you installed Boost at the standard/default place on your
                system, then the library will be automatically detected, i.e. the
                assignment -DBOOST_INSTALL_DIR=$BOOST-INSTALL-DIR can be omitted
                in the statement above.
    - $CAPSTONE-NEXT-ROOT: The install directory of Capstone framework.
    - $Z3-ROOT: The install directory of the Z3 SMT solver.

Optionally, you can extend the 'cmake' command by these assignments:

    -DINCLUDE_TESTS=$INCLUDE-TESTS -DMATHSAT5_ROOT=$MATHSAT5-ROOT -DBOOLECTOR_ROOT=$BOOLECTOR-ROOT 
    
where we used the following symbols (they must be  substituted by proper values):
    - $INCLUDE-TESTS: Do you want to build tests? Possible values are: yes, no (default is yes).
    - $MATHSAT5-ROOT:  The install directory of the MATHSAT5 SMT solver.
    - $BOOLECTOR-ROOT: The install directory of the BOOLECTOR SAT solver.

In case you would prefer to build Rebours in MS Visual Studio, then replace
the last four commands above by these two:

    cmake .. -G "Visual Studio $VS-VERSION $VS-YEAR Win64" -DCMAKE_BUILD_TYPE=$BUILD-TYPE -DBOOST_INSTALL_DIR=$BOOST-INSTALL-DIR -DCAPSTONE_NEXT_ROOT=$CAPSTONE-NEXT-ROOT -DZ3_ROOT=$Z3-ROOT
    rebours.sln

where we used the following symbols (they must be  substituted by proper values):
    - $VS-VERSION: A version of the MS Visual Studio to be used.
    - $VS-YEAR: A year of the MS Visual Studio to be used.
If you do not know what is the version or year of your Visual Studio, then type
this command this command into the console and look to section 'Generators':
    cmake --help

Finally, in case you would prefer to build Rebours in QT Creator, then instead of performing
commands above, simply open the file '$REBOURS-INSTALL-DIR/CMakeLists.txt' in QT Creator and
set values, like $CAPSTONE-NEXT-ROOT, to corresponding CMake variables inside QT Creator.

### Building Rebourse on Linux

Type the folling commands into a console: 

    cd $REBOURS-INSTALL-DIR
    git clone https://github.com/trtikm/Rebours.git .
    mkdir build
    cd build
    mkdir $BUILD-TYPE
    cd $BUILD-TYPE
    cmake ../.. -DCMAKE_BUILD_TYPE=$BUILD-TYPE -DBOOST_INSTALL_DIR=$BOOST-INSTALL-DIR -DCAPSTONE_NEXT_ROOT=$CAPSTONE-NEXT-ROOT -DZ3_ROOT=$Z3-ROOT
    make install

where we used the following symbols (they must be  substituted by proper values):
    - $REBOURS-INSTALL-DIR: The install directory of the Capstone framework.
    - $BUILD-TYPE: Can be one of these keywords: Debug, RelWithDebInfo, Release.
    - $BOOST-INSTALL-DIR: The install directory of Boost C++ libraries.
        - NOTE: In case you installed Boost at the standard/default place on your
                system, then the library will be automatically detected, i.e. the
                assignment -DBOOST_INSTALL_DIR=$BOOST-INSTALL-DIR can be omitted
                in the statement above.
    - $CAPSTONE-NEXT-ROOT: The install directory of Capstone framework.
    - $Z3-ROOT: The install directory of the Z3 SMT solver.

Optionally, you can extend the 'cmake' command by these assignments:

    -DINCLUDE_TESTS=$INCLUDE-TESTS -DMATHSAT5_ROOT=$MATHSAT5-ROOT -DBOOLECTOR_ROOT=$BOOLECTOR-ROOT 
    
where we used the following symbols (they must be  substituted by proper values):
    - $INCLUDE-TESTS: Do you want to build tests? Possible values are: yes, no (default is yes).
    - $MATHSAT5-ROOT:  The install directory of the MATHSAT5 SMT solver.
    - $BOOLECTOR-ROOT: The install directory of the BOOLECTOR SAT solver.

Finally, in case you would prefer to build Rebours in QT Creator, then instead of performing
commands above, simply open the file '$REBOURS-INSTALL-DIR/CMakeLists.txt' in QT Creator and
set values, like $CAPSTONE-NEXT-ROOT, to corresponding CMake variables inside QT Creator.


## Usage

After successfull build of Rebours, distribution (deployment) directory, located by default at
$REBOURS-INSTALL-DIR/dist, has the following structure:

TODO!
