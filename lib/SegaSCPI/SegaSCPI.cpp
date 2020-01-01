#include "SegaSCPI.h"

/****************************************************************************
	Funciones de PilaCodigoErrires
*****************************************************************************/
/****************************************************************************
 * Inicializador
*****************************************************************************/
void PilaErrorores::begin(uint8_t maxIn)//Constructor
{
		//Crea la pila "arrayErrores" y la apunta con *arrayErrores
		if (maxIndice > MIN_INDICE && maxIndice < MAX_INDICE)
		{
			arrayErrores = new int[maxIn];
			this->maxIndice = maxIn;//Determina la profundidad de la pila
		}
		else
		{
			arrayErrores = new int[MAX_INDICE];
			this->maxIndice = MAX_INDICE;//Determina la profundidad de la pila

		}
		for (this->indice = 0; this->indice < this->maxIndice; this->indice++)
			arrayErrores[this->indice] = 0;//Toda la pila sin error 
		this->indice = 0;
    
}
/*************************************************************************
   * Error
 **************************************************************************/
int PilaErrorores::error(int codigo)
{
	int retorno;//Valor a devolver -1 o el �ltimo c�digo de error
	if (codigo < -1 || codigo >MAX_CODIGO) return 0;
	switch (codigo)
	{
		case -1://Limpia la pila
			for (this->indice = 0; this->indice < this->maxIndice; this->indice++)
				arrayErrores[this->indice] = 0;//Toda la pila sin error 
			indice = 0;
			retorno = -1;//no tiene que devolver un codigo
			break;
		case 0://Devuelve el último y decrementa el índice
			retorno = arrayErrores[this->indice];
			arrayErrores[this->indice] = 0;
			if (this->indice == 0) this->indice = this->maxIndice - 1;//Decrementa indice o pasa de 0 a maxIndice-1
			else this->indice--;
			break;
		default: //Incrementa el �ndice y a�ade un error
			if (this->indice == this->maxIndice - 1) this->indice = 0;//Si está en el final de la pila sigue en 0
			else this->indice++;//Aumenta el índice
			arrayErrores[this->indice] = codigo;//Almacena el error
			retorno = -1;//No tiene que devolver un codigo
			break;
	}//Switch
		return retorno;// si 0, no hay errores
}
/****************************************************************************
	Fin de funciones de PilaCodigoErrires
*****************************************************************************/
/*****************************************************************************
	Funcion publica: 	Inicializa la pila scpi
***************************************************************************/
//SegaSCPI::SegaSCPI(){}
void SegaSCPI::begin(tipoNivel *pRaiz ,String *codigosErr,String *nombre)//Inicializa la pila
{
 
  nombreSistema=*nombre;
  //Raiz apunta a una estructura tipoNivel cuyo puntero sub apunta al array de
  // estructuras tipoNivel NivelDos
  Raiz=pRaiz; 
  codigosError=codigosErr;
  this->pilaErrores.begin(8);
}
/*****************************************************************************
	Funci�n p�blica: 	
    Funci�n de entrada a SegaSCPI. Se la llama con el  PuertoSCPI a utilizar como par�metro. 
    Lee el buffer del serial en buffCom y lo analiza con "LeeComando" en varias pasadas.
    Cuando se recibe un comando de un submenu SUBMENU:COMANDO [lista de parametros], en una
	primera pasada del bucle while se procesa SUBMENU: y en otra :COMANDO dejando "FinComando" 
	apuntando a la lista de par�metros contenidas en buffCom
*******************************************************************************/
 void SegaSCPI::scpi(HardwareSerial /*USARTClass*/* PuertoSerie) 
{
  PuertoActual=PuertoSerie;
  if (Raiz==NULL)return;//Si no hay men� de comandos salimos
  if (codigosError==NULL)return; //Si no hay array de errores salimos
  locCom=0; // Pone a 0 el �ndice de la cadena por la que recibe del puerto serie
	indComRd = 0; // Pone a cero el indice de la cadena de comando que va a leer
	//locCom=Serial.readBytesUntil('\r',buffCom, BUFFCOM_SIZE);
  locCom=PuertoActual->readBytesUntil('\r',buffCom, BUFFCOM_SIZE);
	buffCom[locCom]='\r'; // A�ade un terminador porque readBytesUntil se lo quita
	buffCom[locCom+1]='\0'; // A�ade un fin de cadena
	locCom++;// Ajusta la longitud de la cadena porque le he a�adido un caracter m�s
  while(locCom)// Mientras haya caracteres en el buffer, sigo ejecutando buscando comandos
	{ 
		char serie[LONG_SCPI];//En "serie" se va concatenando los caracteres leidos
		char c; //en "c" se copia el caracter leido desde buffCom a la cadena serie[]
		unsigned char n = 0;//n es el indice de la cadena serie
    do{// Lee caracteres hasta que encuentra un caracter separador 
			c=CaracterBueno(lee_caracter());//verifica si el car�cter apuntado en buffCom es valido  
			if (!c){errorscpi(1);return;}//Si no es valido da un error y sale
			serie[n] = c; //Si el caracter es valido lo mete en la cadena "serie"
			if (n < LONG_SCPI - 1){n++;}//si no estamos al final f�sico de la cadena avanzamos el indice
			else{errorscpi(3);return;} //Si estamos al final de la cadena da un error y sale
		 }while (!separador(c));//Mientras no encontremos un separador seguimos recorriendo la cadena
			if (c != ':') n--; // En caso de ser un serapador distinto de : lo machaco con un 0x00
			serie[n] = 0;// Los : son transmitidos a EjecutaComandos. otros separadores no
      LeeComandos(serie);//Una vez encontrada una cadena valida la envia a ejecutar
	}//while(locCom)
}//scpi(void)
/*******************************************************************************************
  Funci�n privada: 
  Lee comandos. Se pasa como parametro de entrada una cadena con un comando.
  Si encuentra un comando valido de un submenu, apunta a ese menu y si es
  un comando con puntero a func�on, la ejecuta. Si no encuentra ni una cosa 
  ni otra, apunta al men� raiz. Y si en este tampoco esta el comando, sale 
  con error
********************************************************************************************/
 void SegaSCPI::LeeComandos(char *cadena)
{
  unsigned char a, n, b/* ,NumComandos*/;
  static /*struct*/ tipoNivel  *PNivel = Raiz, *PNivelSup = Raiz;
   while (*cadena == ' ')cadena++; //Limpiamos los blancos del comienzo
  // Si comienza la cadena con ':' hay que rastrear en el raiz 
  if (cadena[0] == ':') {PNivel = Raiz;   return;}// Apunta al raiz y sale
  n = strcspn(cadena, "? :"); // Busca si existen parametros despues de la funcion y 
  if (n < strlen(cadena))    
	{FinComando = cadena + n;}// si existen apunta FinComando a la cadena que los contiene
	b = PNivel->NumNivInf; // b contiene el numero de comandos en este nivel
  //PuertoActual->println("comandos en el nivel comparando cadenas "+String(b)); BORRAR
  
  // Compara el comando con cada uno de los que hay en el nivel
  for (a = 0; a < b; a++) {  
    if (
		(n == strlen(PNivel->sub[a].corto) && !strncmp(cadena, PNivel->sub[a].corto, n))
		 ||
        (n == strlen(PNivel->sub[a].largo) && !strncmp(cadena, PNivel->sub[a].largo, n))
	   ) 
		{
		// Si encuentra el comando en el nivel apuntado
		if (PNivel->sub) {//Si hay un nivel debajo...
			PNivelSup = PNivel;//..guarda el nivel actual..        
			PNivel = &(PNivel->sub[a]);//.. y lo apunta bajando un nivel.
		} //Pero si en vez de un nivel inferior hay una funci�n:
		if (PNivel->pf!=NULL) {  // Si hay una funcion a ejecutar...
			PNivel->pf(); // ...la ejecuto. AQU� SE EJECUTA LA FUNCI�N
			PNivel = PNivelSup;// Restauro el puntero un nivel por encima
		}   
      return;// Tanto si ejecuta el comando como si cambia el nivel apuntado, sale
    }//if ((n == strlen(....
  }//for (a = 0; a < b; a++) 
  // Busqueda del comando en el raiz, despu�s de no encontrarlo en el nivel actual
  if (PNivel != Raiz) { // Si el comando no existe en este nivel...
    PNivelSup = PNivel; // ...guarda el nivel actual...
    PNivel = Raiz;// ...y voy al raiz y..
	LeeComandos(cadena);// busco llamando de forma recursiva a "LeeComando"
    if (PNivel == Raiz) PNivel = PNivelSup; // Si tampoco lo encontr� en el raiz restauro el nivel
  } //if (PNivel != Raiz)                                   
  else errorscpi(2);     // Si ya estaba en el raiz el comando no existe
}
/***********************************************************************
  Funci�n p�blica: Gestiona la pila de errores de segainvex_scpi_Serial
*************************************************************************/
 void SegaSCPI::errorscpi(int codigo) 
 {
	int codigoDevuelto;
	if (codigo < -1) return;// c�digo erroneo
	codigoDevuelto = pilaErrores.error(codigo);
	if (codigoDevuelto != -1) 
   {PuertoActual->println(codigosError[codigoDevuelto]);}
}
/***********************************************************************
  Funci�n privada: Busca un caracter separador : ; \n \r
************************************************************************/
 unsigned char SegaSCPI::separador(char a) {
  if (a == ':' || a == ';' || a == '\n' || a == '\r') return (1);
  return (0);
}
/**************************************************************************
  Funci�n privada: Busca otros caracteres v�lidos . ? * - + '
***************************************************************************/
 unsigned char SegaSCPI::valido(char a) {
  if (a == '?' || a == '.' ||  a == '*'  || a == '-' || a == '+' || a == ',' ) return (1);
  return (0);
}
/**************************************************************************
  Funci�n privada: Verifica si el car�cter es v�lido. (alfanamerico, separador � puntuaci�n)
***************************************************************************/
 char SegaSCPI::CaracterBueno(char caracter) {
  if (!isalnum(caracter))       //Si no es alfanumerico...
    if (!isspace(caracter))    // ni un espacio...
      if (!separador(caracter)) // ni un separador admitido...
        if (!valido(caracter)) // ni un caracter especial...
          return (0);// no es un caracter bueno.
  return (toupper(caracter));   //Si es un caracter bueno lo pasa a mayuscula y lo devuelve
}
/***************************************************************************
    Funci�n privada: Lee caracteres del buffer buffCom[]
**************************************************************************/
 char SegaSCPI::lee_caracter()  {
  unsigned char carRecv;
  if (locCom == 0) return (0x00); //Si no hay caracteres en buffer salimos con caracter nulo 0
  carRecv = buffCom[indComRd];  //Si lo hay,leemos el "char" correspondiente del buffer
  indComRd++;                   //Apuntamos el indice de lectura al siguiente caracter de buffer
  locCom--;                     // Descontamos contador de "caracteres pendientes" en buffCom
  if (indComRd == BUFFCOM_SIZE) indComRd = 0; // Si se sale del buffCom lo ponemos al pricipio
  return (carRecv);             // Salimos devolviendo el caracter leido
}
/*****************************************************************************
	FUNCIONES AUXILIARES PARA CAMBIAR VALORES ENTEROS, DOBLES Y BOOLEANOS EN EL SISTEMA
  
 Estas funciones hay que ejecutarlas dentro de las funciones que responden a comandos 
 scpi y sabemos que hay UN par�metro en el buffer "FinComando". Reciben como par�metro
 principal la direcci�n de la variable que se quiere actualizar.

 Leen un par�metro del array tipo char "FinComando[]".
 
 Si el car�cter FinComando[0] es '?' devuelve el valor actual del par�metro. Si es un espacio,
 lee el par�metro que tiene que estar a continuaci�n, si no est� da un error. Si no es ninguno
 de los anteriores da un error 6. 
 ******************************************************************************/
