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
*  Funcion que escribe bitsPorMuestreo bits en la pista a partir de la posicion indicada por bitPos
*  pista: apunta a un vector de short que contiene los muestreos de una pista
*  bitpos: posicion a partir de la cual se desea escribir el muestreo
*  muestreo: valor del muestreo que se desea guardar en bitPos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
void escribirMuestreo ( unsigned short * pista, int bitpos, unsigned short muestreo, int bitsPorMuestreo )
{
  int incio=bitsPorMuestreo;
  while (bitsPorMuestreo)
  {
    pista[bitpos+incio-bitsPorMuestreo]=muestreo;

    bitsPorMuestreo--;
  }


}

/*
*  Funcion que lee bitsPorMuestreo bits de la pista a partir de la posicion indicada por bitPos
*  pista: apunta a un vector de short que contiene los muestreos de una pista
*  bitpos: posicion a partir de la cual se desea leer el muestreo
*  Retorna el valor del muestreo que se encuentra en la posici�n bitPos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
unsigned short leerMuestreo( unsigned short * pista, int bitpos, int bitsPorMuestreo )
{
  unsigned short aRetornar=0;
    //Posición del short en el que se encuntra el bitpos. Se hace la division entera con 16 puesto que un unsigned short tiene 16 bits.
    int posEnArrrglo=bitpos/16;
    //Posición del bit dentro del unsigned short.
    int numBitsDesdeInicio=bitpos%16;

    //Número de bits restantes hasta que se acabe el unisigned short actual. Incluye el bit actual.
    int bitsRestantes=16-numBitsDesdeInicio;

    for (int i = 0; i < bitsPorMuestreo; i++)
    {

    }
    unsigned short actual=pista[posEnArrrglo];
    unsigned short sigueinte=1;
    // Se comprueba si se debe acceder al sigueinte short o no.Además se debe comprobar si es el ultimo short de la cadea porque si lo es
    //habria un error al intentar acceder a posEnArrrglo+1.
    if (bitsPorMuestreo>bitsRestantes)
    {
      sigueinte=pista[posEnArrrglo+1];

      for (int i = numBitsDesdeInicio; i < bitsPorMuestreo-numBitsDesdeInicio+1; i++)
      {


      }

    }
     else
    {

    }





}
/*
*Funcion que retorna la representación en int del bit en la posicion deseada
*indice: shor del cual se quiere saber el valor del bit en al posición
*posiciom: Posición del bit que se desea encontrar.
*/
int darBitEnPosicion(unsigned short indice, int posicion)
{

  //Se empieza a contar desde el inicio del unsigned short de izquierda a derecha, por ende
  // la posición n de izqierda a derecha seria al posicion 15-n de derecha a izquierda.
  int posicionVerdadera=15-posicion;
  //EL uno tiene la ventaja de ser cero en todos sus bits menos en la posicion 15 de izquierda a derecha
  unsigned short uno=1;
  //Correo el unico 1 del uno hacia la posicion deseada
  int mascara =  uno << posicionVerdadera;
  //El & anula todos las posiciones del uno que son 0, por ende solo queda la respuesta en la posicion deseada
  // si hay un 1 en el short buscado sera 1 en la mascaraDeN, si hay un cero ser aun cero
  int mascaraDeN = indice & mascara;
  // Se devuelve la mascara hasta que el bit en al posicion deseada queda en la posicion 15 de iziquierda a derecha
  int aRetornar = mascaraDeN >> posicionVerdadera;
  // Retorna el valor, si el bit en la poscion deseada era 0 se retonra un cero, si era 1 se retorna un 1.
  return aRetornar;
}

/**
* Modifica el bit en la posicion deseada por uno dado por aprametro
*indice:  unsigned short al cual se le va a cambiar el valor deseado
*posicion: posicion del unsigned short en la cual se va a cambiar el bit
*bit: Valor del bit por el cual se va a cambiar.
*/
void cambiarBitEnposicion(unsigned short indice, int posicion,unsigned short bit)
{
  //Inicializa la respuesta
  unsigned short aRetinrar =indice;
  //Cambia la posicion relativa de izquierda a derecha a de derecha a izquierda
  int posicionVerdadera=15-posicion
  //Compruba si debe hacer algun cambio
  if(darBitEnPosicion(indice,posicion)==bit)
    {

    }
      else
    {
      //Se debe hacer un cambio, el operador ^ niega los bits que esten ^1
  unsigned short uno =  1;
  //Asegurarse que solo el bit en la posicion deseada tenga valor 1
  unsigned short mascara=uno << posicionVerdadera;
  //Negar el bit solo en la posicion deseada
  aRetinrar =indice^ mascara;

}

// Retornar la respeusta
  return aRetinrar;

}
/*
*  Funcion para fundir dos pistas monofonicas en una sola estereo
*  parte1: apunta a un vector de short que contiene los muestreos de una pista
*  parte2: apunta a un vector de short que contiene los muestreos de una pista
*  salida: apunta a un vector de short que contendra la fusion de las dos pistas anteriores
*  bitsPorMuestreo: tamanio en bits de los muestreos
*/
//TODO: DESARROLLAR COMPLETAMENTE ESTA FUNCION
void unirArchivosWAVE( unsigned short *parte1, unsigned short *parte2, unsigned short *salida, int bitsPorMuestreo )
{

int tam=sizeof(*parte1)/sizeof(unsigned short);
  for(posActua=0;posActua<tam;posActua++)
  {
    salida[2*posActua]=parte1[posActua];
    salida[2*posActua+1]=parte2[posActua]
  }
  int res=(2*tam)%16
  int otro=res;
while (res)
 {
   salida[2*tam+(otro-res)]=0;
  res--;
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
