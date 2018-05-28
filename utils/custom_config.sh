#!/bin/bash

DummyDevice="off"
MockProducer="off"
ESSStream="off"
DetectorIndex="off"

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../source" && pwd )/config"
FILE=$DIR"/CMakeLists.txt"

mkdir -p $DIR

if [ ! -f ${FILE} ]; then
  MockProducer="on"
else
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

cmd2=(--title Producers --checklist "Build the following data producer plugins:" 14 60 16)
options2=(
         1 "Dummy device" "$DummyDevice"
         2 "Mock producer" "$MockProducer"
         3 "ESS Stream" "$ESSStream"
         4 "Detector Index" "$DetectorIndex"
        )

cmd=(dialog --backtitle "DAQuiri BUILD OPTIONS" --separate-output)
choices=$("${cmd[@]}" "${cmd2[@]}" "${options2[@]}" 2>&1 >/dev/tty)

if test $? -ne 0
then
  clear 
  exit
fi

clear

text=$'set(DAQuiri_config TRUE PARENT_SCOPE)\n'
for choice in $choices
do
    case $choice in
        1)
            text+=$'list(APPEND DAQuiri_enabled_producers DummyDevice)\n'
            ;;
        2)
            text+=$'list(APPEND DAQuiri_enabled_producers MockProducer)\n'
            ;;
        3)
            text+=$'list(APPEND DAQuiri_enabled_producers ESSStream)\n'
            ;;
        4)
            text+=$'list(APPEND DAQuiri_enabled_producers DetectorIndex)\n'
            ;;
    esac
done

text+=$'set(DAQuiri_enabled_producers ${DAQuiri_enabled_producers} PARENT_SCOPE)\n'

touch $FILE
printf '%s\n' "$text" > ${FILE}

