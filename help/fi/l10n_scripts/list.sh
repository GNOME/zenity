#!/bin/sh

zenity --list \
  --title="Valitse viat joita haluat katsoa" \
  --column="Vikanumero" --column="Vakavuus" --column="Kuvaus" \
    992383 normaali "GtkTreeView kaatuu monivalinnoissa" \
    293823 korkea "GNOME-sanakirja ei osaa käyttää välipalvelinta" \
    393823 kriittinen "Valikkojen muokaus ei toimi GNOME 2.0:ssa"
