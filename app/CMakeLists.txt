# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             rtd

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             #src/main/cpp/naviMain.cpp
             #src/main/cpp/naviMain.h
             src/main/cpp/phoneObsData.cpp
             src/main/cpp/phoneObsData.h
             src/main/cpp/native-lib.cpp
             src/main/cpp/rtklib.h
             src/main/cpp/rtdlib.h
             src/main/cpp/rtdlib.cpp
             src/main/cpp/binex.c
             src/main/cpp/cmr.c
             src/main/cpp/convgpx.c
             src/main/cpp/convkml.c
             src/main/cpp/convrnx.c
             src/main/cpp/crescent.c
             src/main/cpp/datum.c
             src/main/cpp/download.c
             src/main/cpp/ephemeris.c
             src/main/cpp/geoid.c
             src/main/cpp/gis.c
             src/main/cpp/gw10.c
             src/main/cpp/ionex.c
             src/main/cpp/javad.c
             src/main/cpp/lambda.c
             src/main/cpp/novatel.c
             src/main/cpp/nvs.c
             src/main/cpp/options.c
             src/main/cpp/pntpos.c
             src/main/cpp/postpos.c
             src/main/cpp/ppp.c
             src/main/cpp/ppp_ar.c
             src/main/cpp/ppp_corr.c
             src/main/cpp/preceph.c
             src/main/cpp/qzslex.c
             src/main/cpp/rcvlex.c
             src/main/cpp/rcvraw.c
             src/main/cpp/rinex.c
             src/main/cpp/rt17.c
             src/main/cpp/rtcm.c
             src/main/cpp/rtcm2.c
             src/main/cpp/rtcm3.c
             src/main/cpp/rtcm3e.c
             src/main/cpp/rtkcmn.c
             src/main/cpp/rtkpos.c
             src/main/cpp/rtksvr.c
             src/main/cpp/sbas.c
             src/main/cpp/septentrio.c
             src/main/cpp/skytraq.c
             src/main/cpp/solution.c
             src/main/cpp/ss2.c
             src/main/cpp/stream.c
             src/main/cpp/streamsvr.c
             src/main/cpp/tersus.c
             src/main/cpp/tides.c
             src/main/cpp/tle.c
             src/main/cpp/ublox.c
             )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       rtd

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib})
