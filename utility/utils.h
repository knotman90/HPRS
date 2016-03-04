#ifndef UTILS_H
#define UTILS_H
#include<cstdlib>
template<typename T>
T d_rand(T lower, T upper) {
  T f = (T)rand() / RAND_MAX;

  return lower + f * (upper - lower);
}


template <class PRINTABLE>
void PRINTDEBUG(const PRINTABLE& p){
    #ifdef NDEBUG
    std::cout<<p<<std::endl;
    #endif
}
#endif //UTILS_H
