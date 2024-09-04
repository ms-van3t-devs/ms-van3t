#ifndef QUADKEYTILESYSTEM_H
#define QUADKEYTILESYSTEM_H

#include <math.h>
#include <string.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <array>

namespace QuadKeys
{
    class QuadKeyTS
    {
        private:
        	double Clip(double n, double minValue, double maxValue);
			int m_levelOfDetail; //used to set the levelOfDetail indipendently
       		double m_latlon_variation;
       		static bool finder(std::string vi, std::string vk, int l, std::array<int,4> &chars);

        public:
        	QuadKeyTS();
        	unsigned int MapSize(int levelOfDetail);
        	void setLevelOfDetail(int levelOfDetail = 16);
        	std::string LatLonToQuadKey(double latitude, double longitude);
        	std::vector<std::string> LatLonToQuadKeyRange(double min_latitude, double max_latitude, double min_longitude, double max_longitude);
        	// This method should be called on the output of LatLonToQuadKeyRange(), in order to consolidate together quadkeys, when possible,
        	// and reduce the size of the filter
        	// The input vector is passed by reference and it is thus modified by unifyQuadkeys()
    		void unifyQuadkeys(std::vector<std::string> &quadKeys);
    		void checkdim(std::vector<std::string> &quadKeys);
    };
}

#endif // QUADKEYTILESYSTEM_H
