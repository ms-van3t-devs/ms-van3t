#!/bin/bash

for filename in ./model/* ./helper/* ./test/* ; do
	if [ -f ${filename} ]; then
		filename_no_path=$(basename ${filename})
		filename_path_only=$(dirname ${filename})
		new_filename=$(echo "${filename_path_only}/cv2x_${filename_no_path}")
		new_filename_no_path=$(basename ${new_filename})

		#sed -i -E "s/(.*)namespace ns3(.*)/\1namespace cv2xns3\2/g" ${filename}

		mv -- ${filename} ${new_filename}

		echo "Replacing all occurrences of ${filename_no_path} with ${new_filename_no_path}..."

		for filename_subst in ./model/* ./helper/* ./test/*; do
			if [ -f ${filename_subst} ]; then
				if [ ${filename_no_path} != "c-v2x-patch.sh" ]; then
					#echo "(Substitution phase) Processing file... ${filename_subst}"

					sed -i -E "/cv2x_/! s|(.*)#include <ns3/${filename_no_path}(.*)|\1#include <ns3/${new_filename_no_path}\2|g" ${filename_subst}
					sed -i -E "/cv2x_/! s|(.*)#include \"ns3/${filename_no_path}(.*)|\1#include \"ns3/${new_filename_no_path}\2|g" ${filename_subst}
					sed -i -E "/cv2x_/! s|(.*)#include <${filename_no_path}(.*)|\1#include <${new_filename_no_path}\2|g" ${filename_subst}
					sed -i -E "/cv2x_/! s|(.*)#include \"${filename_no_path}(.*)|\1#include \"${new_filename_no_path}\2|g" ${filename_subst}
				fi
			fi
		done

		# Look for all the class definitions inside the file
		classes=($(grep "^[[:blank:]]*class" ${new_filename} | grep -v ";" | cut -d" " -f2 | cut -d":" -f1))

		for i in "${!classes[@]}"; do
			if [[ "${classes[$i]}" =~ "cv2x_" ]]; then
				echo "Warning: found a class definition already containing the cv2x prefix!"
				unset classes[$i]
			fi
		done

		echo "Changing names for classes:"
		echo "${classes[@]}"

		for cl in ${classes[@]}; do
			for filename_subst in ./model/* ./helper/* ./test/*; do
				if [ -f ${filename_subst} ]; then
					if [ ${filename_no_path} != "c-v2x-patch.sh" ]; then
						#echo "(Substitution phase) Processing file... ${filename_subst}"

						# echo "cv2x_EpcEnbS1SapProvider::~EpcEnbS1SapProvider ()" | sed -E "s#([^a-zA-Z]+|^)$EpcEnbS1SapProvider(.*)#\1cv2x_EpcEnbS1SapProvider\2#g"
						# sed -i -E "s#([^a-zA-Z]+|^)${cl}(.*)#\1cv2x_${cl}\2#g" ${filename_subst}

						sed -i -E "s#([^a-zA-Z_]+|^)${cl}(.*)#\1cv2x_${cl}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${cl}(.*)#\1cv2x_${cl}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${cl}(.*)#\1cv2x_${cl}\2#g" ${filename_subst}
						sed -i -E "s#([^a-zA-Z_]+|^)${cl}(.*)#\1cv2x_${cl}\2#g" ${filename_subst}
					fi
				fi
			done
		done

		if [[ $new_filename =~ ".h" ]]; then
			header_check_macro=$(grep "^#ifndef" ${new_filename} | cut -d" " -f2)
			sed -i -E "s/(.*)${header_check_macro}(.*)/\1CV2X_${header_check_macro}\2/g" ${new_filename}
		fi
	fi
done

sed -i -E "s|(.*)model/(.*)|\1model/cv2x_\2|g" wscript
sed -i -E "s|(.*)helper/(.*)|\1helper/cv2x_\2|g" wscript
sed -i -E "s|(.*)test/(.*)|\1test/cv2x_\2|g" wscript
