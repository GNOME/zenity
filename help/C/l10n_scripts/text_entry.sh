#!/bin/sh

                if zenity --entry \
                --title="Add an Entry" \
                --text="Enter your _password:" \
                --entry-text "password" \
                --hide-text
                  then echo $?
                  else echo "No password entered"
                fi

