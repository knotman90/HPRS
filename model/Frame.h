/*
 * Frame.h
 *
 *  Created on: Jan 7, 2016
 *      Author: knotman
 */

#ifndef FRAME_H_
#define FRAME_H_

#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include <string>

#include <Particle.h>

template<class T = double>
class Frame{
public:
	uint ID;
	uint nParticles;
	std::vector<Particle<T>> particles;

	Frame<T>(){
		ID=0;
		nParticles=0;
	}

	Frame<T>(uint ID,uint nparticles){
		this->ID=ID;
		this->nParticles = nparticles;
	}

    void print(std::ostream& os){
		for(auto p: particles){
            p.print(os, " ");
			os<<std::endl;
		}
	}

    virtual ~Frame<T>(){
        std::cout<<"Frame destructor called"<<std::endl;
    };
};

#endif /* FRAME_H_ */
