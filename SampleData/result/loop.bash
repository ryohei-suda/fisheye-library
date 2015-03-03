#!/bin/bash

#XML="lines_simulated.xml"
DIR="./result/"
XML=("Aresult.xml" "Bresult.xml" "Cresult.xml" "Dresult.xml" "Eresult.xml" "Fresult.xml" "Gresult.xml" "Hresult.xml")
OUT="params_d_"

#X=("944" "950" "952" "954" "956" "958" "964")
#Y=("587" "593" "595" "597" "599" "601" "607")
#F=("390" "396" "398" "400" "402" "406" "410")
#X=("620" "630" "640" "650" "660")
#Y=("460" "470" "480" "490" "500")
#F=("460" "470" "480" "490" "500")
#X=("802" "805" "808" "811")
#Y=("594" "597" "600" "603")
#F=("497" "400" "403" "406")

for x in ${XML[@]}; do
    echo "./Calibration" "${DIR}""${x}" "5" "${OUT}${x}"
    ./Calibration "${DIR}""${x}" 5 "${OUT}${x}"
done
