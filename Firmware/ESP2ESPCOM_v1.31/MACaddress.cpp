/******************************************************************************
                              Online C++ Compiler.
               Code, Compile, Run and Debug C++ program online.
Write your code in this editor and press "Run" button to compile and execute it.
*******************************************************************************/


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Arduino.h"
#include "MACaddress.h"

using namespace std;

bool DEBUG = true;


/*
Array2D<char, 6, 2> fnc(const Array2D<char, 6, 2> &a) {
  Array2D<char, 6, 2> a = 
  {{
    {"4","C"},
    {"1","1"},
    {"A","E"},
    {"E","A"},
    {"E","E"},
    {"B","4"},
  }};
  return a;
}*/


//Getting the MAC address characters from the input
char* MACextraction(String input, char *result[])
{
    if (DEBUG)
    {
      Serial.println("2. Enetered in MACextraction function"); 
    }
    //int sizein = strlen(input);
    int sizein=input.length();
    if (DEBUG)
    {
       Serial.print("2. Size of input is: "); 
       Serial.println(sizein);
    }
    int j=0;
    for(int i=0; i<sizein;++i)
    {   /*
        if (DEBUG)
        {
          Serial.println("2.1 Enetered in MACextraction function foor loop"); 
        }
        */
        char comp = 'x';
        if(input[i]==comp)
        {
            if (DEBUG)
            {
              Serial.println("2.2 Enetered in MACextraction function foor loop comparison"); 
            }
            result[j][0]=input[i+1];
            //Serial.println(result[j][0]);
            result[j][1]=input[i+2];
            //Serial.println(result[j][1]);
            result[j][2]='\0';
            ++j;
        }
    }
    if (DEBUG)
    {
      Serial.println("2.3 MCextraction function foor endend"); 
    }     
    return *result;
}


int hex2decimal(char hex[])
{
      if (DEBUG)
    {
      Serial.println("4. Enetered in hex2decimal function"); 
    }
    long long decimal, place;
    int i = 0, val, len;
    decimal = 0;
    place = 1;

    /* Input hexadecimal number from user */
    //printf("Enter any hexadecimal number: ");
    //gets(hex);

    /* Find the length of total number of hex digit */
    len = strlen(hex);
    len--;

    /*
     * Iterate over each hex digit
     */
    for(i=0; hex[i]!='\0'; i++)
    {
 
        /* Find the decimal representation of hex[i] */
        if(hex[i]>='0' && hex[i]<='9')
        {
            val = hex[i] - 48;
        }
        else if(hex[i]>='a' && hex[i]<='f')
        {
            val = hex[i] - 97 + 10;
        }
        else if(hex[i]>='A' && hex[i]<='F')
        {
            val = hex[i] - 65 + 10;
        }

        decimal += val * pow(16, len);
        len--;
    }
    //cout<<"Hexadecimal number = is: "<< hex<<" \n";
    //cout<<"Decimal number is: "<< decimal<<" \n";
    //printf("Hexadecimal number = %s\n", hex);
    //printf("Decimal number = %lld", decimal);
    return decimal;
}


char MACaddress(String input,char result[])
{
    if (DEBUG)
    {
      Serial.println("1. Enetered in MACaddress function"); 
      Serial.print("Input value is: ");
      Serial.println(input);
    }
    //char arraybuf[6][3];
    //Creating a new array to restore the conversion results.
    char** arraybuf = new char*[6];
    for (int i=0; i<6;++i)
    {
        arraybuf[i]=new char[3];
    }
    /*
    char input[]="[\"0x4C\",\"0x11\",\"0xAE\",\"0xEA\",\"0xEE\",\"0xB4\"]";
    
    char* output[]=  {{
    {"4C"},
    {"11"},
    {"AE"},
    {"EA"},
    {"EE"},
    {"B4"},
  }};*/
  
    //int size = sizeof(input);
    MACextraction(input,arraybuf);
    for (int i=0;i<6;++i)
    {
        result[i]=hex2decimal(arraybuf[i]);
    }
    
    return *result;
}
