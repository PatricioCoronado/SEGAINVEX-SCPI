/*
  TODO

  Hay que ver que pasas con la pila de errores.
  Habría que hacer una para cada puerto.
  
*/
#include <Arduino.h>
#include "SegaSCPI.h"
// Prototipos de funciones
void miFuncion0(void);
void miFuncion1(void);
void errorSCPI(void);
void opcSCPI(void);
void idnSCPI(void);
void clsSCPI(void);
// Menú de comandos SCPI
tipoNivel NivelDos[] = //Array de estructuras tipo Nivel
{
    //Comandos definidos por el usuario
	SCPI_COMANDO(FUNCION0,FS0,miFuncion0)
	SCPI_COMANDO(FUNCION1,FS1,miFuncion1)// Envía el ultimo error
	SCPI_COMANDO(ERROR,ERR,errorSCPI)// Envía el ultimo error
  SCPI_COMANDO(*IDN,*IDN,idnSCPI)// Identifica el instrumento
	SCPI_COMANDO(*OPC,*OPC,opcSCPI)// Devuelve un 1 al PC
	SCPI_COMANDO(*CLS,*CLS,clsSCPI)// Borra la pila de errores
};
//SCPI_NIVEL_RAIZ// Macro que hace la declaración obligatoria del nivel Raiz
//tipoNivel miRaiz[]=SCPI_RAIZ
tipoNivel miRaiz[]={sizeof(NivelDos)/sizeof(*NivelDos),"","",NULL,NivelDos};
/*
String misErrores[]=
{
  // Errores del sistema SCPI 0...6
  "0 no hay errores",						// ERROR N. 0
  "1 Caracter no valido",                // ERROR N. 1
  "2 Comando desconocido",               // ERROR N. 2
  "3 Cadena demasiado larga",            // ERROR N. 3
  "4 Parametro inexistente",             // ERROR N. 4
  "5 Formato de parametro no valido",    // ERROR N. 5
  "6 Parametro fuera de rango",          // ERROR N. 6
  // Errores personalizados por el usuario 
  "7 El dato no esta listo",		// ERROR N. 7
};
*/
SegaSCPI MiScpi;//Objeto SCPI
unsigned int milisegundos=3000;
void setup() 
{
  String nombreSistema="Mi sistema";
  MiScpi.begin(miRaiz, &nombreSistema);
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.flush();
}
void loop() 
{
  if (Serial1.available())
  {
      MiScpi.scpi(&Serial1);
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
  //Serial.println("Se ha ejecutado la funcion 0");
  MiScpi.PuertoActual->println("Se ha ejecutado la funcion 0");
}
/**********************************************************************
    Comando: FUNCION1 o FS1
    
 *********************************************************************/
void miFuncion1(void)
{
  MiScpi.PuertoActual->println("Se ha ejecutado la funcion 1");
}
/**********************************************************************
    Comando: ERROR ó ERR
    Envia por el puerto el último error registrado por SEGAINVEX-SCPI
 *********************************************************************/
void errorSCPI(void)
{
  if (MiScpi.FinComando[0] == '?')MiScpi.errorscpi(0);
	else MiScpi.errorscpi(4);
}
/**********************************************************************
    Comando: *IDN"
   Envia por el puerto una cadena que identifica al sistema
 *********************************************************************/
void idnSCPI(void)
{
	//ESCRIBE_PUERTO_SERIE(IdentificacionDelSistema)
  MiScpi.PuertoActual->println(MiScpi.nombreSistema);
}	
 /**********************************************************************
  Comando:*OPC
  Envia por el puerto un uno
 *********************************************************************/
void opcSCPI(void)
{
	MiScpi.PuertoActual->println("1");
  //ESCRIBE_PUERTO_SERIE("1")
}
/**********************************************************************
    Comando: CLS
    Limpia la pila de errores de SEGAINVEX-SCPI
 *********************************************************************/
void clsSCPI(void){MiScpi.errorscpi(-1);}
/*********************************************************************/
 