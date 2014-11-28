#!/bin/bash

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_100.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_50.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_25.0GeV-${i}.0deg.xml ../physics.xml &
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_15.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_10.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_5.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_3.0GeV-${i}.0deg.xml ../physics.xml&
done

wait

for i in 10 20 40 85; 
    do dd_sim ../../ILD/compact/ILD_o1_v05.xml process_singleMuons_1.0GeV-${i}.0deg.xml ../physics.xml&
done
