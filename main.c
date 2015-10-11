#include "STDIO.H"
#include "math.h"
#include "stdlib.h"
#include "string.h"


#define BIT_POR_BYTE   8

//En esta estructura se manejaran las pistas de sonido
struct WaveData {
  unsigned int SoundLength;       //Numero de bytes ocupados por la pista
  unsigned int numSamples;        //Numero de muestreos en la pista
  unsigned int bitsPerSample;     //Numero de bits en cada muestreo
  unsigned short *Sample;         //Secuencia de muestreos
};                                //  numSamples muestreos, cada uno de bitsPerSample bits

struct HeaderType {
  int            RIFF;              //RIFF header
  char           relleno1 [18];     //No lo usamos
  unsigned short Canales;           //canales 1 = mono; 2 = estereo
  int            Frecuencia;        //frecuencia
  int            TasaBit;           //Frecuencia * canales * BitRes/8
  short          AlineamientoBloque;//Alineamento de los boques
  unsigned short BitRes;            //bit resolucion 8/16/32 bit
  int            relleno2;          //No lo usamos
  int            subChunckSize;     // numSamples * canales * BitRes/8
} Header;

void cargarWAVE( struct HeaderType *, struct WaveData *, char * );
int escribirWAVE( struct HeaderType *, struct WaveData *, char * );
void escribirMuestreo ( unsigned short * pista, int bitpos, unsigned short muestreo, int bitsPorMuestreo );
unsigned short leerMuestreo( unsigned short * pista, int bitpos, int bitsPorMuestreo );
void unirArchivosWAVE( unsigned short *, unsigned short *, unsigned short *, int );
int detectarBitsPorMuestreo( struct WaveData * );
void copiarMuestreo(unsigned short *fuente, int *posEntrada, unsigned short *destino, int *posSalida, int bitsPorMuestreo );
void escribir1bit( unsigned short *, int, unsigned short );
void empaquetar( struct WaveData *, int );
void desempaquetar ( struct WaveData *, int );
void corregirHeader( struct HeaderType * );

struct WaveData pistaEntrada1;      //Estructura para la primera pista de entrada
struct WaveData pistaEntrada2;      //Estructura para la segunda pista de entrada
struct WaveData pistaSalida;		    //Estructura para la pista de salida


int main (int argc, char* argv[])
{
	int bitsPorMuestreo;

	if ( argc != 4 ){
		printf( "Faltan argumentos - Deben ser 3 archivos:\n" );
		printf( "  - archivo de entrada 1 (monofonico)\n" );
        printf( "  - archivo de entrada 2 (monofonico)\n" );
        printf( "  - archivo de salida (esfonico)\n" );
		system( "pause" );
		return -1;
	}

	printf( "Archivo fuente 1 %s\n", argv[1] );
	printf( "Archivo fuente  2 %s\n", argv[2] );
	printf( "Archivo Destino %s\n", argv[3] );
	system( "pause" );

	cargarWAVE( &Header, &pistaEntrada1, argv[1] );
	cargarWAVE( &Header, &pistaEntrada2, argv[2] );

	bitsPorMuestreo = detectarBitsPorMuestreo( &pistaEntrada1 );
  if ( bitsPorMuestreo != detectarBitsPorMuestreo( &pistaEntrada2 ) ){
    printf( "Los archivos tienen diferente numero de bits por muestreo\n" );
  }
	empaquetar( &pistaEntrada1, bitsPorMuestreo );
	empaquetar( &pistaEntrada2, bitsPorMuestreo );

	pistaSalida.bitsPerSample = bitsPorMuestreo;
	pistaSalida.numSamples = pistaEntrada1.numSamples;
	pistaSalida.SoundLength = 2*pistaEntrada1.SoundLength;
	pistaSalida.Sample = (unsigned short*)malloc( pistaSalida.SoundLength );

	unirArchivosWAVE( pistaEntrada1.Sample, pistaEntrada2.Sample, pistaSalida.Sample, bitsPorMuestreo );
	corregirHeader( &Header );
	desempaquetar( &pistaSalida, bitsPorMuestreo );
	escribirWAVE( &Header, &pistaSalida, argv[3] );

	printf ("Concluy� exitosamente.\n");
	system("pause");
	return 0;
}

