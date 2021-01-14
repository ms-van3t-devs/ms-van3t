#!/bin/bash

for filename in ./*; do
	echo "Processing file... ${filename}"
	sed -i -E 's/(#include )<(.+)>(.*)/\1"\2"\3/g' ${filename}
	sed -i -E 's/#include <malloc.h>(.*)/#include "malloc.h"\1/g' ${filename}
	sed -i -E 's/#include <stdint.h>(.*)/#include "stdint.h"\1/g' ${filename}
	sed -i -E 's/#include <windows.h>(.*)/#include "windows.h"\1/g' ${filename}
	sed -i -E 's/#include <float.h>(.*)/#include "float.h"\1/g' ${filename}
	sed -i -E 's/#include <stdio.h>(.*)/#include "stdio.h"\1/g' ${filename}
	sed -i -E 's/#include <stdlib.h>(.*)/#include "stdlib.h"\1/g' ${filename}
	sed -i -E 's/#include <errno.h>(.*)/#include "errno.h"\1/g' ${filename}
done