#!/bin/bash  
echo "Shell script to compile cg and test on simu dataset"

#compile CGO
sh ./demo_compile.sh

#Loop over data dir
for dir in data/sim2/*/
do
    echo ${dir}    # print everything after the final "/"
	# echo "snr = ${dir}" >> boundary_ct.txt
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
			#run CT
			output="$(./demo $filename $filename $filename $alpha $filename_gt)"
			output_2="$(./demo $filename_b2 $filename_b2 $filename_b2 $alpha $filename_gt)"
			output_3="$(./demo $filename_b3 $filename_b3 $filename_b3 $alpha $filename_gt)"
			if [[ output_2 -gt output ]]; then
				output=$((output_2))
			fi
			if [[ output_3 -gt output ]]; then
				output=$((output_3))
			fi

			avg_score=$((avg_score + output))
			count=$((count + 1))
			# echo "$output $output_2 $output_3"
			echo "$output $avg_score $count"
			read -p "Press [Enter] key to continue"
		done
		#Synthsized precision/recall		
		echo "${dir}; ${alpha}; $avg_score; $((avg_score/count))" >> boundary_ct.txt
		# echo "${dir}; ${alpha}; $TP; $FP; $FN; " $((10000*TP / (TP + FN))) | sed 's/..$/.&/' >> boundary_ct.txt
		# echo -e "\n" >> boundary_ct.txt
	done


done

#chmod +x run_ct_boundary.sh