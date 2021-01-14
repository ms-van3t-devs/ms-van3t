#!/bin/bash

for filename in ./model/* ./helper/* ./test/* ; do
	if [ -f ${filename} ]; then
		filename_no_path=$(basename ${filename})
		filename_path_only=$(dirname ${filename})

		echo "Analyzing and processing file: ${filename}"

		# Look for all the struct definitions inside the file
		structures=($(grep "^[[:blank:]]*struct" ${filename} | grep -v ";" | cut -d" " -f2 | cut -d":" -f1 | cut -d"{" -f1))

		for i in "${!structures[@]}"; do
			if [[ "${structures[$i]}" =~ "cv2x_" ]]; then
				echo "Warning: found a struct definition already containing the cv2x prefix!"
				unset structures[$i]
			fi
		done

		echo "Changing names for struct:"
		echo "${structures[@]}"

		for strct in ${structures[@]}; do
			for filename_subst in ./model/* ./helper/* ./test/*; do
				if [ -f ${filename_subst} ]; then
					if [ ${filename_no_path} != "c-v2x-patch.sh" ]; then
						#echo "(Substitution phase) Processing file... ${filename_subst}"

						# echo "cv2x_EpcEnbS1SapProvider::~EpcEnbS1SapProvider ()" | sed -E "s#([^a-zA-Z]+|^)$EpcEnbS1SapProvider(.*)#\1cv2x_EpcEnbS1SapProvider\2#g"
						# sed -i -E "s#([^a-zA-Z]+|^)${strct}(.*)#\1cv2x_${strct}\2#g" ${filename_subst}

						sed -i -E "s#([^a-zA-Z_]+|^)${strct}(.*)#\1cv2x_${strct}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${strct}(.*)#\1cv2x_${strct}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${strct}(.*)#\1cv2x_${strct}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${strct}(.*)#\1cv2x_${strct}\2#g" ${filename_subst}
					fi
				fi
			done
		done
	fi
done