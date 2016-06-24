#!/bin/bash
#first argument is the test and file name
TEST=$1
#second argument is the line to append
APPEND=$2
#The file will be added to /home/$USER/testPDFs/ folder
if (($# < 2 )); then
    echo "No all the input parameters where supplied:";
    echo "1st parameter is test and file name";
    echo "2nd parameter is line to append";
    echo "All the parameters without spaces.";
    exit 0
fi
#and his name will be $TEST_timestamp.pdf
FILE=/home/$USER/JUSTINA/catkin_ws/src/vision/vision_export/temp/$TEST.tex;
echo $APPEND "\\\\" >> $FILE;
exit 0;
