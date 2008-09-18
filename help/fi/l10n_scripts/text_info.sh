#!/bin/sh

FILE=`zenity --file-selection \
  --title="Valitse tiedosto"`

case $? in
         0)
                zenity --text-info \
                  --title=$FILE \
                  --filename=$FILE \
                  --editable 2&gt;/tmp/tmp.txt;;
         1)
                echo "Tiedostoa ei valittu.";;
        -1)
                echo "Tiedostoa ei valittu.";;
esac
