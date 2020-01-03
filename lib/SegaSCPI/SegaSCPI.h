#ifndef SegaSCPI_H
#define SegaSCPI_H
//
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif
/*************************************************************************
                CONSTANTES
*************************************************************************/
#define BUFFCOM_SIZE 32 //Longitud del buffer de lectura del comando recibido
#define LONG_SCPI 32 // Longitud máxima del comando sin contar parámetros
/*************************************************************************
                TIPOS
*************************************************************************/
struct tipoNivel
{             
  int   NumNivInf;       // Número de niveles por debajo de este
  const char *largo, *corto;   // Nombre largo y nombre corto del comando
  /*tpf pf*/void (*pf)();// Puntero a función a ejecutar
  tipoNivel *sub;     // Puntero a la estructura de niveles inferiores
};

/****************************************************************************
CLASE PilaErrorores: Monta una pila circular de enteros
para guardar códigos de error. La pila puede tener una profundidad
de MIN_INDICE a MAX_INDICE. los códigos de error que guarda van
de 1 a 255. Para crear un objeto hay que introducir un entero
de de MIN_INDICE a MAX_INDICE si no crea una pila de MAX_INDICE.
Para leer o enviar errores a la pila hay que llamar al método
"error" con un parámetro int entre -1 y MAX_CODIGO. Si es -1
resetea la pila, si es cero devuelve el último codigo de error,
si el código devuelto es 0 es que no hay errores.
Si el código introducido es de 1 a MAX_CODIGO lo guarda en la pila.
La pila es circular y guarda los profundida-1 últimos valores.3
*****************************************************************************/
class PilaErrorores //Pila de codigo de errores
{
  #define MIN_INDICE 4 //Tamaño mínimo de la pila 
  #define MAX_INDICE 16//Tamaño máximo de la pila y Valor por defecto
  #define MAX_CODIGO 255//Los códigos de error posibles son de 1 a MAX_CODIGO
	//private:
  public:
	  uint8_t maxIndice;//Profundidad de la pila de errores
	  uint8_t indice;// Indice de la pila
	  int *arrayErrores;//array de códigos de error
    void begin(uint8_t maxIndice);//Constructor
    int error(int);
 };// FIN CLASE PilaErrorores
  /***********************************************************************/
/*************************************************************************
                CLASE SEGA SCPI
**************************************************************************/ 
class SegaSCPI
{
public:
    //Variables y objetos públicas
      int lonPila=12;
      char *FinComando;// Puntero al final del comando para leer parámetros
      HardwareSerial /*USARTClass*/ *PuertoActual;
      PilaErrorores pilaErrores;
      String nombreSistema;
    //Métodos públicos
      void begin(tipoNivel *,String *,String *);//Inicializa la pila
      void scpi(/*USARTClass**/ HardwareSerial* );//Función principal
      void errorscpi(int); //Gestión de errores
      int actualizaInt(int *,int,int);//Actualiza variable entero
      int actualizaDiscr(int *,int*,int);//Actualiza entero discreta
      int actualizaBool(bool *);//Actualiza Booleano
      int actualizaDec(double *,double,double);//Actualiza decimal
private://Variables privadas  

      String *erroresDelSistema;
      int PuertoSCPI;
      tipoNivel *Raiz;
      //String *codigosError;//Array de cadenas con la descripción de los errores
      char buffCom[BUFFCOM_SIZE]; // Buffer leido del puerto serie con el comando
      int locCom; // Cantidad de caracteres en el buffer
      unsigned char indComRd;// = 0;
    //Métodos privados
      char lee_caracter(void);
      unsigned char separador(char);
      unsigned char valido(char);
      char CaracterBueno(char);
      void LeeComandos(char *cadena);
      String codigosError[16];
      //String* codigosError;//Para generarlo dinámcicamente
};
/***********************************************************************
			        MACROS
************************************************************************/      
// Para definir submenús 
#define SCPI_SUBMENU(X,Y) sizeof(X)/sizeof(*X), #X,#Y,NULL,X,  
// Para definir comandos
#define SCPI_COMANDO(X,Y,Z) 0, #X,#Y,Z,NULL, //Para definir comandos 
//Para definir el nivel raiz de comandos
#define SCPI_RAIZ {sizeof(NivelDos)/sizeof(*NivelDos),"","",NULL,NivelDos}; 
#define MENU_SCPI tipoNivel NivelDos[]= 
/************************************************************************
 Macros para leer y escribir el pueto serie actual
************************************************************************/

/****************************************************************************/
#endif 