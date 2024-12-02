#!/bin/bash

# This script will be called by joj at the last stage
# It will run at the root directory of the project
# Please write commands to compile your code and run your own tests here
# The sample files can be found in /opt/lemondb, same as the server
# You can make symlinks to these files by ln -s /opt/lemondb/db db
# If you think your code makes mistakes, you can return non-zero value to have a cross mark
# The comments can be removed as you wish 

# tests_arr=(few_read few_insert_delete few_read_dup few_read_update many_insert_delete many_read_dup many_read_update many_read single_insert_delete single_read_dup single_read_update single_read)
tests_arr=(few_read)
#in src
mkdir build
cd build
# pwd
ln -s /opt/lemondb/db ../db
ln -s /opt/lemondb/sample ../sample #Or any other soft links, do as you wish
# comment out building with cmake
cmake -DCMAKE_C_COMPILER=/usr/bin/clang-18 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-18 ../src #Change clang++ to clang++-16
cmake --build . -- -j8
# ls
#Use 'time' to count time cost
for test in "${tests_arr[@]}";
do
    echo $test
    time ./lemondb --listen ../sample/$test.query  > test.out
    #time ./lemondb --listen ../sample/many_read_dup.query  > test.out

    #Use diff to compare the output correctness

    #usage of htop on the server
    #htop
    #Test baseline
    # time /opt/lemondb/bin/lemondb --listen /opt/lemondb/sample/$test.query > test_standard.out
    diff test.out /opt/lemondb/sample_stdout/$test.out
    # diff test.out /opt/lemondb/sample_stdout/many_read_dup.out
    rm *.tbl
done
# echo "Hint: No tests provided. Please modify test/run.sh to add your own tests."
# exit -1
