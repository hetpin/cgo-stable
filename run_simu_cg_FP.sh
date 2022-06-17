echo "Shell script to compile mto and all run fake_test_6_ FP"
cp -a /home/nguyentx/Sundial/TOS/mtos/component-graph/InteractiveColorFiltering/data/sim_empty/. /home/nguyentx/Sundial/TOS/mtos/component-graph/Simulation/data/sim_empty

sh ./demo_compile.sh

for dir in data/sim_empty/*/
do
    echo ${dir}    # print everything after the final "/"
	acc=0
	multi=0
	c=0
	unit=1
	for filename in ${dir}band_1*.fits #fake_6_2/*.fits
	# for filename in ${dir}*.fits #fake_6_2/*.fits
	do
		echo $filename
		filename_b2=${filename/band_1/band_2}
		echo $filename_b2
		filename_b3=${filename/band_1/band_3}
		echo $filename_b3
		output="$(./demo $filename $filename_b2 $filename_b3)"
		# output="$(./demo $filename $filename $filename)"
		echo "$output object found"
		if [ "$output" -gt 0 ]
		then
            echo "Objects found"
			multi=$(( multi + $output))
			cpname="/home/nguyentx/Desktop/${c}.ppm"
			cp "all_obj.ppm" $cpname
        else
			acc=$(( acc + unit ))
        fi

		c=$((c+unit))
		# echo "$acc"
		# imview out.png
		out="${filename}.ppm"
		# echo $out
		mv "all_obj.ppm" $out
	done
	echo "${dir}; $multi; $acc; $c; " $(( 10000 * multi / c )) | sed 's/..$/.&/' >> cg_test_1_FP.txt
	# echo print $acc/$c. | python  >> cg_test_1_FP.txt
	echo "$dir" | tr -d -c 0-9 >> cg_test_1_FP.txt
	echo -e "\n" >> cg_test_1_FP.txt
done
#chmod +x run_simu_cg_FP.sh