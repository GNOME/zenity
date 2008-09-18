#!/bin/sh

FILE=`zenity --file-selection --title="Valitse tiedosto"`

case $? in
         0)
                echo "\"$FILE\" valittu.";;
         1)
                echo "Tiedostoa ei valittu.";;
        -1)
                echo "Tiedostoa ei valittu.";;
esac