/*******************************************************************************
   Funci�n p�blica.
   Cambia la variable del sistema tipo double cuya direcci�n se pasa como argumento. Adem�s 
   hay que    pasarle el valor m�ximo, el m�nimo. Devuelve 1 si cambi� la variable. 0, si
   no cambi� la variable por errores y 2 si devolvi� el valor de la variable.
 ********************************************************************************/
 int SegaSCPI::actualizaDec(double *pVariableDelSistema,double Maximo, double Minimo)
{
  double Var; //variable intermedia
  // Si solo pregunta por el dato, se responde y sale
  if(FinComando[0]=='?')  
  {
	  //Serial.println(*pVariableDelSistema);
    ESCRIBE_PUERTO_SERIE(*pVariableDelSistema) 
    return 2;
  }      
  if(FinComando[0]==' ')// Si el primer car�cter de FinComado es 'espacio' lee el par�metro
  {   
    Var=strtod(FinComando,NULL);//Lee un double de la cadena FinComando
  }   
  else {errorscpi(4);return 0;} // Si FinComando no empieza por 'espacio'�Error!Par�metros inexistentes.
// Si el n�mero de par�metros leidos es correcto comprueba los rangos
  if ((Var <= Maximo && Var >= Minimo)){*pVariableDelSistema=Var;return 1;}//si esta en rango,cambia la variable
  else  {errorscpi(6);return 0;}// si no est� en rango da el error correspondiente y sale
}
/**************************************************************************
   Funci�n p�blica.
   Cambia la variable del sistema tipo int cuya direcci�n se pasa como argumento.
    Adem�s hay que pasarle el valor m�ximo y el m�nimo.
   Devuelve 1 si cambi� la variable. 0, si no cambi� la variable por errores.
   Y 2 si devolvi� el valor de la variable.
 ****************************************************************************/
 int SegaSCPI::actualizaInt(int *pVariableDelSistema,int Maximo, int Minimo)
{
  unsigned int np; // n�mero de par�metros leido por sscanf
  int Var; //variable intermedia
  // Si solo pregunta por el dato, se responde y sale
  if(FinComando[0]=='?')  
	{
	  //Serial.println(*pVariableDelSistema);
    ESCRIBE_PUERTO_SERIE(*pVariableDelSistema) 
    return 2;
  }      
   // Si el primer car�cter de FinComado es 'espacio' lee los par�metros
    if(FinComando[0]==' ')  
  {   
    np = sscanf(FinComando,"%u",&Var);// Lee la cadena de par�metros
    if(np != 1){errorscpi(5);return 0;}// Si no lee 1 par�metro�Error!:Formato de par�metro no v�lido
  }   
  else {errorscpi(4);return 0;} // Si el comando no empieza por 'espacio'�Error!Par�metros inexistentes.
// Si el n�mero de par�metros leidos es correcto comprueba los rangos
  if ((Var<=Maximo && Var>=Minimo)){*pVariableDelSistema=Var;return 1;} //si esta en rango, cambia la variable
  else  errorscpi(6);
  {
    return 0;
  }// si no est� en rango da el error "par�metro fuera de rango"
}
/****************************************************************************
 Funci�n p�blica.
 Cambia una variable tipo int que puede tener un conjunto discreto de valores
 Se pasa como argumento un array con los posibles valores y un entero con 
 la cantidad de posibles valores.
 Devuelve 1 si cambi� la variable. 0, si no cambi� la variable por errores.
 Y 2 si devolvi� el valor de la variable.
 *****************************************************************************/
 int SegaSCPI::actualizaDiscr(int *pVariableDelSistema,int ValoresPosibles[],int NumeroValores)// 
{
  unsigned int np; // n�mero de par�metros leido por sscanf
  int i, Var; //variable intermedia
  // Si solo pregunta por el dato, se responde y sale
  if(FinComando[0]=='?')  
  {
    //Serial.println(*pVariableDelSistema);
    ESCRIBE_PUERTO_SERIE(*pVariableDelSistema) 
    return 2;
  }      
   // Si el primer car�cter de FinComado es 'espacio' lee los par�metros
    if(FinComando[0]==' ')  
  {   
    np = sscanf(FinComando,"%u",&Var);// Lee el nuevo valor
    if(np != 1){errorscpi(5);return 0;}// Si no lee 1 par�metro�Error!:Formato de par�metro no v�lido
  }   
  else {errorscpi(4);return 0;} // Si el comando no empieza por 'espacio'�Error!Par�metro inexistente.
// Si se ha leido un par�metro se testea su valor
  for(i=0;i<NumeroValores;i++)
  {
  if (Var == ValoresPosibles[i]) // Si encuentra una coincidencia cambia el valor y sale
  {*pVariableDelSistema=Var;return 1;}
  }
  errorscpi(6);// si no est� en la lista de valores posible da el error "par�metro fuera de rango"
  return 0;
}
/*******************************************************************************
   Funci�n p�blica.
   Cambia la variable del sistema tipo booleana cuya direcci�n se pasa 
   como argumento
 *******************************************************************************/
 int SegaSCPI::actualizaBool(bool *pVariableDelSistema)
{
  unsigned int np; // n�mero de par�metros leido por sscanf
  int Var; //variable intermedia
  // Si solo pregunta por el dato, se responde y sale
  if(FinComando[0]=='?')  
	  {
       //Serial.println(*pVariableDelSistema);
       ESCRIBE_PUERTO_SERIE(*pVariableDelSistema)  
       return 2;
	  }      
   // Si el primer car�cter de FinComado es 'espacio' lee los par�metros
    if(FinComando[0]==' ')  
  {   
    np = sscanf(FinComando,"%u",&Var);// Lee la cadena de par�metros
    if(np != 1){errorscpi(5);return 0;}// Si no lee 1 par�metro�Error!:Formato de par�metro no v�lido
  }   
  else {errorscpi(4);return 0;} // Si el comando no empieza por 'espacio'�Error!Par�metros inexistentes.
// Si el n�mero de par�metros leidos es correcto comprueba los rangos
  if ((Var == 0 || Var == 1)){*pVariableDelSistema=Var;return 1;} // si vale 0 � 1, cambia_la variable
  else  {errorscpi(6); return 0;}// si no est� en rango da el error de "par�metro fuera de rango"
} 
/***************************** FIN ***********************************/
