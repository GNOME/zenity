#!/bin/sh


if zenity --calendar \
--title="Valitse päivä" \
--text="Napsauta päivämäärää valitsemaan päivä." \
--day=10 --month=8 --year=2004
  then echo $?
  else echo "Päivää ei valittu"
fi
