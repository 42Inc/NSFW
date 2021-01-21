#!/bin/bash

PWD=`pwd`

. ${PWD}/starter.dat
if [[ $? != 0 ]]; then exit 1; fi

for (( i=0; i < ${#HOSTS[@]}; i+=1)); do
  echo start process with rank $i and comm ${#HOSTS[@]}, ${HOSTS[$i]}
  ssh ${HOSTS[$i]} ${PWD}/bin/rmpi $i ${#HOSTS[@]} ${HOSTS[@]} $* > ${PWD}/proc_$i.log 2>&1 &
done