# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-src")
  file(MAKE_DIRECTORY "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-src")
endif()
file(MAKE_DIRECTORY
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-build"
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix"
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/tmp"
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/src/ldtkloader-populate-stamp"
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/src"
  "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/src/ldtkloader-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/src/ldtkloader-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Real-Time Game Server ECS (FPS-lite Arena)/build/_deps/ldtkloader-subbuild/ldtkloader-populate-prefix/src/ldtkloader-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
