#!/bin/bash

if ! [[ -z $DAQUIRI_KAFKA_BROKER ]]; then
    echo "Environment setting DAQUIRI_KAFKA_BROKER is set to : $DAQUIRI_KAFKA_BROKER"
    echo -n "proceed (y/n)? "
    read YESNO
    if [[ $YESNO != "y" ]]; then
        echo "exiting ..."
        exit
    fi
fi

pushd $(dirname "${BASH_SOURCE[0]}")/../build

./bin/daquiri $@
