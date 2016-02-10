#ifndef BINARYCONVERTE_H_
#define BINARYCONVERTE_H_
#include<iostream>

typedef unsigned char byte;

template <class T>
struct converter{

 static const size_t size = sizeof(T);

    union conv{
        T value;
        byte bytes[ sizeof( T ) ];
    } c ;

    T fromBytes( const  byte *bytes, bool endianness = false){

        if(endianness)
            #pragma unroll
            for(int i=0;i<size;i++)
                c.bytes[size-1-i] = bytes[i];
        else
          #pragma unroll
            for(int i=0;i<size;i++)
                c.bytes[i] = bytes[i];

        return c.value;
    }

     byte* toBytes(const T& value,bool endianness = false){
        c.value =value;
        if(endianness)
            reverse();
        return c.bytes;
    }

    void reverse(){
        #pragma unroll
        for(int i=0;i<size/2;i++){
            byte tmp = c.bytes[i];
            c.bytes[i]=c.bytes[size-1-i];
            c.bytes[size-1-i] = tmp;
        }

    }

};

template<class T>
void printHex(const T& key, size_t size){
  std::cout.setf(std::ios::hex, std::ios::basefield);
    for (int i = 0; i < size; i++){
         if (i > 0) printf(":");
            printf("%02X", (unsigned char)key[i]);
}
     printf("\n");

}

template<class T>
const byte* convert_union(T val){
    converter<T> c = {.value=val };
    return c.bytes;
}

template<class FROM, class TO = byte*>
TO convert_cast(FROM val){
    return reinterpret_cast<TO>(val);
}


template<class T>
void IntegerToBytes(long long val, T& b, uint offset) {
    b+= (byte )((val >> 56) & 0xff);
    b+= (byte )((val >> 48) & 0xff);
    b+= (byte )((val >> 40) & 0xff);
    b+= (byte )((val >> 32) & 0xff);
    b+= (byte )((val >> 24) & 0xff);
    b+= (byte )((val >> 16) & 0xff);
    b+= (byte )((val >> 8) & 0xff);
    b+= (byte )(val & 0xff);
}










#endif /* BINARYCONVERTE_H_ */
