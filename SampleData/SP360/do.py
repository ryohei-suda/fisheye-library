#!/usr/bin/python

import sys
import subprocess

#subprocess.call("./Reconstruction9 params_sp360_5.xml 2700 70 103_015?.JPG", shell=True)
#subprocess.call("./Reconstruction5 params_sp360_5.xml 1800 65 103_015?.JPG", shell=True)

#subprocess.call("~/Desktop/openMVG_Build/software/SfM/openMVG_main_computeMatches -i images -e *.png -o matches", shell=True)

subprocess.call("~/Desktop/openMVG_Build/software/SfM/openMVG_main_IncrementalSfM -i images -m matches -o outReconstruct -p 1", shell=True)

#subprocess.call("cd outReconstruct/PMVS", shell=True)
#subprocess.call("~/Desktop/CMVS-PMVS-master/build/main/pmvs2 ./outReconstruct/PMVS/ ./outReconstruct/PMVS/pmvs_options.txt", shell=True)
subprocess.call("cd outReconstruct/PMVS; ~/Desktop/CMVS-PMVS-master/build/main/pmvs2 ./ pmvs_options.txt", shell=True)
