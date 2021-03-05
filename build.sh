#! /bin/bash

BUILD_TYPE="Debug"
BUILD_DOC="OFF"
BUILD_SHARED_LIBS="OFF"

function usage() {
	echo "build.sh [release | debug | doc]"
}

case ${1} in
  "release")
  BUILD_TYPE="Release"
  BUILD_SHARED_LIBS="ON"
  ;;
  
  "debug")
  BUILD_TYPE="Debug"
  BUILD_SHARED_LIBS="ON"
  ;;

  "doc")
  BUILD_TYPE="Release"
  BUILD_DOC="ON"
  ;;

  "help")
  usage
  ;;

  "")
  BUILD_TYPE="Debug"
  ;;
  
  *)
  echo "Invalid Argument."
 	usage
  ;;

esac


[ -d build ] || mkdir build
# rm -rf build/*
cd build
 



# -DCMAKE_BUILD_TYPE=Debug | Release
# -DBUILD_SHARED_LIBS=ON
# -DCMAKE_INSTALL_PREFIX=$(pwd/../dest)
# -DQCA_MAN_INSTALL_DIR:PATH=/usr/share/man 
cmake -DCMAKE_INSTALL_PREFIX="$(pwd)/../dest" \
  -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} \
  -DBUILD_DOC=${BUILD_DOC} ..

cmake --build .

cpack

# cmake --build . --target install
 
