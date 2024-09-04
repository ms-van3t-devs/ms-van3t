#include <string>
#include <iostream>
#include <sstream>
#include <cmath>
#include <fstream>
#include <numeric>
#include "sample_quad_final.h"

namespace QuadKeys
{
	QuadKeyTS::QuadKeyTS() {
		// Set the default level of detail and corresponding needed lat-lon variation
		m_levelOfDetail = 16;
		m_latlon_variation = 0.001;
	}

	double
	QuadKeyTS::Clip(double n, double minValue, double maxValue) {
		return std::min(std::max(n, minValue), maxValue);
	}

	unsigned int
	QuadKeyTS::MapSize(int levelOfDetail) {
		return (unsigned int) 256 << levelOfDetail;
	}

	bool
	QuadKeyTS::finder(std::string vi, std::string vk, int l, std::array<int,4> &chars) {
		std::array<int,4> myints = {1,2,3,4};

		if(vi[l] == '0') {
			chars[0] = 1;
		}
		else if(vi[l] == '1') {
			chars[1] = 2;
		}
		else if(vi[l] == '2') {
			chars[2] = 3;
		}
		else if(vi[l] == '3') {
			chars[3] = 4;
		}

		if(vk[l] == '0') {
			chars[0] = 1;
		}
		else if(vk[l] == '1') {
			chars[1] = 2;
		}
		else if(vk[l] == '2') {
			chars[2] = 3;
		}
		else if(vk[l] == '3') {
			chars[3] = 4;
		}

		/*for(int i=0;i<4;i++){
			std::cout<<chars[i]<<" ";
		}*/

		if(chars == myints){
			chars = {};
			return 1;
		}
		else {
			return 0;
		}
	}

	// Here we distinguish the cases and set the private attributes on the base of desired levelOfDet
	void
	QuadKeyTS::setLevelOfDetail(int levelOfDetail) {
		m_levelOfDetail = levelOfDetail;

		if(levelOfDetail < 14){
			m_levelOfDetail = 14;
		}
		if(levelOfDetail > 18){
			m_levelOfDetail = 18;
		}

		switch(m_levelOfDetail){

		case 18:
			m_latlon_variation = 0.0001; //level 18,17 needs a scan of 0.0001 to have all quadkeys correctly unified
			break;
		case 17:
			m_latlon_variation = 0.0001;
			break;
		case 16:
			m_latlon_variation = 0.001;
			break;
		case 15:
			m_latlon_variation = 0.001;
			break;
		case 14:
			m_latlon_variation = 0.001;
			break;
		default: // We should never reach this point
			m_latlon_variation = 10000;
			break;
		}
	}

	std::string
	QuadKeyTS::LatLonToQuadKey(double latitude, double longitude) {
		std::stringstream quadKey;
		// int levelOfDetail = ...; // The value of the desired zoom is obtained directly from the private attribute

		double x = (longitude + 180) / 360;
		double sinLatitude = sin(latitude * M_PI / 180);
		double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);

		uint mapSize = MapSize(m_levelOfDetail);
		int pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
		int pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
		int tileX =  pixelX / 256;
		int tileY =  pixelY / 256;

		for (int i = m_levelOfDetail; i > 0; i--) {
			char digit = '0';
			int mask = 1 << (i - 1);
			if ((tileX & mask) != 0)
			{
				digit++;
			}
			if ((tileY & mask) != 0)
			{
				digit++;
				digit++;
			}
			quadKey << digit;
		}

