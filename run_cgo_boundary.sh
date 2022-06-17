#!/bin/bash  
echo "Shell script to compile cg and test on simu dataset"

#compile CGO
sh ./demo_compile.sh

#Loop over data dir
for dir in data/sim2/*/
do
    echo ${dir}    # print everything after the final "/"
	# echo "snr = ${dir}" >> boundary_cgo.txt
	alphas=(0.000001)
	for alpha in "${alphas[@]}"
	do 
		echo "alpha = $alpha"
		avg_score=0
		count=0
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
			output="$(./demo $filename $filename_b2 $filename_b3 $alpha $filename_gt)"
			avg_score=$((avg_score + output))
			count=$((count + 1))
			echo "$output $avg_score $count"
			read -p "Press [Enter] key to continue"
		done
		#Synthsized precision/recall		
		echo "${dir}; ${alpha}; $avg_score; $((avg_score/count))" >> boundary_cgo.txt
		# echo "${dir}; ${alpha}; $TP; $FP; $FN; " $((10000*TP / (TP + FN))) | sed 's/..$/.&/' >> boundary_cgo.txt
		# echo -e "\n" >> boundary_cgo.txt
	done


done

#chmod +x run_simu_cg.sh