#!/bin/bash

cmd="off"
gui="off"

DummyDevice="off"
MockProducer="off"
ESSStream="off"
DetectorIndex="off"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../source" && pwd )/config"
FILE=$DIR"/CMakeLists.txt"

mkdir -p $DIR

if [ ! -f ${FILE} ]; then
  MockProducer="on"
  gui="on"
else
  if grep -q DAQuiri_cmd ${FILE}; then
    cmd="on"
  fi
  if grep -q DAQuiri_gui ${FILE}; then
    gui="on"
  fi
  if grep -q DummyDevice ${FILE}; then
    DummyDevice="on"
  fi
  if grep -q MockProducer ${FILE}; then
    MockProducer="on"
  fi
  if grep -q ESSStream ${FILE}; then
    ESSStream="on"
  fi
  if grep -q DetectorIndex ${FILE}; then
    DetectorIndex="on"
  fi
fi

cmd1=(--title Options --checklist "Base program options:" 10 60 16)
options1=(
         1 "DAQuiri Graphical Interface" "$gui"
         2 "Command line tool" "$cmd"
        )

cmd2=(--and-widget --title Producers --checklist "Build the following data producer plugins:" 14 60 16)
options2=(
         3 "Dummy device" "$DummyDevice"
         4 "Mock producer" "$MockProducer"
         5 "ESS Stream" "$ESSStream"
         6 "Detector Index" "$DetectorIndex"
        )

cmd=(dialog --backtitle "DAQuiri BUILD OPTIONS" --separate-output)
choices=$("${cmd[@]}" "${cmd1[@]}" "${options1[@]}" "${cmd2[@]}" "${options2[@]}" "${cmd3[@]}" "${options3[@]}" 2>&1 >/dev/tty)

if test $? -ne 0
then
  clear 
  exit
fi

clear

text=''
for choice in $choices
do
    case $choice in
        1)
            text+=$'set(DAQuiri_gui TRUE PARENT_SCOPE)\n'
            ;;
        2)
            text+=$'set(DAQuiri_cmd TRUE PARENT_SCOPE)\n'
            ;;
        3)
            text+=$'list(APPEND DAQuiri_enabled_producers DummyDevice)\n'
            ;;
        4)
            text+=$'list(APPEND DAQuiri_enabled_producers MockProducer)\n'
            ;;
        5)
            text+=$'list(APPEND DAQuiri_enabled_producers ESSStream)\n'
            ;;
        6)
            text+=$'list(APPEND DAQuiri_enabled_producers DetectorIndex)\n'
            ;;
    esac
done

text+=$'set(DAQuiri_enabled_producers ${DAQuiri_enabled_producers} PARENT_SCOPE)\n'

touch $FILE
printf '%s\n' "$text" > ${FILE}

