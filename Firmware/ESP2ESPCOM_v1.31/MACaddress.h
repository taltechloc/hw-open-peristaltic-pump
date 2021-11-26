/*
  Morse.h - Library for flashing Morse code.
  Created by Martin Grosberg, September 5, 2020.
  Released into the public domain.
*/

//Getting the MAC from recieved json string

#ifndef MACaddress_h
#define MACaddress_h

#include "Arduino.h"
/*
template <class T, size_t N>
struct Array {
    T data[N];
   
    T &operator[](size_t index) { return data[index]; }
    const T &operator[](size_t index) const { return data[index]; } 
   
    T *begin() { return &data[0]; }
    const T *begin() const { return &data[0]; }
    T *end() { return &data[N]; }
    const T *end() const { return &data[N]; }
};

template <class T, size_t R, size_t C>

using Array2D = Array<Array<T, C>, R>;
const int rows= 6;
const int columns =3;*/
char* MACextraction(String input, char *result[]);
char MACaddress(String input,char result[]);
int hex2decimal(char hex[]);

#endif