/*
*Funcion que retorna la representación en int del bit en la posicion deseada
*indice: shor del cual se quiere saber el valor del bit en al posición
*posicion: Posición del bit que se desea encontrar.
*cantidad: cantidad de bits deseados.
*/
unsigned short darBitEnPosicion(unsigned short indice, int posicion,int cantidad)
{
	// Posción de izquierda a derecha
	int posicionVerdadera = 15 - posicion;
	//Ayudante de la mascara, genera tantos 1 al final como posiciones deseadas
	unsigned short uno = pow(2, cantidad) - 1;

	//Crea la mascara, dejando los 1 solo en las posciones en las que  se desea concer l valor de los bit
	unsigned short mascara = uno << (posicionVerdadera - cantidad+1);
	//Hace el &, por lo cual solo los valores de bit deseados quedaran en mascar de n
	unsigned short mascaraDeN = indice & mascara;
	//Reotorna los valores de bits deseados en un unsigned short.
	unsigned short aRetornar = mascaraDeN >> (posicionVerdadera-cantidad+1);
	return aRetornar;

}

/*
* Modifica el bit en la posicion deseada por uno dado por aprametro
*indice:  apuntador al unsigned short al cual se le va a cambiar el valor deseado
*posicion: posicion del unsigned short en la cual se va a cambiar el bit
*cantidad: cantidad de bits a cambiar.
*bit: Valor del bit por el cual se va a cambiar.
*/
void cambiarBitEnposicion(unsigned short *indice, int posicion,int cantidad, unsigned short bit)
{


	//Recorre cada uno de los bits deseados
	for (int i = 0; i < cantidad; i++)
	{
		//La posisicón actual de izquierda a derecha del char
		int posicionVerdadera = posicion+i;
		//Posicion de izquierda a derecha del bit
		int posicionEnBit = 15- cantidad +i + 1;
		//Compara si los bits son iguales.
		if (darBitEnPosicion(*indice, posicionVerdadera, 1) == darBitEnPosicion(bit, posicionEnBit, 1))
		{
			//Si son iguales no hay que hacer nada.
		}
		else
		{
			//Crea un 1 que s eencargara de cambiar el valor del bit deseado
			unsigned short uno = 1;
			//Corre el uno hasta la posicion deseada
			unsigned short mascara = uno << 15-posicionVerdadera;
			//Iniverte el bit en la posción deseada
			*indice = *indice^ mascara;

		}
	}

}


