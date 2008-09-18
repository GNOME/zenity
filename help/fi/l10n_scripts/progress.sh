#!/bin/sh
(
echo "10" ; sleep 1
echo "# Päivitetään postilokeja" ; sleep 1
echo "20" ; sleep 1
echo "# Nollataan cron-töitä" ; sleep 1
echo "50" ; sleep 1
echo "Tämä rivi vain ohitetaan" ; sleep 1
echo "75" ; sleep 1
echo "# Uudelleenkäynnistetään järjestelmä" ; sleep 1
echo "100" ; sleep 1
) |
zenity --progress \
  --title="Päivitetään järjestelmälokeja" \
  --text="Tutkitaan postilokeja..." \
  --percentage=0

if [ "$?" = -1 ] ; then
        zenity --error \
          --text="Päivitys keskeytetty."
fi
