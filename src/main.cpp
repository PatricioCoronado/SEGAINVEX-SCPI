#include <Arduino.h>
#include "SegaSCPI.h"
// Prototipos de funciones
void errorSCPI(void);
void opcSCPI(void);
void idnSCPI(void);
void clsSCPI(void);
void miFuncion0(void);
void miFuncion1(void);
/*
void errorSCPI(void);
void opcSCPI(void);
void idnSCPI(void);
void clsSCPI(void);
*/
// Menú de comandos SCPI
MENU_SCPI  //Array de estructuras tipo Nivel
{
    //Comandos definidos por el usuario
	SCPI_COMANDO(FUNCION0,FS0,miFuncion0)
	SCPI_COMANDO(FUNCION1,FS1,miFuncion1)// Envía el ultimo error
	SCPI_COMANDO(ERROR,ERR,errorSCPI)// Envía el ultimo error
  SCPI_COMANDO(*IDN,*IDN,idnSCPI)// Identifica el instrumento
	SCPI_COMANDO(*OPC,*OPC,opcSCPI)// Devuelve un 1 al PC
	SCPI_COMANDO(*CLS,*CLS,clsSCPI)// Borra la pila de errores
};
tipoNivel Raiz[]= SCPI_RAIZ //
/*
  DEFINICION DE ERRORES DE USUARIO
*/
String misErrores[]=
{//Los errores de 0 a 6 son de scpi
"7 mi error con mas texto freno",
"8 mi error malo decicaccc",
"9 mi error no tan biç",
"10 mi error ñ rrty",
"11 mi errorr ",
"12 mi er",
"13 mi ",
};
SegaSCPI segaScpi;//Objeto SCPI
unsigned int milisegundos=3000;
void setup() 
{
  String nombreSistema="Mi sistema";
  segaScpi.begin(Raiz, &nombreSistema,misErrores);
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.flush();
}
void loop() 
{
  if (Serial.available())
  {
      segaScpi.scpi(&Serial);
  }
  if (Serial1.available())
  {
      segaScpi.scpi(&Serial1);
  }
/*
 if(millis() > (milisegundos+5000))
  {
    PuertoActual->println("Estoy esperando un comando"+String(milisegundos));
    milisegundos=millis();
  }
*/
}

/**********************************************************************
    Comando: FUNCION0 o FS0
    
 *********************************************************************/
void miFuncion0(void)
{
  segaScpi.errorscpi(1);
  segaScpi.errorscpi(2);
  segaScpi.errorscpi(3);
  segaScpi.errorscpi(4);
  segaScpi.errorscpi(5);
  segaScpi.errorscpi(6);
  segaScpi.errorscpi(7);
  segaScpi.errorscpi(8);
  segaScpi.errorscpi(9);
  segaScpi.errorscpi(10);
  segaScpi.errorscpi(11);
  segaScpi.errorscpi(12);
  segaScpi.errorscpi(13);
  segaScpi.errorscpi(14);

  //Serial.println("Se ha ejecutado la funcion 0");
  segaScpi.PuertoActual->println("Se ha ejecutado la funcion 0");
}
/**********************************************************************
    Comando: FUNCION1 o FS1
    
 *********************************************************************/
void miFuncion1(void)
{
  segaScpi.PuertoActual->println("Se ha ejecutado la funcion 1");
}
/*********************************************************************/
 /**************************************************************************
    Función privada del Comando: ERROR ó ERR
    Envia por el puerto el último error registrado por SEGAINVEX-SCPI
 **************************************************************************/
void errorSCPI(void)
{
  if (segaScpi.FinComando[0] == '?')segaScpi.errorscpi(0);
	else segaScpi.errorscpi(4);
}
/**************************************************************************
  Función privada del Comando: *IDN"
   Envia por el puerto una cadena que identifica al sistema
 **************************************************************************/
void idnSCPI(void)
{
	//ESCRIBE_PUERTO_SERIE(IdentificacionDelSistema)
  segaScpi.PuertoActual->println(segaScpi.nombreSistema);
}	
 /**************************************************************************
  Función privada del Comando:*OPC
  Envia por el puerto un uno
 **************************************************************************/
void opcSCPI(void)
{
	segaScpi.PuertoActual->println("1");
  //ESCRIBE_PUERTO_SERIE("1")
}
/**************************************************************************
    Comando: CLS
    Limpia la pila de errores de SEGAINVEX-SCPI
 **************************************************************************/
void clsSCPI(void){segaScpi.errorscpi(-1);}
/**************************************************************************/