/*
*  Funcion que escribe bitsPorMuestreo bits en la pista a partir de la posicion indicada por bitPos
*  pista: apunta a un vector de short que contiene los muestreos de una pista
*  bitpos: posicion a partir de la cual se desea escribir el muestreo
*  muestreo: valor del muestreo que se desea guardar en bitPos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
void escribirMuestreo(unsigned short * pista, int bitpos, unsigned short muestreo, int bitsPorMuestreo)
{
	//Posición del short en el que se encuntra el bitpos. Se hace la division entera con 16 puesto que un unsigned short tiene 16 bits.
	//la posición  empieza en cero.
	int posEnArrrglo = bitpos / 16;
	//Posición del bit dentro del unsigned short.
	int numBitsDesdeInicio = bitpos % 16;

	//Número de bits restantes hasta que se acabe el unisigned short actual. Incluye el bit actual.
	int bitsRestantes = 16 - numBitsDesdeInicio;

	//Número de bits para completar los requeidos
	int bitsSiguiente = bitsPorMuestreo - bitsRestantes;

	//Apuntador al unsigned short actual
	unsigned short *apuntdorAactual = &pista[posEnArrrglo];

	//Apuntador al unsigned short siguiente
	unsigned short *apuntdorAsiguiente = &pista[posEnArrrglo + 1];

	if (bitsPorMuestreo>bitsRestantes)
	{
		//Toma la siguiente posicon

		//Cmabia el incio indice por los bits finales de actual

		cambiarBitEnposicion(apuntdorAactual, numBitsDesdeInicio, bitsRestantes, darBitEnPosicion(muestreo, 0, bitsRestantes));
		//Cambia los bits restantes para completar el meustreo de incio por los inciales de sigueinte
		cambiarBitEnposicion(apuntdorAsiguiente, 0, bitsSiguiente, darBitEnPosicion(muestreo, bitsRestantes, bitsSiguiente));

	}
	else
	{
		//Solocambia los bits inciales de inidice por los de actual a aprtir de la posición especificada.

		cambiarBitEnposicion(apuntdorAactual, numBitsDesdeInicio, bitsPorMuestreo, darBitEnPosicion(muestreo, 0, bitsPorMuestreo));
	}
}

/*
*  Funcion que lee bitsPorMuestreo bits de la pista a partir de la posicion indicada por bitPos
*  pista: apunta a un vector de short que contiene los muestreos de una pista
*  bitpos: posicion a partir de la cual se desea leer el muestreo
*  Retorna el valor del muestreo que se encuentra en la posici�n bitPos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
unsigned short leerMuestreo(unsigned short * pista, int bitpos, int bitsPorMuestreo)
{
	//El miestreo se almacena asi xxxxx00000, donde el numero de x es el tamaño del meustreo y las otras xxx son el valor del meustreo
	unsigned short indice = 0;
	unsigned short *aRetornar = &indice;

	//Posición del short en el que se encuntra el bitpos. Se hace la division entera con 16 puesto que un unsigned short tiene 16 bits.
	//la posición  empieza en cero.
	int posEnArrrglo = bitpos / 16;
	//Posición del bit dentro del unsigned short.
	int numBitsDesdeInicio = bitpos % 16;

	//Número de bits restantes hasta que se acabe el unisigned short actual. Incluye el bit actual.
	int bitsRestantes = 16 - numBitsDesdeInicio;

	//Numero de bits para completar los requeidos
	int bitsSiguiente = bitsPorMuestreo - bitsRestantes;

	unsigned short actual = pista[posEnArrrglo];
	unsigned short sigueinte = 0;
	// Se comprueba si se debe acceder al sigueinte short o no.Además se debe comprobar si es el ultimo short de la cadea porque si lo es
	//habria un error al intentar acceder a posEnArrrglo+1.
	if (bitsPorMuestreo>bitsRestantes)
	{
		//Toma la siguiente posicon
			sigueinte = pista[posEnArrrglo + 1];
	//Cmabia el incio indice por los bits finales de actual
		cambiarBitEnposicion(aRetornar, 0, bitsRestantes, darBitEnPosicion(actual, numBitsDesdeInicio, bitsRestantes));
		//Cambia los bits restantes para completar el meustreo de incio por los inciales de sigueinte
		cambiarBitEnposicion(aRetornar, bitsRestantes, bitsPorMuestreo - bitsRestantes, darBitEnPosicion(sigueinte, 0, bitsSiguiente));

	}
	else
	{
		//Solocambia los bits inciales de inidice por los de actual a aprtir de la posición especificada.
		cambiarBitEnposicion(aRetornar, 0, bitsPorMuestreo, darBitEnPosicion(actual, numBitsDesdeInicio, bitsPorMuestreo));
	}


	return indice;
}


/*
*  Funcion para fundir dos pistas monofonicas en una sola estereo
*  parte1: apunta a un vector de short que contiene los muestreos de una pista
*  parte2: apunta a un vector de short que contiene los muestreos de una pista
*  salida: apunta a un vector de short que contendra la fusion de las dos pistas anteriores
*  bitsPorMuestreo: tamanio en bits de los muestreos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
void unirArchivosWAVE(unsigned short *parte1, unsigned short *parte2, unsigned short *salida, int bitsPorMuestreo)
{
	//TODO definir tamaño
	int tam = 100;
	//Calcula cuantos ceros quedan al final
	int ceros = (tam * 16) % bitsPorMuestreo;
	//Poscion incial
	int posSalida = 0;
	//Numero de muestreos
	int numMuestreos = (tam * 16) / bitsPorMuestreo;

	//para cada muestreo
	for (int i = 0; i < numMuestreos; i++)
	{
		//posciion de la parte 1 de la cual se va a partir
		int posParte1 = i*bitsPorMuestreo;

		//muesteo a poner en la salida
		unsigned short aPoner = leerMuestreo(parte1, posParte1, bitsPorMuestreo);
		//escribir el meusteo
		escribirMuestreo(salida, posSalida, aPoner, bitsPorMuestreo);
		//cambiar la psicion de la salida
		posSalida += bitsPorMuestreo;
		//cammbiar la poscion en el la parte 2
		int posParte2 = i*bitsPorMuestreo;
		// muestreo que se va a escribir
		aPoner = leerMuestreo(parte2, posParte2, bitsPorMuestreo);
	//Escribir el muesteo
		escribirMuestreo(salida, posSalida, aPoner, bitsPorMuestreo);

		//Cambiar la posición en la salida
		posSalida += bitsPorMuestreo;


	}

	//Si hay que poner ceros
	if (ceros != 0)
	{
		//Esocoge el ultimo shar y le pone los ceros
    //Por alguna razón funciona acá pero no al final
		unsigned short *apuntdorAlFinal = &salida[tam - 1];
		cambiarBitEnposicion(apuntdorAlFinal, posSalida%16, ceros, 0);

	}


}

/*
* Funci�n que detecta el numero de bits por muestreo
* NO MODIFICAR
*/
int detectarBitsPorMuestreo( struct WaveData * voice ){
	int posiciones = 0;
	unsigned short sample = voice->Sample[0];

	while ( sample ){
		posiciones++;
		sample <<= 1;
	}

	return posiciones;
}

