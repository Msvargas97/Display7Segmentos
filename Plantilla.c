#include "EMm47J53_StackConfig.h"
#INCLUDE <math.h>
#INCLUDE <stdlib.h>

//############################
//Declaracion de caracteristicas del circuito de displays 7 segmentos
#define NUM_DISPLAYS 4
#define CATODO_COMUN 0x01
#define ANODO_COMUN 0x00
#define NPN 0x01
#define PNP 0x00
#define output_Displays(x) output_d(x)
#define TIME_MUX 1000 // tiempo de multiplexacion en microsegundos

     //Tipos de displays   //tipo de transistores
int1 D7Seg= CATODO_COMUN , Q = PNP;

//############################
/********************************************************/
/*------- Espacio para declaracion de constantes  ------*/
/********************************************************/
const unsigned int16 transistorPins[NUM_DISPLAYS]={PIN_A3,PIN_A2,PIN_A1,PIN_A0};
const unsigned char Tabla7seg[10]=
{ // HGFEDCBA <-- Segmento
   0b00111111, // 0
   0b00000110, // 1
   0b01011011, // 2
   0b01001111, // 3
   0b01100110, // 4
   0b01101101, // 5
   0b01111101, // 6
   0b00000111, // 7
   0b01111111, // 8
   0b01100111  // 9
};

float val=-2.100;
signed int16 num=6253;
/********************************************************/
/*--- Espacio para declaracion de variables globales  --*/
/********************************************************/
/********************************************************/
/********************************************************/
/*-------------- Espacio para funciones  ---------------*/
/********************************************************/
void displayMux(float numero,unsigned int16 time,char precision=0);

/******************************************************************************/
/******************************************************************************/
/*--------------------- Espacio de codigo principal --------------------------*/
/******************************************************************************/ 
#zero_ram

void main()
{
   mcu_init (); // Inicializa microcontrolador
   set_tris_d (0) ;
   set_tris_a (0) ;
   for (;;)
   {
      displayMux (-0.014,5,2) ;
     val= val + 0.010;
   }//end void loop
}//end main

void displayMux(float numero,unsigned int16 time,char precision=0)
{
  //Crea variables estaticas dentro de la función para evitar conflictos con el código del menu principal
   static unsigned char digito[NUM_DISPLAYS];
   static unsigned int size, max = 0;
   static signed int32 temp;
   static int16 i = 0, j = 0, decimal, dec = 0;
   static int1 signo = 0, limit = 0;
   static const unsigned int32 powersOf10[]  =
   {
      1, // 10 ^ 0
      10,
      100,
      1000,
      10000,
      100000,
      1000000,
      10000000,
      100000000,
      1000000000
   }; // 10 ^ 9
   memset(digito, 0, NUM_DISPLAYS);//Borra los digitos anteriores
  temp = numero; //Separa la parte entera
   if (numero < 0)
   {
      temp = abs (numero); 
      signo = 1; //Verifica si el numero es negativo
      }else{
      signo = 0;
   }
   max = NUM_DISPLAYS - precision - signo; //Calcula el numero de displays maximos para terminar los rangos permitidos
   limit = (signo) ? numero > (powersOf10[max] * - 1): numero < powersOf10[max]; // determina si el limite del numero segun los digitos ingresados
   if (precision > 0)       //Separa la parte decimal siempre y se escogen la cantidad de decimales
   {
      decimal = abs (numero * powersOf10[precision]) ;
      decimal %= powersOf10[precision];
      dec=decimal;
   }

   if (limit)// condicion para calcular los digitos si el numero ingresado es menor que el posible a mostrar
   {
      size = 0;
       //Obtiene los digitos y cuenta cuantos digitos son
      do{
         if (precision > 0 && size < precision) 
         {
           digito[size] = abs(decimal % 10);
            decimal /= 10;
            }else{
            digito[size] = temp % 10;
            temp /= 10;
         }
         size++;
      }while (temp > 0 || decimal > 0);
   temp=abs(numero);
   size+=(signo && temp==0 && precision > 1) ? (precision - round(precision*0.5)) : (temp>=1) ? 0 : round(precision*0.5);
 // if(precision>1) size+=(dec>=1 && precision > 1) ? (precision - round(precision*0.5))  : 0;
   }
   for (i = 0; i < NUM_DISPLAYS; i++){output_Displays (~D7Seg); output_bit (transistorPins[i], ~Q); } //Apaga todos los displays
   time = time / ( (TIME_MUX / 1000) * size); //Calcula el numero de ciclos segun el tiempo de multiplexacion que se introdujo
   if (time == 0) time = 1; //Evita que el valor sea 0 y nunca entre al bucle
   for (j = 0; j < time; j++)
   {
      if (limit)
      {
         for (i = 0; i < (size+((signo) ? 1 : 0)); i++)
         {
            output_bit (transistorPins[i], Q); //Satura el transistor dependiendo si son PNP o NPN
            if (i == (size) && signo){temp = 0b01000000; } //Asigna el signo
            else{ temp = tabla7Seg[digito[i]] | ( (i == precision && precision > 0) ? 0b10000000 : 0); } // escoge el numero y añade el punto si es necesario
            output_Displays ( ( (D7Seg) ? temp : ~ (temp))); //Activa la salida dependiendo si es de anodo o catodo comun los displays
            delay_us (TIME_MUX);
            output_bit (transistorPins[i], ~Q) ;//Apaga el transistor
         }

         }else{
         time=1;
         for (i = 0; i < NUM_DISPLAYS; i++) //Indicador que el numero ingresado es mayor de lo permitido
         {
            output_bit (transistorPins[i], Q) ;
            output_Displays ( ( (D7Seg) ? 0b01000000 : ~0b01000000));
            delay_us (TIME_MUX) ;
            output_bit (transistorPins[i], ~Q) ;
         }
      }
   }
}

