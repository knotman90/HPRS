#ifndef BINARYCONVERTE_H_
#define BINARYCONVERTE_H_

typedef char byte;

template <class T>
struct converter{
    union conv{
        T value;
        byte bytes[ sizeof( T ) ];
    } c ;

    T fromBytes( const  byte *bytes, bool reverse = false){
        int size = sizeof(T);
        if(reverse)
            for(int i=0;i<size;i++)
                c.bytes[size-1-i] = bytes[i];
        else
            for(int i=0;i<size;i++)
                c.bytes[i] = bytes[i];

        return c.value;
    }

     byte* toBytes(const T& value){
        c.value =value;
        return c.bytes;
    }

};

template<class T>
void printHex(const T& key, size_t size){

                 for (int i = 0; i < size; i++)
                 {
                     if (i > 0) printf(":");
                     printf("%02X", key[i]);
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



void print_bytes(const void *object, size_t size){
  size_t i;

  printf("[ ");
  for(i = 0; i < size; i++)
  {
    printf("%02x ", ((const unsigned char *) object)[i] & 0xff);
  }
  printf("]\n");
}








#endif /* BINARYCONVERTE_H_ */