/*
*  Funcion que copia un muestreo (bitsPorMuestreo) a partir del bit posEntrada de fuente
*  a los bits a partir de la posicion posSalida de destino
*  fuente: apunta a un vector de short que contiene los muestreos de una pista
*  destino: apunta a un vector de short que contiene los muestreos de una pista
*  posEntrada: posicion de fuente (en bits) desde donde se copiara el muestreo
*  posSalida: posicion de destino (en bits) a donde se copiara el muestreo
*  bitsPorMuestreo: tamanio en bits de los muestreos
*/
void copiarMuestreo(unsigned short *fuente, int *posEntrada, unsigned short *destino, int *posSalida, int bitsPorMuestreo )
{
	unsigned short muestreo = leerMuestreo( fuente, *posEntrada, bitsPorMuestreo );
	escribirMuestreo ( destino, *posSalida, muestreo, bitsPorMuestreo );
	*posEntrada += bitsPorMuestreo;
	*posSalida += bitsPorMuestreo;
}

/*
* Funcion para empaquetar los muestreos de una pista
* NO MODIFICAR
*/
void empaquetar ( struct WaveData *pista, int bitsPorMuestreo ){
	int i;
	int posEntrada = 0;
	int posSalida = 0;
    unsigned short * nuevoSample;

	nuevoSample = (unsigned short *)malloc( pista->SoundLength );

	for (i = 0; i < pista->numSamples; i++){
		copiarMuestreo( pista->Sample, &posEntrada, nuevoSample, &posSalida, bitsPorMuestreo );
		posEntrada += ( 2*BIT_POR_BYTE - bitsPorMuestreo );
	}
	pista->bitsPerSample = bitsPorMuestreo;
	free(pista->Sample);
	pista->Sample = nuevoSample;
}

