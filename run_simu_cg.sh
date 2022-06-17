#!/bin/bash  
echo "Shell script to compile cg and test on simu dataset"
# cp -a /home/nguyentx/Sundial/TOS/mtos/component-graph/InteractiveColorFiltering/data/sim/. //home/nguyentx/Sundial/TOS/mtos/component-graph/Simulation/data/sim

#compile CGO
sh ./demo_compile.sh

#Loop over data dir
for dir in data/sim2/*/
do
    echo ${dir}    # print everything after the final "/"
	echo "snr = ${dir}" >> cg_test_1.txt
	alphas=(0.01 0.001 0.0001 0.00001 0.000001)
	for alpha in "${alphas[@]}"
	do 
		echo "alpha = $alpha"
	    TP=0
	    FP=0
	    FN=0
		acc=0
		multi=0
		c=0
		unit=1
		base_dir=${dir/data/}
		base_dir=${base_dir/sim2/}
		base_dir=${base_dir///}
		filename_gt="${dir}segmaps_fake_test_6_${base_dir}.fits" 
		#Loop over file in each snr_dir
		for filename in ${dir}band_1*_ori.fits
		do
			echo $filename
			filename_b2=${filename/band_1/band_2}
			echo $filename_b2
			filename_b3=${filename/band_1/band_3}
			echo $filename_b3
			echo $filename_gt
			#run CGO
			output="$(./demo $filename $filename_b2 $filename_b3 $alpha)"
			filename_det="${filename}_det.fits"
			echo $filename_det
			mv "all_obj_.fits" $filename_det
			out="${filename}.ppm"
			mv "all_obj.ppm" $out
			#Run evaluation module
			eval=($(python3 eval/segmap_eval.py $filename $filename_gt $filename_det))
			tp=${eval[0]}
			fp=${eval[1]}
			fn=${eval[2]}
			TP=$((TP + tp))
			FP=$((FP + fp))
			FN=$((FN + fn))
			echo "TP=$TP, FP=$FP, FN=$FN \n"
			# read -p "Press [Enter] key to continue"
		done
		#Synthsized precision/recall		
		echo "${dir}; ${alpha}; $TP; $FP; $FN; " $((10000*TP / (TP + FP))) | sed 's/..$/.&/' >> cg_test_1.txt
		echo "${dir}; ${alpha}; $TP; $FP; $FN; " $((10000*TP / (TP + FN))) | sed 's/..$/.&/' >> cg_test_1.txt
		# echo print $acc/$c. | python  >> cg_test_1.txt
		# echo "$dir" | tr -d -c 0-9 >> cg_test_1.txt
		echo -e "\n" >> cg_test_1.txt
	done


done

#chmod +x run_simu_cg.sh