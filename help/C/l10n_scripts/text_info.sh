#!/bin/sh

FILE=`zenity --file-selection \
  --title="Select a File"`

case $? in
         0)
                zenity --text-info \
                  --title=$FILE \
                  --filename=$FILE \
                  --editable 2>/tmp/tmp.txt;;
         1)
                echo "No file selected.";;
        -1)
                echo "No file selected.";;
esac