/*
*  Funcion que escribe 1 bit en la pista en la posicion indicada por bitPos
*  pista: apunta a un vector de short que contiene los muestreos de una pista
*  bitpos: posicion del bit de la pista que se desea modificar
*  bit: vale 1 o 0, indicando cu�l es el valor que se desea asignar al bit
*/
void escribir1bit( unsigned short * pista, int bitpos, unsigned short bit ){
	escribirMuestreo ( pista, bitpos, bit, 1 );
}

/*
* Funcion para desempaquetar los muestreos de una pista
* NO MODIFICAR
*/
void desempaquetar ( struct WaveData * pista, int bitsPorMuestreo ){
	int i;
	int j;
	int posEntrada = 0;
	int posSalida = 0;
  unsigned short * nuevoSample;

	nuevoSample = (unsigned short*)malloc( pista->SoundLength );
	for ( i = 0; i < 2*pista->numSamples; i++ ){
		copiarMuestreo( pista->Sample, &posEntrada, nuevoSample, &posSalida, bitsPorMuestreo );
		for ( j = 0; j < 16 - bitsPorMuestreo; j++ ){
			escribir1bit( nuevoSample, posSalida, 0 );
			posSalida++;
		}
	}

	pista->bitsPerSample = 16;
	free( pista->Sample );
	pista->Sample = nuevoSample;

}

/*
*  Carga el archivo WAVE en memoria.
*  NO MODIFICAR.
*/
void cargarWAVE ( struct HeaderType * header, struct WaveData * pista, char * FileName ){
  FILE * WAVFile;

  WAVFile = fopen( FileName, "rb" );
  if ( WAVFile == NULL ){
    printf( "No se puede abrir el archivo (%s)\n", FileName );
    exit( 0 );
  }

  //Cargar el encabezado
  fread( header, 44, 1, WAVFile );

  pista->SoundLength = header->subChunckSize;
  pista->Sample = (unsigned short *)malloc( pista->SoundLength ); //Asignar memoria
  if ( pista->Sample == NULL ){
    printf( "No hay memoria para cargar el archivo (%s)\n", FileName );
    exit( 0 );
  }

  //Check RIFF header
  if ( header->RIFF != 0x46464952 ){
    printf( "El archivo no es de tipo wav (%s)\n", FileName );
    exit( 0 );
  }

  //Check canales
  if ( header->Canales != 1 ){
    printf( "El archivo no es monofonico (%s)\n", FileName );
    exit( 0 );
  }

  //Check resoluci�n bits
  if ( header->BitRes != 16 ){
    printf( "El archivo no es de 16 bits (%s)\n", FileName );
    exit( 0 );
  }

  //Carga los muestreos
  fread( pista->Sample, pista->SoundLength, 1, WAVFile );

  fclose( WAVFile );

  pista->numSamples = header->subChunckSize / 2;

}

/*
* Funcion que escribe un archivo WAVE en su totalidad
* NO MODIFICAR
*/
int escribirWAVE ( struct HeaderType * header, struct WaveData * pista, char * FileName ){
  FILE * WAVFile;

  WAVFile = fopen( FileName, "wb" );

  if ( WAVFile == NULL ){
    printf( "No se puede crear el archivo (%s)\n", FileName );
    return (0);
  }

  fwrite( header,44,1, WAVFile );
  fwrite( pista->Sample, header->subChunckSize, 1, WAVFile );
  return 1;
}

/*
* Funci�n que corrige el header: cambio de monof�nico a estereof�nico
* NO MODIFICAR
*/
void corregirHeader ( struct HeaderType * header ){
	header->Canales = 2;
	header->TasaBit = header->Frecuencia * header->BitRes / 8 * header->Canales;
	header->AlineamientoBloque = header->BitRes / 8 * header->Canales;
	header->subChunckSize = 2*header->subChunckSize;
}
