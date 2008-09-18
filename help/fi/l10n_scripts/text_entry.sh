#!/bin/sh

if zenity --entry \
    --title="Lisää syöte" \
    --text="Syötä _salasanasi:" \
    --entry-text "salasana" \
    --hide-text; then
    echo $?
else
    echo "Salasanaa ei syötetty"
fi