		std::cout<<"sto inviando: "<<quadKey.str()<<std::endl;
		return quadKey.str();
	}

	std::vector<std::string>
	QuadKeyTS::LatLonToQuadKeyRange(double min_latitude, double max_latitude, double min_longitude, double max_longitude) {
		// int levelOfDetail = ...; // The value is now obtained directly from the private attribute
		//std::cout<<"\nCurrent variation: "<<m_latlon_variation<<"\nCurrent levelOfDetail: "<<m_levelOfDetail<<std::endl;
		std::string quadKey;
		std::vector<std::string> v = {};
		std::cout<<"Receiver initialization"<<std::endl;

		//starting scan the lan_lon using the private attribute m_latlon_variation
		for(double j = min_latitude; j <= max_latitude; j+=m_latlon_variation){
			for(double k = min_longitude; k <= max_longitude; k+=m_latlon_variation){

					quadKey = "";
					double x = (k + 180) / 360;
					double sinLatitude = sin(j * M_PI / 180);
					double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);
					uint mapSize = MapSize(m_levelOfDetail);
					int pixelX = (int) Clip(x * mapSize + 0.5, 0, mapSize - 1);
					int pixelY = (int) Clip(y * mapSize + 0.5, 0, mapSize - 1);
					int tileX =  pixelX / 256;
					int tileY =  pixelY / 256;

					for (int i = m_levelOfDetail; i > 0; i--) {
					char digit = '0';
					int mask = 1 << (i - 1);
					if ((tileX & mask) != 0) {
						digit++;
					}
					if ((tileY & mask) != 0) {
						digit++;
						digit++;
					}
					quadKey += digit;
				}
				v.push_back(quadKey);
			}
		}

		//clear the vector to construct a desired one for testing

		/*v.clear();

		v.push_back("012311211231");
		v.push_back("012311211232");
		v.push_back("012311211233");
		v.push_back("012311211222");
		v.push_back("012311211211");
		v.push_back("012311211202");

		v.push_back("21000");
		v.push_back("21001");
		v.push_back("21002");
		v.push_back("21003");
		v.push_back("21010");
		v.push_back("21011");
		v.push_back("21012");
		v.push_back("21013");
		v.push_back("21020");
		v.push_back("21021");
		v.push_back("21022");
		v.push_back("21023");
		v.push_back("21030");
		v.push_back("21031");
		v.push_back("21032");
		v.push_back("22211");
		v.push_back("21033");*/

		 std::ofstream fout("firstvec.txt");
		 for(size_t i = 0; i<v.size(); i++){
		 	 fout << v.at(i) << "\n";
		 }
		 fout.close();
		 std::cout<<"Area defined in quadkeys"<<std::endl;

		return v;
	}

	void QuadKeyTS::unifyQuadkeys(std::vector<std::string> &quadKeys) {

		std::cout<<"Starting unifying"<<std::endl;
		size_t k;
		bool loop = true;
		std::array<int,4> chars;
		bool complete;
		std::vector<std::string> vf = {};
		std::vector<std::string> vk = {};

		while(loop){
			//std::cout<<"loop\n";
			size_t i = 0;
			vf.clear();
			loop = false;
			//int l = v.at(i).size() - 2;
			//found = 0;

			while(i <= quadKeys.size()-1) {
				k = 0;
				//found = 0;
				int l = quadKeys.at(i).size() - 2;
				complete = false;
				chars = {};
				//std::cout<<"while() "<<i<<std::endl;


				while(k <= (quadKeys.size() - 1)) {
					std::array<int,4> *charsp = &chars;

					if(i != k && quadKeys.at(i).compare(0,l+1,quadKeys.at(k),0,l+1) == 0){

						if(quadKeys.at(i).size() != quadKeys.at(k).size()){
							k++;
							continue;
						}

						vk.push_back(quadKeys.at(k).substr(0,l+1));

						if(finder(quadKeys.at(i), quadKeys.at(k), l+1, *charsp)){
							//std::cout<<"complete"<<std::endl;
							complete = true;
							chars = {};
						}
					}

					if(k == quadKeys.size() - 1 && !complete) {
						//std::cout<<"printing on final\n";
						vf.push_back(quadKeys.at(i).substr(0,l+2));
						vk.clear();
						break;
					}

					if(k == quadKeys.size() - 1 && complete) {
						size_t j = 0;
						while(j<=vk.size()-1){
							if(quadKeys.at(i).compare(0,l+1,vk.at(j),0,l+1) == 0){
								if(finder(quadKeys.at(i), vk.at(j), l+1, *charsp )){
									l--;
									chars = {};
								}
							j++;
							}
						}
						vf.push_back(quadKeys.at(i).substr(0,l+1));
						loop = true;
						vk.clear();
						break;
					}
					k++;
				}
				i++;
			}

			quadKeys = vf;
			std::cout<<"in loop for unifying == "<<loop<<std::endl;
		}

		//erasing egual elements
		for(size_t i = 0; i < quadKeys.size(); i++){
			for(size_t k = 0; k < quadKeys.size(); k++){
				if(i!=k && quadKeys.at(i) == quadKeys.at(k)){
					auto it = quadKeys.begin();
					quadKeys.erase(it + k );
					i--;
					break;
				}
			}
		}

		std::ofstream fout("lastvec.txt");
        for(size_t i = 0; i < quadKeys.size(); i++){
            fout << quadKeys.at(i) << "\n";
        }
        fout.close();

        //Start controlling the dimension of the vector

		/*int total = 0;
		int *tot = &total;
		for(size_t i = 0; i<vf.size(); i++){
            total = total + sizeof(char)*vf.at(i).size();
		}*/

		//std::cout<<"Vector erased, checking dimension: "<< total + 21*(vf.size()-1) + 17 + 180 <<std::endl; //21 chars are added for every vector's string, while for the last one 17 chars are added insted. 180 is the offset btw the computed size and the one given by the error.

        std::cout<<"Vector erased, checking dimension: "<<std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180<<std::endl;
	}

    void QuadKeyTS::checkdim(std::vector<std::string> &quadKeys){
        std::vector<std::string> vf = quadKeys;
		const int max_length = 2200; //131072; //222; maximum length in byte to transfer infos to the broker
		int l = m_levelOfDetail; //5;
		//int vec_len = vf.size();

		while(std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 > max_length ){

            std::cout<<"Dimensions exceeded, resizing:"<<std::endl;
            int flag = 0;


            for (size_t j = 0; j < vf.size(); j++){
                //std::cout<<vf.at(j).size() <<std::endl;
               // std::cout<<j<<std::endl;


                if(vf.at(j).size() ==  (size_t)l){
                    //std::cout<<"found"<<vf.at(j).size() <<std::endl;
                    flag++;
                    vf.push_back(vf.at(j).substr(0,l-1));
                    auto it = vf.begin();
                    vf.erase(it + j);
                    break;

                }
                if(j == vf.size() - 1 && flag == 0){
                    l--;
                }
            }


            for(size_t i = 0; i < vf.size(); i++){
			    for(size_t k = 0; k < vf.size(); k++){
				    if(i!=k && vf.at(i) == vf.at(k)){
					    auto it = vf.begin();
					    vf.erase(it + k );
					    i--;
					    break;
				    }
			    }
		    }
		    QuadKeyTS::unifyQuadkeys(vf);
		    /* *tot = 0;

		    for(size_t i = 0; i < vf.size(); i++){

                *tot = *tot + sizeof(char)*vf.at(i).size();
		    }*/

		    std::cout<<"New dimension: "<< std::accumulate(vf.begin(), vf.end(), -4, [](int sum, const std::string& elem) {return sum + elem.size() + 21;}) + 180 <<std::endl;

		    if(l<0){break;}
		}

		quadKeys = vf;


        std::cout<<"\nFinished\n";
        /*//Generate a vector to visualize the quadkeys' strings
        std::fstream fout2("lastvec.txt");
        for(int i = 0; i < quadKeys.size(); i++){
            fout2 << quadKeys.at(i) << "\n";
        }
        fout2.close();*/


	}
}
