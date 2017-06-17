# katas
A collection of simple C/C++ exercises.
Problems: https://codility.com/programmers/lessons/

## How to compile and run *katas*

    cd /path/to/my/build/folder
    cmake /path/to/my/source/folder
    make
    ./target


## Dev env set up
### Requirements (proven environment)
* AppleClang 7.3.0 (Xcode 7.3)
* cmake 3.5.1

### Set up Dev env C++ in OS X El capitan
* open terminal and type 'gcc', then it will ask to install command-line developer tools
* install MacPorts: https://www.macports.org/install.php (should be located at /opt/local/bin

### Install googletest libraries:
* go to a folder where to download de source code. eg: gtest

        cd gtest
        git clone https://github.com/google/googletest.git
       
* build googletest

        cd googletest
        mkdir install
        cd install
        cmake -DCMAKE_CXX_COMPILER="c++" -DCMAKE_CXX_FLAGS="-std=c++11 -stdlib=libc++" ../ 
        make
        sudo make install

* in Mac the library would be located at: /usr/local/include/gtest
* Do the same to build **googlemoc**

### Install  CGAL libraries:
* install libs from MacPorts

        sudo port install cgal
        sudo port install cgal +qt5 +universal +demos # with the demos

* download CGAL source from https://github.com/CGAL/cgal/releases, and uncompress it to: /opt/local/CGAL-4.7
* open cmake-gui 3.5 (http://www.cmake.org/) use Unix Makefiles
    * Where is the source code: /opt/local/CGAL-4.7
    * Where to build the binaries: /Users/xxx/CGAL-4.7
    * Configure and Generate
* open terminal in /Users/xxx/CGAL-4.7
* make examples
* make demos (Qt5 need it)
