#!/bin/bash

for e in 1.0 3.0 5.0 10.0 15.0 25.0 50.0 100.0;
 do for a in 10.0  20.0 40.0 85.0;
    do
        sed -e "s@XXXINPUTXXX@GEN.singleMuons_${e}GeV_${a}deg.slcio@g" process_singleMuons-template.xml | sed -e "s@XXXOUTXXX@SIM.singleMuons_${e}GeV_${a}deg@g" >> process_singleMuons_${e}GeV-${a}deg.xml
    done;
 done;
 
