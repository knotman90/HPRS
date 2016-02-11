/*
 * Particle.h
 *
 *  Created on: Jan 7, 2016
 *      Author: Davide Spataro
 */

#ifndef PARTICLE_H_
#define PARTICLE_H_
#include<iostream>
#include<cmath>
// Include GLM
#include <glm/glm.hpp>

template <typename T = double>
class Particle {
public:

    uint id,group,type;
    T volume;
    T mass;
    //position
    glm::vec3 p;
    //velocity
    glm::vec3 v;



    Particle(){};
    virtual ~Particle(){
       // std::cout<<"Particle "<<id<<" destructor called"<<std::endl;
    }

    template<class SEPARATOR>
    void print(std::ostream& os, const SEPARATOR& sep){

        os
                <<id<<sep
               <<group<<sep
              <<type<<sep
             <<volume<<sep
            <<mass<<sep
           <<p.x<<sep
          <<p.y<<sep
         <<p.z<<sep
        <<v.x<<sep
        <<v.y<<sep
        <<v.z<<sep;
    }
};



#endif /* PARTICLE_H_ */
