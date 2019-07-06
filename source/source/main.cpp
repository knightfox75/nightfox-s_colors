// NightFox's Colors
// (c) NightFox 2007 - 2009

// Inicio del proyecto Sabado, 15 de Diciembre del 2007
// Ultima revision Martes, 4 de Agosto del 2009


// Incluye las librerias
#include <PA9.h>			// Librerias PA_LIB
#include <fat.h>			// Librerias FAT


#include <string.h>			// Libreria adicionales de C
#include <unistd.h>


#include "efs_lib.h"		// Librerias EFS


// Incluye los graficos (RAM)
#include "gfx/all_gfx.h"	// Importa todos los graficos (usa los .BIN de DATA)

// Incluye los efectos de sonido (RAM)
#include "boom.h"
#include "click.h"
#include "menumove.h"
#include "clash.h"



// Define constantes
#define BG_SLOTS 40				// Slots para fondos
#define spr_ficha 20			// Primer Sprite de las fichas [n + 80]
#define pal_ficha 1				// Paleta de las fichas
#define spr_siguiente 17		// Primer sprite del marcador de siguiente ficha
#define pal_borrada 2			// Paleta de la animacion de borrado
#define spr_combo 12			// Primer Sprite del combo
#define pal_combo 3				// Paleta del combo
#define spr_comboscore 2		// Primer Sprite del contador de Combo Score



// Define Funciones VC++
void Error(u8 codigo, const char* descripcion);					// Mensages de error para debug
void InitPA(void);				// Inicializa las PA_LIB
void InitFAT(void);				// Inicializa las FAT lib
void WaitForVsync(void);		// Espera al VSYNC y mira s debes de entrar en pausa	
void PauseGame(void);			// Pausa el Juego
void FadeIn(void);				// Efecto Fade In (Exclusivo)
void FadeOut(void);				// Efecto Fade Out (Exclusivo)
void SplashScreens(void);		// Muestra las pantallas de bienvenida
void InitGame(void);			// Inicializa el juego
void CreateSprites(void);		// Crea los Sprites del tablero
void RefreshSprites(void);		// Refresca los Sprites del tablero
u8 MovePiece(void);				// Mueve la ficha y devulve si toca fondo
void ChangePiece(void);			// Calcula la siguiente ficha
u8 AutoDrop(void);				// Caida automatica de la ficha
void BackupArray(void);			// Crea una copia del Array de borrado
void RestoreArray(void);		// Recupera la copia del Array de borrado
void CleanArray(void);			// Vacia el array de borrado y su copia
void CheckRows(void);			// Testea las fichas a borrar en una fila
void CheckColumns(void);		// Testea las fichas a borrar en una columna
void CheckToRight(void);		// Testea las diagonales (>)
void CheckToLeft(void);			// Testea las diagonales (<)
u8 CleanPieces(void);			// Mira si tienes que borrar las fichas
void DestroyPieces(void);		// Anima las piezas a ser borradas
void DropDown(void);			// Rellena los espacios vacios despues de la limpieza
void TurnColor(void);			// Cambia los colores de las fichas al tocarla
void WhiteErase(void);			// Elimina las fichas del color tocado
void Delay(void);				// Retardo de 10 frames
void InitBuffers(void);			// Inicializa los Buffers de carga de graficos
void LoadFatBg(const char* referencia, const char* nombre);	// Carga un fondo desde la FAT a la RAM
void LoadRamBg(u16 pantalla, u16 fondo, const char* referencia);		// Carga un fondo de la RAM a la pantalla
void PlayFatSfx(const char* nombre);	// Reproduce un archivo RAW desde FAT
u8 Menu(void);					// Menu del juego
void CheckSaveGame(void);		// Verifica el savegame
void ShowRecords(void);			// Muestra los records
void EnterYourName(u8 game);	// Entra tu nombre en la table de records
void LoadNewStage(u8 newstage);	// Carga en memoria un nuevo decorado y musica
u8 CheckRecord(void);			// Verifica si has logrado record
void ShowCombo(u8 numero);			// Muestra el combo
void ShowComboScore(u32 numero);	// Muestra el combo score
void HideCombo(void);				// Oculta el combo
void HideComboScore(void);		// Oculta el combo score
s8 MainGame(void);				// Nucleo del juego
void ShowCredits(u8 endgame);	// Muestra los creditos del juego
void GameOver(void);			// Game Over del juego
void GameOptions(void);			// Opciones del juego
void GameComplete(void);		// Animacion al completar el juego
void GameAborted(void);			// Animacion al abortar la partida
void LoadGameData(void);		// Carga todos los fondos en RAM
void LoadScreenAnim(u8 actual, u8 total);		// Barra de progreso de la carga
void Publicidad(void);		// Publicidad del Sponsor



// Declara estructuras

typedef struct {			// Estructura del Savegame
	char control[27];		// Control
	char nombre[17];		// Nombre del jugador
	u32 score;				// Score
	u8 level;				// Nivel
	u16 combo;				// Combo
	s32 comboscore;			// Combo Score
	u8 complete;			// Juego Completado
	u8 options;				// Opciones del juego			
	u32 checksum;			// Checksum	
} savegameinfo;
savegameinfo savegame[18];	// Archivo de savegame

char savegamefile[256];		// Nombre del archivo de guardado

u8 SLOT;					// Guarda el tipo de SLOT




// Buffers de memoria (Graficos) [40] - BG_SLOTS
char* buffer_info[40];		
char* buffer_map[40];		
char* buffer_pal[40];
char* buffer_tiles[40];

typedef struct {		// Informacion de los fondos cargados
	char name[32];
	u32 tiles;	// Tamaño de los tiles
	u32 map;	// Tamaño del mapa
	u8 slot;	// Estado del slot (libre o en uso)
} bgloadinfo;
bgloadinfo bginfo[40];

// Buffers de memoria (Sfx)
char* buffer_sfx;



// Variables Globales
u8 tablero[8][22];			// Tablero de juego
u8 borrado[8][22];			// Fichas a eliminar
u8 backup[8][22];			// Copia de seguridad
u8 ficha[3];				// Ficha actual
u8 fx;						// Coordenada X de la ficha
u8 fy;						// Coordenada Y de la ficha
u8 siguiente[3];			// Siguiente ficha
u16 gfx_ficha[2][13];		// Graficos de los Sprites de las fichas [pantalla][sprite]
u16 gfx_borrada[2][12];		// Graficos de la animacion del borrado	[pantalla][sprite]
u16 gfx_combo[2];			// Graficos del combo (texto)   [sprite]
u16 gfx_comboscore[2];		// Graficos del Score del combo (texto) [sprite]
u16 gfx_numero[2][12];		// Graficos del combo (numeros) [pantalla][sprite]
u8 autodrop;				// Contador del autodrop
u8 nextdrop;				// Limite para el siguiente autodrop
u32 score;					// Puntuacion
u16 tempscore;				// Contador de combo
u8 level;					// Nivel actual
u16 nextlevel;				// Fichas para avanzar de nivel
s16 currentlevel;			// Fichas en el actual nivel

u8 holdleft;				// Control del teclado		
u8 holdright;
u8 holddown;
u8 fastleft;
u8 fastright;
u8 fastdown;

u8 special;					// Ficha especial
u8 nextspecial;

u8 delflag;					// Flag para ejecucion de la rutina de borrado

u16 maxcombo;				// Guarda el maximo combo por jugada
s32 maxcomboscore;			// Score maximo por combo

// Kernel del programa (Main)
int main(void) {

	// Inicializaciones primarias
	InitPA();			// Inicializa las PA_LIB
	InitFAT();			// Inicializa el sistema FAT
	CheckSaveGame();	// Verifica el SaveGame
	InitBuffers();		// Inicializa los Buffers de carga de los fondos

	// Carga TODOS los fondos
	LoadGameData();

	// Publicidad
	// Publicidad();

	// Pantallas de Bienvenida
	SplashScreens();


	// Bucle principal (repite para siempre)
	while (1) {

		// Menu
		switch (Menu()) {	// Lee el menu del juego

			case 1:				// ( Jugar / Play )
				InitGame();			// Resetea los datos iniciales
				MainGame();			// Juega
				// Publicidad();
				SplashScreens();	// Pantallas de Bienvenida
				break;

			case 2:				// ( Records )
				ShowRecords();
				break;

			case 3:				// ( Creditos / Credits )
				ShowCredits(0);
				break;

			case 4:				// ( Opciones )
				GameOptions();
				break;
		}

		PA_WaitForVBL();		// Espera al sincronismo vertical (60fps)

	}

	return 0;

}





/***********************************************************************************
	Area de debug
***********************************************************************************/



// Errores de sistema (debug) //
void Error(u8 codigo, const char* descripcion) {

		PA_Init();					// Inicializa la PA_Lib
		PA_InitVBL();				// Inicializa la VBL estandar
		PA_SetVideoMode(0, 0);		// Inicializa pantalla
		PA_SetVideoMode(1, 0);
		PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
		PA_ResetBgSysScreen(1);
		PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
		PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
		PA_DisableSpecialFx(1);
		PA_InitText(0, 0);			// Inicializa el texto en ambas pantallas
		PA_InitText(1, 0);

		PA_InitCustomText(0, 0, fuente_error);		// Inicializa el texto en ambas pantallas
		PA_InitCustomText(1, 0, fuente_error);		// En la capa 0
		PA_SetTextCol(0, 31, 31, 31);
		PA_SetBrightness(0, -16);		// Brillo bajado
		PA_SetBrightness(1, -16);

		switch (codigo) {		// Mensage segun el codigo

			case 0:
				PA_OutputText(1, 1, 1, "SIN ERRORES");
				PA_OutputText(0, 1, 1, "NO ERRORS");
				break;

			case 1:
				PA_OutputText(1, 1, 1, "EL ARCHIVO");
				PA_OutputText(1, 1, 3, "%s", descripcion);
				PA_OutputText(1, 1, 5, "NO SE HA ENCONTRADO.");
				PA_OutputText(0, 1, 1, "FILE");
				PA_OutputText(0, 1, 3, "%s", descripcion);
				PA_OutputText(0, 1, 5, "NOT FOUND.");
				break;

			case 2:
				PA_OutputText(1, 1, 1, "MEMORIA RAM INSUFICIENTE.");
				PA_OutputText(0, 1, 1, "OUT OF MEMORY.");
				break;

			case 3:
				PA_OutputText(1, 1, 1, "PARAMETRO EN LA FUNCION");
				PA_OutputText(1, 1, 3, "%s", descripcion);
				PA_OutputText(1, 1, 5, "NO VALIDO.");
				PA_OutputText(0, 1, 1, "INVALID ARGUMENT IN");
				PA_OutputText(0, 1, 3, "%s", descripcion);
				PA_OutputText(0, 1, 5, "FUNCTION.");
				break;

			case 4:
				PA_OutputText(1, 1, 1, "EL FONDO");
				PA_OutputText(1, 1, 3, "%s", descripcion);
				PA_OutputText(1, 1, 5, "NO SE HA ENCONTRADO.");
				PA_OutputText(0, 1, 1, "BACKGROUND");
				PA_OutputText(0, 1, 3, "%s", descripcion);
				PA_OutputText(0, 1, 5, "NOT FOUND.");
				break;

			default:
				PA_OutputText(1, 1, 1, "ERROR DESCONOCIDO.");
				PA_OutputText(1, 1, 1, "UNKNOW ERROR.");
				break;
			
		}
		
		PA_OutputText(1, 1, 18, "CODIGO DE ERROR: %d.", codigo);
		PA_OutputText(1, 1, 20, "SISTEMA DETENIDO.");
		PA_OutputText(0, 1, 18, "ERROR CODE: %d.", codigo);
		PA_OutputText(0, 1, 20, "SYSTEM STOPPED.");

		FadeIn();		// Muestra el error
		
		while(1) {		// Repite para siempre
			PA_WaitForVBL();
		}



}





/***********************************************************************************
	Area de Funciones
***********************************************************************************/



// Funcion InitPA();			// Inicializa las PA_LIB
void InitPA(void) {

	PA_Init();					// Inicializa la PA_Lib
	PA_InitVBL();				// Inicializa la VBL estandar
	PA_SetVideoMode(0, 0);		// Inicializa las pantallas con el modo de video correspondiente
	PA_SetVideoMode(1, 0);
	PA_VBLFunctionInit(AS_SoundVBL);		// Asegurate que se sincronizara el sonido
	AS_Init(AS_MODE_MP3 | AS_MODE_SURROUND | AS_MODE_16CH);  // Inicializa la AS_Lib
	AS_SetDefaultSettings(AS_PCM_8BIT, 11025, AS_SURROUND);  // y pon los parametros por defecto
	PA_InitText(0, 0);			// Inicializa el texto en ambas pantallas
	PA_InitText(1, 0);

}

// Funcion InitFAT();			// Inicializa las FATlib
void InitFAT(void) {

	// Inicializa el texto
	PA_SetVideoMode(0, 0);		// Inicializa las pantallas con el modo de video correspondiente
	PA_SetVideoMode(1, 0);
	PA_InitCustomText(0, 0, fuente_system);		// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_system);		// En la capa 0
	PA_SetTextCol(0, 31, 31, 31);
	PA_SetBrightness(0, -16);		// Brillo bajado
	PA_SetBrightness(1, -16);

	// Inicia el sistema FAT
	PA_OutputText(1, 1, 21, "INICIANDO EL SISTEMA EFS...");		// Mensage de error en Castellano
	PA_OutputText(0, 1, 21, "STARTING EFS SYSTEM...");			// Mensage de error en ingles
	FadeIn();

	if(EFS_Init(EFS_AND_FAT | EFS_DEFAULT_DEVICE, NULL)) {
		// Si has iniciado correctamente, define EFS por defecto
		chdir("efs:/");
		// Y detecta si el linker es SLOT 1 o SLOT 2
		if (strcmp(efs_path, "GBA ROM") == 0) {		// Si es SLOT 2 o emulador
			SLOT = 2;
		} else {		// Si es SLOT 1
			SLOT = 1;
		}
	} else {
		PA_OutputText(1, 1, 1, "SI LEES ESTE MENSAGE, TU");		// Mensage de error en Castellano
		PA_OutputText(1, 1, 2, "LINKER NO ES COMPATIBLE");
		PA_OutputText(1, 1, 3, "CON EFS O NO HAS APLICADO");
		PA_OutputText(1, 1, 4, "EL PARCHE DE DLDI.");
		PA_OutputText(1, 1, 7, "SISTEMA DETENIDO.");	
		PA_OutputText(0, 1, 1, "IF YOU'RE READING THIS,");		// Mensage de error en ingles
		PA_OutputText(0, 1, 2, "YOUR LINKER IS NOT EFS");
		PA_OutputText(0, 1, 3, "COMPATIBLE OR YOU");
		PA_OutputText(0, 1, 4, "HAVEN'T PATCHED WITH DLDI.");
		PA_OutputText(0, 1, 7, "SYSTEM STOPPED.");
		while(1) {
			PA_WaitForVBL();
		}
	}

	FadeOut();

}



// Funcion FadeIn();
void FadeIn(void) {
	float brillo = -16;
	u8 control = 1;
	while (control) {
		brillo += 0.5;
		if (brillo >= 0) {
			control = 0;
		}
		PA_SetBrightness(0, int(brillo));
		PA_SetBrightness(1, int(brillo));
		PA_WaitForVBL();		// Espera al sincronismo vertical (60fps)
	}
}



// Funcion FadeOut();
void FadeOut(void) {
	float brillo = 0;
	u8 control = 1;
	while (control) {
		brillo -= 0.5;
		if (brillo <= -16) {
			control = 0;
		}
		PA_SetBrightness(0, int(brillo));
		PA_SetBrightness(1, int(brillo));
		PA_WaitForVBL();		// Espera al sincronismo vertical (60fps)
	}
}



// Funcion SplashScreens();		// Muestra los logos iniciales
void SplashScreens(void) {

	u8 z = 1;					// Variable de control del bucle
	u8 control = 1;				// Control de ejecucion
	u16 tiempo = 0;				// Contador de tiempo

	while (z) {

		switch (control) {

			case 1:		// Baja el brillo
				PA_SetVideoMode(0, 0);		// Inicializa pantalla
				PA_SetVideoMode(1, 0);
				PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
				PA_ResetBgSysScreen(1);
				PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
				PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
				PA_DisableSpecialFx(1);
				PA_SetBrightness(0, -16);
				PA_SetBrightness(1, -16);
				PA_WaitForVBL();
				tiempo = 0;
				control ++;
				break;

			case 2:		// Espera 0,5 segundo
				tiempo ++;
				if (tiempo > 30) {
					tiempo = 0;
					control ++;
				}
				break;

			case 3:		// Carga los logos DEVKITPRO, Visual C++ y PALIB
				LoadRamBg(1, 3, "palib");
				LoadRamBg(0, 3, "devkitpro");
						// Carga musica menu
				AS_MP3StreamPlay("/bgm/bgm00.mp3");
				AS_SetMP3Loop(true);
				AS_SetMP3Volume(127);
				control ++;
				break;

			case 4:		// Sube el brillo
				FadeIn();
				control ++;
				break;

			case 5:		// Espera 2 segundos
				tiempo ++;
				if (
					(tiempo > 120)
					||
					(Pad.Newpress.Anykey)
				){
					tiempo = 0;
					control ++;
				}
				break;

			case 6:		// Baja el brillo
				FadeOut();
				control ++;
				break;

			case 7:
				LoadRamBg(1, 3, "nfoxup");
				LoadRamBg(0, 3, "nfoxdown");
				control ++;
				break;

			case 8:		// Sube el brillo
				FadeIn();
				control ++;
				break;

			case 9:		// Espera 3 segundos
				tiempo ++;
				if (
					(tiempo > 180)
					||
					(Pad.Newpress.Anykey)
				){
					tiempo = 0;
					control ++;
				}
				break;

			case 10:	// Baja el brillo
				FadeOut();
				control ++;
				break;

			case 11:	// Espera 0,5 segundo y sal de la rutina
				tiempo ++;
				if (tiempo > 30) {
					z = 0;
					tiempo = 0;
					control ++;
				}
				break;

			default:
				break;

		}

		PA_WaitForVBL();		// Espera al sincronismo vertical (60fps)

	}

}



// Funcion Publicidad();		// Muestra los logos iniciales
void Publicidad(void) {

	u8 z = 1;					// Variable de control del bucle
	u8 control = 1;				// Control de ejecucion
	u16 tiempo = 0;				// Contador de tiempo

	while (z) {

		switch (control) {

			case 1:		// Baja el brillo
				PA_SetVideoMode(0, 0);		// Inicializa pantalla
				PA_SetVideoMode(1, 0);
				PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
				PA_ResetBgSysScreen(1);
				PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
				PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
				PA_DisableSpecialFx(1);
				PA_SetBrightness(0, -16);
				PA_SetBrightness(1, -16);
				PA_WaitForVBL();
				tiempo = 0;
				control ++;
				break;

			case 2:		// Espera 0,5 segundo
				tiempo ++;
				if (tiempo > 30) {
					tiempo = 0;
					control ++;
				}
				break;

			case 3:		// Carga los logos Evolution Games
				LoadRamBg(1, 3, "neocompo_up");
				LoadRamBg(0, 3, "neocompo_down");
				control ++;
				break;

			case 4:		// Sube el brillo
				FadeIn();
				control ++;
				break;

			case 5:		// Espera 5 segundos
				tiempo ++;
				if (
					(tiempo > 300)
					||
					(Pad.Newpress.Anykey)
				){
					tiempo = 0;
					control ++;
				}
				break;

			case 6:		// Baja el brillo
				FadeOut();
				control ++;
				break;


			case 7:	// Espera 0,5 segundo y sal de la rutina
				tiempo ++;
				if (tiempo > 30) {
					z = 0;
					tiempo = 0;
					control ++;
				}
				break;

			default:
				break;

		}

		PA_WaitForVBL();		// Espera al sincronismo vertical (60fps)

	}

}




// Funcion InitGame();			// Inicializa el juego
void InitGame(void) {

	// Variables locales
	u8 x;
	u8 y;
	u8 n;

	PA_InitRand();				// Inicializa el random

	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);
	PA_SetBrightness(1, -16);
	PA_InitCustomText(0, 0, fuente_ingame);			// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_ingame);			// En la capa 0

	PA_WaitForVBL();		// Espera un ciclo

	// Inicializa las variables
	for (y = 0; y < 22; y ++) {
		for (x = 0; x < 8; x++) {
			tablero[x][y] = 0;		// Matriz de fichas	
			borrado[x][y] = 0;		// Matrix de borrado
			backup[x][y] = 0;		// Copia matrix borrado
		}
	}

	for (n = 0; n < 3; n++) {		
		ficha[n] = int(PA_RandMinMax(1, 90) / 20) + 1;			// Inicializa la ficha
		siguiente[n] = int(PA_RandMinMax(1, 90) / 20) + 1;		// y la siguiente
	}

	fx = 3;								// Coordenada XY de la ficha
	fy = 0;

	tablero[fx][fy] = ficha[0];			// Actualiza el array con la ficha
	tablero[fx][fy + 1] = ficha[1];
	tablero[fx][fy + 2] = ficha[2];

	special = 0;		// Fichas especiales
	nextspecial = 0;

	autodrop = 0;		// Velocidad inicial del Autodrop
	nextdrop = 51;

	holdright = 0;		// Control del automove
	holdleft = 0;
	holddown = 0;
	fastright = 0;
	fastleft = 0;
	fastdown = 0;

	level = 1;			// Inicializa las variables de nivel
	nextlevel = 20;
	currentlevel = 0;
	score = 0;
	tempscore = 0;
	maxcombo = 0;
	maxcomboscore = 0;

	// Carga los graficos de los Sprites
	for (n = 0; n < 2; n++) {
		// Graficos de las fichas
		gfx_ficha[n][0] = PA_CreateGfx(n, (void*)f00_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][1] = PA_CreateGfx(n, (void*)f01_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][2] = PA_CreateGfx(n, (void*)f02_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][3] = PA_CreateGfx(n, (void*)f03_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][4] = PA_CreateGfx(n, (void*)f04_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][5] = PA_CreateGfx(n, (void*)f05_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][6] = PA_CreateGfx(n, (void*)f06_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][7] = PA_CreateGfx(n, (void*)f07_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][8] = PA_CreateGfx(n, (void*)f08_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][9] = PA_CreateGfx(n, (void*)f09_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][10] = PA_CreateGfx(n, (void*)f10_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][11] = PA_CreateGfx(n, (void*)f11_Sprite, OBJ_SIZE_16X16, 1);
		gfx_ficha[n][12] = PA_CreateGfx(n, (void*)f12_Sprite, OBJ_SIZE_16X16, 1);
		// Graficos animacion borrado
		gfx_borrada[n][1] = PA_CreateGfx(n, (void*)a01_Sprite, OBJ_SIZE_16X16, 1);
		gfx_borrada[n][2] = PA_CreateGfx(n, (void*)a02_Sprite, OBJ_SIZE_16X16, 1);
		gfx_borrada[n][3] = PA_CreateGfx(n, (void*)a03_Sprite, OBJ_SIZE_16X16, 1);
		gfx_borrada[n][4] = PA_CreateGfx(n, (void*)a04_Sprite, OBJ_SIZE_16X16, 1);
		gfx_borrada[n][5] = PA_CreateGfx(n, (void*)a05_Sprite, OBJ_SIZE_16X16, 1);
		gfx_borrada[n][11] = PA_CreateGfx(n, (void*)a11_Sprite, OBJ_SIZE_16X16, 1);
		// Graficos del combo (Numeros)
		gfx_numero[n][0] = PA_CreateGfx(n, (void*)n0_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][1] = PA_CreateGfx(n, (void*)n1_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][2] = PA_CreateGfx(n, (void*)n2_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][3] = PA_CreateGfx(n, (void*)n3_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][4] = PA_CreateGfx(n, (void*)n4_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][5] = PA_CreateGfx(n, (void*)n5_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][6] = PA_CreateGfx(n, (void*)n6_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][7] = PA_CreateGfx(n, (void*)n7_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][8] = PA_CreateGfx(n, (void*)n8_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][9] = PA_CreateGfx(n, (void*)n9_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][10] = PA_CreateGfx(n, (void*)n10_Sprite, OBJ_SIZE_16X16, 1);
		gfx_numero[n][11] = PA_CreateGfx(n, (void*)n11_Sprite, OBJ_SIZE_16X16, 1);
	}
	// Graficos del combo (Texto)
	gfx_combo[0] = PA_CreateGfx(0, (void*)combo0_Sprite, OBJ_SIZE_64X32, 1);
	gfx_combo[1] = PA_CreateGfx(0, (void*)combo1_Sprite, OBJ_SIZE_64X32, 1);
	// Graficos del combo score (Texto)
	gfx_comboscore[0] = PA_CreateGfx(1, (void*)comboscore0_Sprite, OBJ_SIZE_64X32, 1);
	gfx_comboscore[1] = PA_CreateGfx(1, (void*)comboscore1_Sprite, OBJ_SIZE_64X32, 1);

}



// Funcion CreateSprites();
void CreateSprites(void) {

	u8 s;	// Sprite
	u8 x;	// Coordenada X del array
	u8 y;	// Coordenada Y del array
	u8 sx;	// Coordenada X del Sprite
	u8 sy;	// Coordeanda Y del Sprite

	// Carga las paletas de los sprites
	PA_LoadSpritePal(1, pal_ficha, (void*)fichas_Pal);
	PA_LoadSpritePal(0, pal_ficha, (void*)fichas_Pal);
	PA_LoadSpritePal(1, pal_borrada, (void*)borradas_Pal);
	PA_LoadSpritePal(0, pal_borrada, (void*)borradas_Pal);
	PA_LoadSpritePal(1, pal_combo, (void*)combo_Pal);
	PA_LoadSpritePal(0, pal_combo, (void*)combo_Pal);

	// Pantalla Superior
	s = spr_ficha;
	for (y = 0; y < 10; y ++) {
		for (x = 0; x < 8; x ++) {
			// Crealo
			sx = (32+(x*16));
			sy = (32+(y*16));
			PA_CreateSpriteFromGfx(1, s, gfx_ficha[1][0], OBJ_SIZE_16X16, 1, pal_ficha, sx, sy);
			// Y Establece prioridad sobre el layer 1
			PA_SetSpritePrio(1, s, 1);
			s ++;
		}
	}

	// Pantalla Inferior
	s = spr_ficha;
	for (y = 0; y < 10; y ++) {
		for (x = 0; x < 8; x ++) {
			// Crealo
			sx = (32+(x*16));
			sy = (y*16);
			PA_CreateSpriteFromGfx(0, s, gfx_ficha[0][0], OBJ_SIZE_16X16, 1, pal_ficha, sx, sy);
			// Y Establece prioridad sobre el layer 1
			PA_SetSpritePrio(0, s, 1);
			s ++;
		}
	}

	// Marcador Siguiente Ficha
	sx = 208;
	sy = 32;
	s = spr_siguiente;
	for (y = 0;	y < 3; y ++) {
		// Crealo
		PA_CreateSpriteFromGfx(1, s, gfx_ficha[1][0], OBJ_SIZE_16X16, 1, pal_ficha, sx, sy);
		// Y Establece prioridad sobre el layer 1
		PA_SetSpritePrio(1, s, 1);
		s ++;
		sy += 16;
	}

	// Marcador Combo
	PA_CreateSpriteFromGfx(0, spr_combo, gfx_combo[0], OBJ_SIZE_64X32, 1, pal_combo, 64, 56);
	PA_SetSpritePrio(0, spr_combo, 1);
	sx = 64;
	for (s = 1; s < 5; s ++) {	// Digitos
		PA_CreateSpriteFromGfx(0, (spr_combo + s), gfx_numero[0][10], OBJ_SIZE_16X16, 1, pal_combo, sx, 80);
		PA_SetSpritePrio(0, (spr_combo + s), 1);
		sx += 16;
	}

	// Marcador Combo Score
	PA_CreateSpriteFromGfx(1, spr_comboscore, gfx_comboscore[0], OBJ_SIZE_64X32, 1, pal_combo, 64, 72);
	PA_SetSpritePrio(1, spr_comboscore, 1);
	sx = 40;
	for (s = 1; s < 8; s ++) {	// Digitos
		PA_CreateSpriteFromGfx(1, (spr_comboscore + s), gfx_numero[1][10], OBJ_SIZE_16X16, 1, pal_combo, sx, 112);
		PA_SetSpritePrio(1, (spr_comboscore + s), 1);
		sx += 16;
	}

}



// Funcion RefreshSprites();		// Refresca los sprites
void RefreshSprites(void) {

	u8 s;	// Sprite
	u8 x;	// Coordenada X
	u8 y;	// Coordenada Y
	u8 f;	// Frame animacion

	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			PA_SetSpriteGfx(1, s, gfx_ficha[1][f]);
			s ++;
		}
	}

	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			PA_SetSpriteGfx(0, s, gfx_ficha[0][f]);
			s ++;
		}
	}

	s = spr_siguiente;				// Actualiza la Ficha "Siguiente"
	for (y = 0;	y < 3; y ++) {
		// Actualiza el frame de animacion
		f = siguiente[y];
		PA_SetSpriteGfx(1, s, gfx_ficha[1][f]);
		s ++;
	}

}



// Funcion MovePiece();				// Mueve la ficha
u8 MovePiece(void) {

	u8 resultado = 0;

	// Teclas pulsadas y mantenidas (derecha)
	if (Pad.Held.Right) {
			holdright ++;
	} else {
		holdright = 0;
	}

	// Tecla FAST (Derecha) R
	if (Pad.Newpress.R) {
		fastright = 1;		// Si la pulsas
	}
	if (Pad.Released.R) {
		fastright = 0;		// Si la sueltas
	}

	if (		// Mira si se pulsa a la derecha
		(Pad.Newpress.R)
		||
		(Pad.Newpress.Right)
		||
		(holdright > 9)
		||
		(fastright && Pad.Held.R)
		) {
			if (		// Mira si puedes moverte a la derecha
				(fx < 7)
				&&
				(tablero[fx + 1][fy] == 0)
				&&
				(tablero[fx + 1][fy + 1] == 0)
				&&
				(tablero[fx + 1][fy + 2] == 0)
				) {
					tablero[fx][fy] = 0;			// Borra la posicion actual
					tablero[fx][fy + 1] = 0;
					tablero[fx][fy + 2] = 0;
					fx ++;							// Actualiza la posicion de la ficha
					tablero[fx][fy] = ficha[0];		// Actualiza el array con la ficha
					tablero[fx][fy + 1] = ficha[1];
					tablero[fx][fy + 2] = ficha[2];
					holdright = 0;					// Borra el automove
			} else {
				holdright = 0;
				fastright = 0;
			}
	}


	// Teclas pulsadas y mantenidas (izquierda)
	if (Pad.Held.Left) {
			holdleft ++;
	} else {
		holdleft = 0;
	}

	// Tecla FAST (Izquierda) L
	if (Pad.Newpress.L) {
		fastleft = 1;		// Si la pulsas
	}
	if (Pad.Released.L) {
		fastleft = 0;		// Si la sueltas
	}

	if (		// Mira si se pulsa a la izquierda
		(Pad.Newpress.L)
		||
		(Pad.Newpress.Left)
		||
		(holdleft > 9)
		||
		(fastleft && Pad.Held.L)
		) {
			if (		// Mira si puedes moverte a la derecha
				(fx > 0)
				&&
				(tablero[fx - 1][fy] == 0)
				&&
				(tablero[fx - 1][fy + 1] == 0)
				&&
				(tablero[fx - 1][fy + 2] == 0)
				) {
					tablero[fx][fy] = 0;			// Borra la posicion actual
					tablero[fx][fy + 1] = 0;
					tablero[fx][fy + 2] = 0;
					fx --;							// Actualiza la posicion de la ficha
					tablero[fx][fy] = ficha[0];		// Actualiza el array con la ficha
					tablero[fx][fy + 1] = ficha[1];
					tablero[fx][fy + 2] = ficha[2];
					holdleft = 0;					// Borra el automove
			} else {
				holdleft = 0;
				fastleft = 0;
			}
	}



	// Teclas pulsadas y mantenidas (abajo)
	if (Pad.Held.Down) {
			holddown ++;
	} else {
		holddown = 0;
	}

	// Tecla FAST (Abajo) B
	if (Pad.Newpress.B) {
		fastdown = 1;		// Si la pulsas
	}
	if (Pad.Released.B) {
		fastdown = 0;		// Si la sueltas
	}

	if (		// Mira si se pulsa abajo
		(Pad.Newpress.B)
		||
		(Pad.Newpress.Down)
		||
		(holddown > 9)
		||
		(fastdown && Pad.Held.B)
		) {
			if (		// Mira si puedes moverte abajo
				(fy < 19)
				&&
				(tablero[fx][fy + 3] == 0)
				) {
					tablero[fx][fy] = 0;			// Borra la posicion actual
					tablero[fx][fy + 1] = 0;
					tablero[fx][fy + 2] = 0;
					fy ++;							// Actualiza la posicion de la ficha
					tablero[fx][fy] = ficha[0];		// Actualiza el array con la ficha
					tablero[fx][fy + 1] = ficha[1];
					tablero[fx][fy + 2] = ficha[2];
					holddown = 0;					// Borra el automove
			} else {	// Si no puedes
				holddown = 0;
				fastdown = 0;
				resultado = 1;		// Devuelve que ha tocado fondo
			}
	}


	if (		// Mira si se pulsa cambiar
		(Pad.Newpress.A)
		||
		(Pad.Newpress.Up)
		) {
			u8 temp[3];				// Array temporal de la ficha
			temp[0] = ficha[0];		// Crea un backup de la ficha
			temp[1] = ficha[1];
			temp[2] = ficha[2];
			ficha[0] = temp[1];
			ficha[1] = temp[2];
			ficha[2] = temp[0];
			tablero[fx][fy] = ficha[0];		// Actualiza el array con la ficha
			tablero[fx][fy + 1] = ficha[1];
			tablero[fx][fy + 2] = ficha[2];
	}

	return resultado;		// Devuelve el resultado

}



// Funcion ChangePiece();		// Cambia la ficha a la siguiente
void ChangePiece(void) {

	u16 tirada;

	ficha[0] = siguiente[0];	// Asigna a la ficha el valor de la previa
	ficha[1] = siguiente[1];
	ficha[2] = siguiente[2];
	special = nextspecial;

	siguiente[0] = int(PA_RandMinMax(1, 90) / 20) + 1;		// Calcula la previa
	siguiente[1] = int(PA_RandMinMax(1, 90) / 20) + 1;
	siguiente[2] = int(PA_RandMinMax(1, 90) / 20) + 1;

	nextspecial = 0;	// Ficha normal

	tirada = int(PA_RandMinMax(0, 1000));

	if (tirada > 950) {		// 5% de posibilidades	
		nextspecial = 1;	// Ficha cambiadora
		siguiente[0] += 5;
		siguiente[1] += 5;
		siguiente[2] += 5;
	}

	if (tirada < 20) {		// 2% de posibilidades
		nextspecial = 2;	// Ficha borradora
		siguiente[0] = 11;
		siguiente[1] = 11;
		siguiente[2] = 11;
	}


	fx = 3;								// Coordenada XY de la ficha
	fy = 0;
	tablero[fx][fy] = ficha[0];			// Actualiza el array con la ficha
	tablero[fx][fy + 1] = ficha[1];
	tablero[fx][fy + 2] = ficha[2];

}


// Funcion AutoDrop();		// Caida automatica de la ficha
u8 AutoDrop(void) {

	u8 resultado = 0;

	if (	// Si no estas pulsando abajo...
		(!fastdown)	
		&&
		(!holddown)
		) {
		autodrop ++;	// Incrementa el contador de espera
	} else {
		autodrop = 0;	// Si estas pulsando, resetealo.
	}

	if (autodrop > nextdrop) {		// Si llegas el limite

		autodrop = 0;	// Resetea el contador

		if (			// Mira si puedes moverte abajo
			(fy < 19)
			&&
			(tablero[fx][fy + 3] == 0)
			) {
				tablero[fx][fy] = 0;			// Borra la posicion actual
				tablero[fx][fy + 1] = 0;
				tablero[fx][fy + 2] = 0;
				fy ++;							// Actualiza la posicion de la ficha
				tablero[fx][fy] = ficha[0];		// Actualiza el array con la ficha
				tablero[fx][fy + 1] = ficha[1];
				tablero[fx][fy + 2] = ficha[2];
		} else {	// Si no puedes
			resultado = 1;		// Devuelve que ha tocado fondo
		}

	}

	return resultado;		// Devuelve si has tocado fondo

}



// Funcion BackupArray();	// Crea una copia del array de borrado
void BackupArray(void) {
	u8 x;
	u8 y;
	for (y = 0; y < 22; y ++) {
		for (x = 0; x < 8; x ++) {
			backup[x][y] = borrado[x][y];
		}
	}
}



// Funcion RestoreArray();	// Restaura el array de borrado
void RestoreArray(void) {
	u8 x;
	u8 y;
	for (y = 0; y < 22; y ++) {
		for (x = 0; x < 8; x ++) {
			borrado[x][y] = backup[x][y];
		}
	}
}



// Funcion CleanArray();	// Vacia el array de borrado y su copia
void CleanArray(void) {
	u8 x;
	u8 y;
	for (y = 0; y < 22; y ++) {
		for (x = 0; x < 8; x ++) {
			borrado[x][y] = 0;
			backup[x][y] = 0;
		}
	}
}



// Funcion CheckRows()		// Mira las fichas a eliminar en las filas
void CheckRows(void) {

	u8 y;	// Variables locales
	u8 x;
	u8 contador;

	BackupArray();	// Crea una copia del array de borrado

	for (y = 2;	y < 22; y ++) {

		contador = 1;

		for (x = 0;	x < 7; x ++) {
			if (
				(tablero[x][y] == tablero[x+1][y])		// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x+1][y] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
		}

		if (contador < 3) {		// Si al terminal la fila, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}

}



// Funcion CheckColumns()		// Mira las fichas a eliminar en las filas
void CheckColumns(void) {

	u8 y;	// Variables locales
	u8 x;
	u8 contador;

	BackupArray();	// Crea una copia del array de borrado

	for (x = 0; x < 8; x ++) {

		contador = 1;

		for (y = 2;	y < 21; y ++) {
			if (
				(tablero[x][y] == tablero[x][y+1])		// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x][y+1] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
		}

		if (contador < 3) {		// Si al terminal la columna, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}

}



// Funcion CheckToRight()		// Mira las fichas a eliminar en las diagonales (>)
void CheckToRight(void) {

	u8 y;	// Variables locales
	u8 x;
	u8 n;
	u8 contador;
	u8 flag;

	BackupArray();	// Crea una copia del array de borrado

	// Eje X
	for (n = 0; n < 6; n ++) {		// De la X = 0 a X = 5

		contador = 1;

		flag = 1;	// Flag arriba
		x = n;		// Asigna coordenadas
		y = 2;

		while (flag) {
			if (
				(tablero[x][y] == tablero[x+1][y+1])	// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x+1][y+1] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
			x ++;	// Incrementa las coordenadas
			y ++;
			if (
				(x > 6)
				||
				(y > 20)
				) {
					flag = 0;	// Flag abajo
			}
		}

		if (contador < 3) {		// Si al terminal la columna, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}


	// Eje Y
	for (n = 2; n < 20; n ++) {		// De la Y = 2 a Y = 19

		contador = 1;

		flag = 1;	// Flag arriba
		x = 0;		// Asigna coordenadas
		y = n;

		while (flag) {
			if (
				(tablero[x][y] == tablero[x+1][y+1])	// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x+1][y+1] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
			x ++;	// Incrementa las coordenadas
			y ++;
			if (
				(x > 6)
				||
				(y > 20)
				) {
					flag = 0;	// Flag abajo
			}
		}

		if (contador < 3) {		// Si al terminal la columna, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}


}



// Funcion CheckToLeft()		// Mira las fichas a eliminar en las diagonales (<)
void CheckToLeft(void) {

	u8 y;	// Variables locales
	u8 x;
	u8 n;
	u8 contador;
	u8 flag;

	BackupArray();	// Crea una copia del array de borrado

	// Eje X
	for (n = 7; n > 1; n --) {		// De la X = 7 a X = 2

		contador = 1;

		flag = 1;	// Flag arriba
		x = n;		// Asigna coordenadas
		y = 2;

		while (flag) {
			if (
				(tablero[x][y] == tablero[x-1][y+1])	// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x-1][y+1] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
			x --;	// Incrementa las coordenadas
			y ++;
			if (
				(x < 1)
				||
				(y > 20)
				) {
					flag = 0;	// Flag abajo
			}
		}

		if (contador < 3) {		// Si al terminal la columna, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}


	// Eje Y
	for (n = 2; n < 20; n ++) {		// De la Y = 2 a Y = 19

		contador = 1;

		flag = 1;	// Flag arriba
		x = 7;		// Asigna coordenadas
		y = n;

		while (flag) {
			if (
				(tablero[x][y] == tablero[x-1][y+1])	// Si son iguales
				&&
				(tablero[x][y] != 0)					// Y no son 0 (casilla vacia)
			) {
				contador ++;			// Suma 1
				borrado[x][y] = 1;		// Y marcalo en el array de borrado
				borrado[x-1][y+1] = 1;
				if (contador > 2) {		// Si hay 3 o mas fichas en linea
					BackupArray();		// Crea un Backup del array
					delflag = 1;		// Activa el flag de borrado
				}
			} else {				// Si no son iguales
				contador = 1;		// Resetea el contador
				RestoreArray();		// Y recupera el array
			}
			x --;	// Incrementa las coordenadas
			y ++;
			if (
				(x < 1)
				||
				(y > 20)
				) {
					flag = 0;	// Flag abajo
			}
		}

		if (contador < 3) {		// Si al terminal la columna, en contador no llega a 3...
			contador = 1;		// Resetea el contador
			RestoreArray();		// Y recupera el array
		}

	}


}



// Funcion CleanPieces();		// Limpia las fichas borradas
u8 CleanPieces(void) {

	u8 x;
	u8 y;
	u8 combo = 0;

	for (y = 0; y < 22; y ++) {
		for (x = 0; x < 8 ; x ++) {
			// PA_OutputText(1, x, y, "%d", borrado[x][y]); 
			if (borrado[x][y] == 1) {		// Si debes de borrar la pieza
				combo ++;					// Suma 1 al combo de borrado
				tablero[x][y] = 0;			// Borrala
				borrado[x][y] = 0;			// Y limpia el array de borrado
			}
		}
	}

	return combo;

}



// Funcion DestroyPieces();		// Animacion del borrado de fichas
void DestroyPieces(void) {

	u8 s;	// Sprite
	u8 x;	// Coordenada X
	u8 y;	// Coordenada Y
	u8 f;	// Frame animacion

	// Cambia el grafico a usar...
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (borrado[x][y]) {	// Si es una ficha borrada...
				PA_SetSpritePal(1, s, pal_borrada);
				PA_SetSpriteGfx(1, s, gfx_borrada[1][f]);
			}
			s ++;
		}
	}
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (borrado[x][y]) {	// Si es una ficha borrada...
				PA_SetSpritePal(0, s, pal_borrada);
				PA_SetSpriteGfx(0, s, gfx_borrada[0][f]);
			}
			s ++;
		}
	}

	// Cambia el frame de la animacion
	for (f = 0; f < 11; f ++) {
		s = spr_ficha;					// Resetea el numero de sprite
		for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
			for (x = 0; x < 8; x ++) {
				// Actualiza el frame de animacion
				if (borrado[x][y]) {	// Si es una ficha borrada...
					PA_SetSpriteAnimEx(1, s, 16, 16, 1, f);
				}
				s ++;
			}
		}
		s = spr_ficha;					// Resetea el numero de sprite
		for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
			for (x = 0; x < 8; x ++) {
				// Actualiza el frame de animacion
				if (borrado[x][y]) {	// Si es una ficha borrada...
					PA_SetSpriteAnimEx(0, s, 16, 16, 1, f);
				}
				s ++;
			}
		}
		for (s = 0; s < 3; s ++) {		// Espera 3 ciclos (1 frame x 3 ciclos)
			WaitForVsync();
		}
	}


	// Restaura las sprites y paletas originales ...
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (borrado[x][y]) {	// Si es una ficha borrada...
				PA_SetSpritePal(1, s, pal_ficha);
				PA_SetSpriteGfx(1, s, gfx_ficha[1][f]);
			}
			s ++;
		}
	}
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (borrado[x][y]) {	// Si es una ficha borrada...
				PA_SetSpritePal(0, s, pal_ficha);
				PA_SetSpriteGfx(0, s, gfx_ficha[0][f]);
			}
			s ++;
		}
	}


}



// Funcion DropDown();		// Rellena los espacios vacios despues de limpiar
void DropDown(void) {

	u8 x;			// Variables de control
	u8 y;
	u8 flag;

	for (x = 0; x < 8; x ++) {	// Examina columna a columna

		flag = 1;

		while (flag) {			// Repitelo mientras queden espacios vacios
			flag = 0;
			for (y = 20; y > 0; y --) {		// Mira cada fila
				if (
					(tablero[x][y] != 0)	// Si existe la ficha
					&&
					(tablero[x][y+1] == 0)	// Y debajo no hay nada
					) {
						tablero[x][y+1] = tablero[x][y];	// Mueve la ficha hacia abajo
						tablero[x][y] = 0;					// Y deja el espacio en blanco
						flag = 1;		// Indica que se repita le test en esta columna
				}
			}

		}

	}

}



// Funcion TurnColor();		// Cambia los colores de las fichas
void TurnColor(void) {

	u8 origen;		// Variables locales
	u8 destino;
	u8 x;
	u8 y;

	ficha[0] -= 5;	// Vuelve las fichas en normales
	ficha[1] -= 5;
	ficha[2] -= 5;

	if (fy < 19) {	// Si tocas una ficha, cambia todas las de ese color por el tuyo
		tablero[fx][fy] = ficha[0];
		tablero[fx][fy + 1] = ficha[1];
		tablero[fx][fy + 2] = ficha[2];
		destino = ficha[2];
		origen = tablero[fx][fy + 3];
		for (y = 2; y < 22; y ++) {
			for (x = 0; x < 8; x ++) {
				if (tablero[x][y] == origen) {
					tablero[x][y] = destino;
				}
			}
		}
		RefreshSprites();	// Refresca el tablero
		WaitForVsync();
		Delay();

	} else {	// Si tocas el suelo, vuelve la ficha normal y randomiza todo el tablero
		tablero[fx][fy] = ficha[0];
		tablero[fx][fy + 1] = ficha[1];
		tablero[fx][fy + 2] = ficha[2];
		for (y = 2; y < 22; y ++) {
			for (x = 0; x < 8; x ++) {
				if (tablero[x][y] != 0) {
					tablero[x][y] = int(PA_RandMinMax(1, 90) / 20) + 1;
				}
			}
		}
		RefreshSprites();	// Refresca el tablero
		WaitForVsync();
		Delay();
	}

}



// Funcion WhiteErase();	// Borra todas las fichas del color que toque
void WhiteErase(void) {

	u8 destino;				// Variables locales
	u8 x;
	u8 y;

	destino = tablero[fx][fy + 3];

	if (fy < 19) {			// Marca para borrar todas las fichas de ese color
		for (y = 2; y < 22; y ++) {
			for (x = 0; x < 8; x ++) {
				if (tablero[x][y] == destino) {
					tablero[x][y] = 11;
					borrado[x][y] = 1;
				}
			}
		}
		borrado[fx][fy] = 1;
		borrado[fx][fy + 1] = 1;
		borrado[fx][fy + 2] = 1;
		RefreshSprites();	// Refresca el tablero
		WaitForVsync();
		Delay();

	} else {
		for (y = 2; y < 22; y ++) {
			for (x = 0; x < 8; x ++) {
				if (tablero[x][y] != 0) {
					tablero[x][y] = 11;
					borrado[x][y] = 1;
				}
			}
		}
		RefreshSprites();	// Refresca el tablero
		WaitForVsync();
		Delay();
	}

	delflag = 1;		// Activa el flag de comprovacion de borrado

}



// Funcion Delay();			// Retardo de 0,5 segundos
void Delay(void) {
	u8 n;
	for (n = 0; n < 10; n ++) {
		WaitForVsync();
	}
}



// Funcion WaitForVsync();		// Espera al VSYNC y mira si debes de entrar en pausa
void WaitForVsync(void) {
	if (		// Si has pulsado el boton de pausa o cerrado la tapa...
		(Pad.Newpress.X)		// Si pulsas X
		||
		(Pad.Newpress.Start)	// pulsas START
		||
		(PA_LidClosed())		// o cierras la tapa
		) {
			PauseGame();		// pausa el juego
	}
	PA_WaitForVBL();			// Espera al sincronismo vertical (60fps)
}



// Funcion PauseGame();		// Pausa el juego
void PauseGame(void) {

	PA_SetBrightness(0, -8);	// Baja el brillo
	PA_SetBrightness(1, -8);
	PA_SetLedBlink(1, 0);		// Led parpadeando
	AS_MP3Pause();				// Pausa la musica
	PA_WaitForVBL();			// Espera al sincronismo

	while (						// Espera a que se pulse la tecla de pausa
		(!Pad.Newpress.X)
		&&
		(!Pad.Newpress.Start)
		) {
			PA_WaitForVBL();
	}

	PA_SetBrightness(0, 0);		// Sube el brillo
	PA_SetBrightness(1, 0);
	PA_SetLedBlink(0, 0);		// Led normal
	AS_MP3Unpause();			// Despausa el MP3
	PA_WaitForVBL();			// Espera al sincronismo
}



// Funcion InitBuffers();		// Inicializa los punteros de carga de los graficos
void InitBuffers(void) {

	u8 x;

	// Buffers de graficos
	for (x = 0; x < BG_SLOTS; x ++) {
		buffer_info[x] = NULL;		
		buffer_map[x] = NULL;			
		buffer_pal[x] = NULL;
		buffer_tiles[x] = NULL;
		bginfo[x].tiles = 0;
		bginfo[x].map = 0;
		bginfo[x].slot = 0;
	}

	// Buffers de sonido
	buffer_sfx = NULL;

}



// Funcion LoadFatBg();		// Carga un fondo desde la FAT
// LoadFatBg(slot, referencia, nombre_archivo)
void LoadFatBg(const char* referencia, const char* nombre) {

	// Busca un slot libre
	u8 n = 0;
	u8 slot = 255;
	for (n = 0; n < BG_SLOTS; n ++) {
		if (bginfo[n].slot == 0) {	// Si esta libre, asignalo y sal
			slot = n;
			bginfo[n].slot = 1;
			n = BG_SLOTS;
		}
	}
	if (slot == 255) {		// Si no hay ningun slot libre, error
		Error(5, NULL);
	}

	free(buffer_info[slot]);		// Vacia el buffer
	buffer_info[slot] = NULL;		
	free(buffer_map[slot]);
	buffer_map[slot] = NULL;
	free(buffer_pal[slot]);
	buffer_pal[slot] = NULL;
	free(buffer_tiles[slot]);
	buffer_tiles[slot] = NULL;

	u32 medida_info = 0;		// Tamaño del archivo a cargar
	u32 medida_map = 0;
	u32 medida_pal = 0;
	u32 medida_tiles = 0;

	FILE* file_info;		// Nombres de archivo
	FILE* file_map;
	FILE* file_pal;
	FILE* file_tiles;

	char filename[256];		// Nombre del archivo

	// Archivo INFO
	sprintf(filename, "/gfx/%s_Info.bin", nombre);	// Abre Info.bin
	file_info = fopen(filename, "rb");
	if (file_info) {		// Si el archivo "Info" existe...
		// Obten el tamaño del archivo
		fseek(file_info, 0, SEEK_END);
		medida_info = ftell(file_info);
		rewind(file_info);
		// Reserva el espacio en RAM
		buffer_info[slot] = (char*) calloc (medida_info, sizeof(char));
		if (buffer_info[slot] == NULL) {
			Error(2, NULL);
		}
		// Carga el archivo en RAM
		fread (buffer_info[slot], 1, medida_info, file_info);
	} else {			// Si no existe...
		Error(1, nombre);
	}
	fclose(file_info);		// Cierra el archivo
	PA_WaitForVBL();		// Espera al cierre de los archivos

	// Archivo MAP
	sprintf(filename, "/gfx/%s_Map.bin", nombre);	// Abre Map.bin
	file_map = fopen(filename, "rb");
	if (file_map) {		// Si el archivo "Map" existe...
		// Obten el tamaño del archivo
		fseek(file_map, 0, SEEK_END);
		medida_map = ftell(file_map);
		rewind(file_map);
		// Reserva el espacio en RAM
		buffer_map[slot] = (char*) calloc (medida_map, sizeof(char));
		if (buffer_map[slot] == NULL) {
			Error(2, NULL);
		}
		// Carga el archivo en RAM
		fread (buffer_map[slot], 1, medida_map, file_map);
	} else {			// Si no existe...
		Error(1, nombre);
	}
	fclose(file_map);		// Cierra el archivo
	PA_WaitForVBL();		// Espera al cierre de los archivos

	// Archivo PAL
	sprintf(filename, "/gfx/%s_Pal.bin", nombre);	// Abre Pal.bin
	file_pal = fopen(filename, "rb");
	if (file_pal) {		// Si el archivo "Pal" existe...
		// Obten el tamaño del archivo
		fseek(file_pal, 0, SEEK_END);
		medida_pal = ftell(file_pal);
		rewind(file_pal);
		// Reserva el espacio en RAM
		buffer_pal[slot] = (char*) calloc (medida_pal, sizeof(char));
		if (buffer_pal[slot] == NULL) {
			Error(2, NULL);
		}
		// Carga el archivo en RAM
		fread (buffer_pal[slot], 1, medida_pal, file_pal);
	} else {			// Si no existe...
		Error(1, nombre);
	}
	fclose(file_pal);		// Cierra el archivo
	PA_WaitForVBL();		// Espera al cierre de los archivos

	// Archivo TILES
	sprintf(filename, "/gfx/%s_Tiles.bin", nombre);	// Abre Tiles.bin
	file_tiles = fopen(filename, "rb");
	if (file_tiles) {		// Si el archivo "Tiles" existe...
		// Obten el tamaño del archivo
		fseek(file_tiles, 0, SEEK_END);
		medida_tiles = ftell(file_tiles);
		rewind(file_tiles);
		// Reserva el espacio en RAM
		buffer_tiles[slot] = (char*) calloc (medida_tiles, sizeof(char));
		if (buffer_tiles[slot] == NULL) {
			Error(2, NULL);
		}
		// Carga el archivo en RAM
		fread (buffer_tiles[slot], 1, medida_tiles, file_tiles);
	} else {			// Si no existe...
		Error(1, nombre);
	}
	fclose(file_tiles);		// Cierra el archivo
	PA_WaitForVBL();		// Espera al cierre de los archivos

	bginfo[slot].tiles = medida_tiles;	// Guarda las medidas del mapa
	bginfo[slot].map = medida_map;
	sprintf(bginfo[slot].name, "%s", referencia);

}



// Funcion LoadRamBg()
void LoadRamBg(u16 pantalla, u16 fondo, const char* referencia) {

	// Busca si existe el archivo
	u8 n;
	u8 slot = 255;
	char nombre[32];

	sprintf(nombre, "%s", referencia);		// Obten el nombre a buscar

	for (n = 0; n < BG_SLOTS; n ++) {
		if (strcmp(nombre, bginfo[n].name) == 0) {	// Si coincide
			slot = n;		// Guarda el SLOT donde esta
			n = BG_SLOTS;	// Y sal
		}
	}

	if (slot == 255) {		// Si el fondo no se encuentra, da error
		Error(4, referencia); 
	}

	PA_DeleteBg(pantalla, fondo);			// Borra el fondo

	// Transfiere el fondo a la VRAM
	PA_EasyBgLoadEx(
					pantalla,						// U8 Screen
					fondo,							// U8 BG Number
					(u32*)buffer_info[slot],		// (u32*) Infos
					(void*)buffer_tiles[slot],		// (void*) Tiles
					bginfo[slot].tiles,				// U32 Tile Size
					(void*)buffer_map[slot],		// (void*) Map
					(bginfo[slot].map / 2),			// U32 Map Size
					(void*)buffer_pal[slot]			// (void*) Palette
					);

}



// Funcion Menu();		// Menu del juego
u8 Menu(void) {

	// Variables
	u16 gfx_boton[7];			// Graficos de los botones [sprite]
	u8 paleta_play = 1;			// Paletas
	u8 paleta_records = 2;
	u8 paleta_credits = 3;
	u8 paleta_opciones = 4;
	s8 sx[4];					// Coordenadas de los botones
	s8 sy[4]; 

	u8 n;						// Variable de uso general
	u8 item;
	float contador;
	u8 in;
	u8 out;
	u8 repeat;


	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);	// Brillo a 0
	PA_SetBrightness(1, -16);

	PA_WaitForVBL();	// Espera un ciclo

	// Carga los Sprites a usar
	// Graficos
	gfx_boton[0] = PA_CreateGfx(0, (void*)menu_play_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[1] = PA_CreateGfx(0, (void*)menu_play_r_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[2] = PA_CreateGfx(0, (void*)menu_records_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[3] = PA_CreateGfx(0, (void*)menu_records_r_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[4] = PA_CreateGfx(0, (void*)menu_credits_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[5] = PA_CreateGfx(0, (void*)menu_credits_r_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[6] = PA_CreateGfx(0, (void*)menu_opciones_Sprite, OBJ_SIZE_32X32, 1);
	// Paletas
	PA_LoadSpritePal(0, paleta_play, (void*)menu_play_Pal);
	PA_LoadSpritePal(0, paleta_records, (void*)menu_records_Pal);
	PA_LoadSpritePal(0, paleta_credits, (void*)menu_credits_Pal);
	PA_LoadSpritePal(0, paleta_opciones, (void*)menu_opciones_Pal);
	// Coordenadas iniciales de los sprites
	sx[0] = 24;
	sy[0] = 24;
	sx[1] = 64;
	sy[1] = 80;
	sx[2] = 104;
	sy[2] = 136;
	sx[3] = 16;
	sy[3] = 144;
	// Crea los Sprites
	PA_CreateSpriteFromGfx(0, 0, gfx_boton[0], OBJ_SIZE_64X32, 1, paleta_play, sx[0], sy[0]);
	PA_CreateSpriteFromGfx(0, 1, gfx_boton[1], OBJ_SIZE_64X32, 1, paleta_play, (sx[0] + 64), sy[0]);
	PA_CreateSpriteFromGfx(0, 2, gfx_boton[2], OBJ_SIZE_64X32, 1, paleta_records, sx[1], sy[1]);
	PA_CreateSpriteFromGfx(0, 3, gfx_boton[3], OBJ_SIZE_64X32, 1, paleta_records, (sx[1] + 64), sy[1]);
	PA_CreateSpriteFromGfx(0, 4, gfx_boton[4], OBJ_SIZE_64X32, 1, paleta_credits, sx[2], sy[2]);
	PA_CreateSpriteFromGfx(0, 5, gfx_boton[5], OBJ_SIZE_64X32, 1, paleta_credits, (sx[2] + 64), sy[2]);
	PA_CreateSpriteFromGfx(0, 6, gfx_boton[6], OBJ_SIZE_32X32, 1, paleta_opciones, sx[3], sy[3]);
	// Y establece prioridad sobre el layer 0 y el frame de animacion
	for (n = 0; n < 6; n ++){
		PA_SetSpritePrio(0, n, 0);
		PA_SetSpriteAnimEx(0, n, 64, 32, 1, 0);
	}
	PA_SetSpritePrio(0, 6, 0);		// Boton opciones
	PA_SetSpriteAnimEx(0, 6, 32, 32, 1, 0);

	// Carga los fondos a usar
	LoadRamBg(1, 3, "titleup");
	LoadRamBg(0, 3, "titledown");
	PA_WaitForVBL();	// Espera un ciclo

	in = 1;				// Inicializa variables
	out = 0;
	contador = -16;
	repeat = 1;
	item = 1;

	while(repeat) {		// Bucle principal

		if (!out) {		// Si no estas en Fade Out...

			// Lee el pad
			if ((Pad.Newpress.Up)&&(item > 1)) {	// Si pulsas arriba
				AS_SoundQuickPlay(menumove);	// Sonido
				item --;
			}
			if ((Pad.Newpress.Down)&&(item < 4)) {	// Si pulsas abajo
				AS_SoundQuickPlay(menumove);	// Sonido
				item ++;
			}

			// Lee el Stylus (Pulsaciones)
			if (PA_SpriteTouched(0) || PA_SpriteTouched(1)) {	// Boton 1
				item = 1;
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					out = 1;
					PlayFatSfx("moveon");	// Sonido aceptacion
				}
			}
			if (PA_SpriteTouched(2) || PA_SpriteTouched(3)) {	// Boton 2
				item = 2;
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					out = 1;
					PlayFatSfx("moveon");	// Sonido aceptacion
				}
			}
			if (PA_SpriteTouched(4) || PA_SpriteTouched(5)) {	// Boton 3
				item = 3;
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					out = 1;
					PlayFatSfx("moveon");	// Sonido aceptacion
				}
			}
			if (PA_SpriteTouched(6)) {							// Boton 4
				item = 4;
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					out = 1;
					PlayFatSfx("moveon");	// Sonido aceptacion
				}
			}

		}

		for (n = 0; n < 6; n ++){					// Resetea los botones
			PA_SetSpriteAnimEx(0, n, 64, 32, 1, 0);
		}
		PA_SetSpriteAnimEx(0, 6, 32, 32, 1, 0);		// (Boton de opciones)

		switch (item) {		// Opciones
			case 1:
				PA_SetSpriteAnimEx(0, 0, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 1, 64, 32, 1, 1);
				break;

			case 2:
				PA_SetSpriteAnimEx(0, 2, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 3, 64, 32, 1, 1);
				break;

			case 3:
				PA_SetSpriteAnimEx(0, 4, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 5, 64, 32, 1, 1);
				break;

			case 4:
				PA_SetSpriteAnimEx(0, 6, 32, 32, 1, 1);
				break;
		}

		if (Pad.Newpress.A && !out) {	// Si se presiona A...
			out = 1;
			PlayFatSfx("moveon");		// Sonido aceptacion
		}

		if (in && !out) {			// Efecto Fade In
			contador += .5;
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (contador == 0) {
				in = 0;
			}
		}

		if (out) {			// Efecto Fade Out
			contador -= .5;
			if (item == 1) {
				AS_SetMP3Volume(int(((16 + contador) * 127) / 16));
			}
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (contador == -16) {
				repeat = 0;
			}
		}

		PA_WaitForVBL();	// Espera un ciclo

	}

	if (item == 1) {		// Si es inicio de partida
		AS_MP3Stop();		// Deten el MP3
		PA_WaitForVBL();	// Espera un ciclo
	}

	return item;		// Devuelve la opcion seleccionada

}



// Funcion CheckSaveGame();		Testea la SRAM
void CheckSaveGame(void) {

	u32 chk = 0;				// Variables para el cheksum
	u8 filefound = 0;
	u8 n = 0;					// Variable para el bucle
	u8 m = 0;
	u16 l;						// Longitud del path
	FILE* savefile;				// Buffer para el archivo de guardado

	PA_SetVideoMode(0, 0);		// Inicializa las pantallas con el modo de video correspondiente
	PA_SetVideoMode(1, 0);
	PA_InitCustomText(0, 0, fuente_system);		// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_system);		// En la capa 0
	PA_SetTextCol(0, 31, 31, 31);
	PA_SetBrightness(0, -16);		// Brillo bajado
	PA_SetBrightness(1, -16);


	if (SLOT == 1) {	// Si el linker es SLOT 1, lee desde FAT

		l = strlen(efs_path);	// Busca el path actual de la ROM
		sprintf(savegamefile, "%s", efs_path);
		savegamefile[l - 3] = 's';	// Y modifica la extension a SAV
		savegamefile[l - 2] = 'a';
		savegamefile[l - 1] = 'v';

		chdir("fat:/");
		PA_WaitForVBL();
		savefile = fopen (savegamefile, "rb");
		PA_WaitForVBL();
		if (savefile) {		// Si el archivo existe
			filefound = 1;
			fread((void*)&savegame, sizeof(savegame), 1, savefile);
			PA_WaitForVBL();
			fclose (savefile);
			PA_WaitForVBL();
		} else {			// Si no existe...
			filefound = 0;
			fclose (savefile);
			PA_WaitForVBL();
		}
		chdir("efs:/");
		PA_WaitForVBL();

	} else {		// Si eres SLOT 2, lee desde SRAM
		// PA_LoadData(1, (void*)&savegame, sizeof(savegame));		// Carga los datos desde la SRAM
		filefound = 1;		// Y da el Ok a archivo encontrado
		sprintf(savegamefile, "%s", "SRAM");
	}

	// Calcula el checksum cargado
	chk = 0;
	for (n = 0; n < 18; n ++) {
		chk += (int(savegame[n].nombre) + savegame[n].score + savegame[n].level + savegame[n].combo + savegame[n].comboscore + savegame[n].complete + savegame[n].options);
	}

	if (
		(chk != savegame[0].checksum)		// Si no supera el checksum
		||
		(!filefound)						// o no existe el archivo
		){

		PA_OutputText(1, 1, 1, "ERROR EN DATOS DE GUARDADO.");		// Texto de error
		PA_OutputText(1, 1, 2, "SE CREARA UN ARCHIVO NUEVO.");
		PA_OutputText(1, 1, 3, "PRESIONA 'A' PARA CONTINUAR");
		PA_OutputText(1, 1, 4, "O APAGA LA CONSOLA.");

		PA_OutputText(0, 1, 1, "SAVEGAME DATA ERROR");				// Texto de error (Ingles)
		PA_OutputText(0, 1, 2, "WILL CREATE A NEW FILE.");
		PA_OutputText(0, 1, 3, "PRESS 'A' TO CONTINUE");
		PA_OutputText(0, 1, 4, "OR SHUTDOWN YOUR CONSOLE.");

		FadeIn();

		while (!Pad.Newpress.A) {	// Espera a que se pulse A
			PA_WaitForVBL();
		}

		PA_OutputText(1, 1, 6, "CREANDO ARCHIVO...");
		PA_OutputText(0, 1, 6, "MAKING FILE...");

		// Cadena de control
		for (n = 0; n < 18; n ++) {
			sprintf(savegame[n].control, "%s", "NightFox's Colors save game");		// Escribe la cadena de control (2/)
		}
		// Resetea las variables de texto
		for (n = 0; n < 18; n ++) {
			for (m = 0; m < 16; m ++) {
				savegame[n].nombre[m] = 32;
			}
			savegame[n].nombre[16] = '\0';
		}
		// Juego Completo "Best Score" (offset 0)
		sprintf(savegame[0].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[0].score = 1;		// Score
		savegame[0].level = 1;		// Level
		savegame[0].combo = 1;		// Max Combo
		savegame[0].comboscore = 1;	// Max comboscore
		savegame[0].complete = 1;	// Juego completado ?

		// Juego Completo "Best Combo" (offset 1)
		sprintf(savegame[1].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[1].score = 1;		// Score
		savegame[1].level = 1;		// Level
		savegame[1].combo = 1;		// Max Combo
		savegame[1].comboscore = 1;	// Max comboscore
		savegame[1].complete = 1;	// Juego completado ?

		// Juego Completo "Best Comboscore" (offset 2)
		sprintf(savegame[2].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[2].score = 1;		// Score
		savegame[2].level = 1;		// Level
		savegame[2].combo = 1;		// Max Combo
		savegame[2].comboscore = 1;	// Max comboscore
		savegame[2].complete = 1;	// Juego completado ?

		// Best Scores Record 1 (offset 3)
		sprintf(savegame[3].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[3].score = 1;		// Score
		savegame[3].level = 1;		// Level
		savegame[3].combo = 1;		// Max Combo
		savegame[3].comboscore = 1;	// Max comboscore
		savegame[3].complete = 1;	// Juego completado ?

		// Best Scores Record 2 (offset 4)
		sprintf(savegame[4].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[4].score = 1;		// Score
		savegame[4].level = 1;		// Level
		savegame[4].combo = 1;		// Max Combo
		savegame[4].comboscore = 1;	// Max comboscore
		savegame[4].complete = 1;	// Juego completado ?

		// Best Scores Record 3 (offset 5)
		sprintf(savegame[5].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[5].score = 1;		// Score
		savegame[5].level = 1;		// Level
		savegame[5].combo = 1;		// Max Combo
		savegame[5].comboscore = 1;	// Max comboscore
		savegame[5].complete = 1;	// Juego completado ?

		// Best Scores Record 4 (offset 6)
		sprintf(savegame[6].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[6].score = 1;		// Score
		savegame[6].level = 1;		// Level
		savegame[6].combo = 1;		// Max Combo
		savegame[6].comboscore = 1;	// Max comboscore
		savegame[6].complete = 1;	// Juego completado ?

		// Best Scores Record 5 (offset 7)
		sprintf(savegame[7].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[7].score = 1;		// Score
		savegame[7].level = 1;		// Level
		savegame[7].combo = 1;		// Max Combo
		savegame[7].comboscore = 1;	// Max comboscore
		savegame[7].complete = 1;	// Juego completado ?

		// Best Combos Record 1 (offset 8)
		sprintf(savegame[8].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[8].score = 1;		// Score
		savegame[8].level = 1;		// Level
		savegame[8].combo = 1;		// Max Combo
		savegame[8].comboscore = 1;	// Max comboscore
		savegame[8].complete = 1;	// Juego completado ?

		// Best Combos Record 2 (offset 9)
		sprintf(savegame[9].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[9].score = 1;		// Score
		savegame[9].level = 1;		// Level
		savegame[9].combo = 1;		// Max Combo
		savegame[9].comboscore = 1;	// Max comboscore
		savegame[9].complete = 1;	// Juego completado ?

		// Best Combos Record 3 (offset 10)
		sprintf(savegame[10].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[10].score = 1;		// Score
		savegame[10].level = 1;		// Level
		savegame[10].combo = 1;		// Max Combo
		savegame[10].comboscore = 1;	// Max comboscore
		savegame[10].complete = 1;	// Juego completado ?

		// Best Combos Record 4 (offset 11)
		sprintf(savegame[11].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[11].score = 1;		// Score
		savegame[11].level = 1;		// Level
		savegame[11].combo = 1;		// Max Combo
		savegame[11].comboscore = 1;	// Max comboscore
		savegame[11].complete = 1;	// Juego completado ?

		// Best Combos Record 5 (offset 12)
		sprintf(savegame[12].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[12].score = 1;		// Score
		savegame[12].level = 1;		// Level
		savegame[12].combo = 1;		// Max Combo
		savegame[12].comboscore = 1;	// Max comboscore
		savegame[12].complete = 1;	// Juego completado ?

		// Best Comboscore Record 1 (offset 13)
		sprintf(savegame[13].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[13].score = 1;		// Score
		savegame[13].level = 1;		// Level
		savegame[13].combo = 1;		// Max Combo
		savegame[13].comboscore = 1;	// Max comboscore
		savegame[13].complete = 1;	// Juego completado ?

		// Best Comboscore Record 2 (offset 14)
		sprintf(savegame[14].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[14].score = 1;		// Score
		savegame[14].level = 1;		// Level
		savegame[14].combo = 1;		// Max Combo
		savegame[14].comboscore = 1;	// Max comboscore
		savegame[14].complete = 1;	// Juego completado ?

		// Best Comboscore Record 3 (offset 15)
		sprintf(savegame[15].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[15].score = 1;		// Score
		savegame[15].level = 1;		// Level
		savegame[15].combo = 1;		// Max Combo
		savegame[15].comboscore = 1;	// Max comboscore
		savegame[15].complete = 1;	// Juego completado ?

		// Best Comboscore Record 4 (offset 16)
		sprintf(savegame[16].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[16].score = 1;		// Score
		savegame[16].level = 1;		// Level
		savegame[16].combo = 1;		// Max Combo
		savegame[16].comboscore = 1;	// Max comboscore
		savegame[16].complete = 1;	// Juego completado ?

		// Best Comboscore Record 5 (offset 17)
		sprintf(savegame[17].nombre, "%s", "NIGHTFOX        ");		// Nombre - (16 char max)
		savegame[17].score = 1;		// Score
		savegame[17].level = 1;		// Level
		savegame[17].combo = 1;		// Max Combo
		savegame[17].comboscore = 1;	// Max comboscore
		savegame[17].complete = 1;	// Juego completado ?

		// Opciones por defecto
		for (n = 0; n < 18; n ++) {		// Resetea todos los marcadores
			savegame[n].options = 0;
		}
		savegame[0].options = 1;		// Muestra el combo
		savegame[1].options = 1;		// Muestra el combo score

		// Cheksum final
		chk = 0;
		for (n = 0; n < 18; n ++) {
			chk += (int(savegame[n].nombre) + savegame[n].score + savegame[n].level + savegame[n].combo + savegame[n].comboscore + savegame[n].complete + savegame[n].options);
		}
		for (n = 0; n < 18; n ++) {
			savegame[n].checksum = chk;
		}


		if (SLOT == 1) {	// Si es SLOT 1

			// Guarda los datos en un archivo en FAT
			chdir("fat:/");
			PA_WaitForVBL();
			savefile = fopen(savegamefile, "wb");
			PA_WaitForVBL();
			fwrite(&savegame, sizeof(savegame), 1, savefile);
			PA_WaitForVBL();
			fclose(savefile);
			PA_WaitForVBL();
			chdir("efs:/");
			PA_WaitForVBL();

		} else {
		// Guarda los archivos en SRAM
			// PA_SaveData(1, &savegame, sizeof(savegame));
		}


		// Textos de ok
		PA_OutputText(1, 1, 8, "ARCHIVO CREADO.");
		PA_OutputText(1, 1, 9, "PRESIONA 'A' PARA CONTINUAR.");
		PA_OutputText(1, 1, 12, "CHECKSUM: %d", chk);
		PA_OutputText(1, 1, 14, "%s", savegamefile);
		PA_OutputText(0, 1, 8, "FILE CREATED.");
		PA_OutputText(0, 1, 9, "PRESS 'A' TO CONTINUE.");
		PA_OutputText(0, 1, 12, "CHECKSUM: %d", chk);
		PA_OutputText(0, 1, 14, "%s", savegamefile);

		while (!Pad.Newpress.A) {	// Espera a que se pulse A
			PA_WaitForVBL();
		}

	} else {	// Si la SRAM esta bien, continua

		PA_OutputText(1, 1, 1, "DATOS DE PARTIDA CORRECTOS.");
		PA_OutputText(1, 1, 2, "INICIANDO...");
		PA_OutputText(1, 1, 12, "CHECKSUM: %d", chk);
		PA_OutputText(1, 1, 14, "%s", savegamefile);
		PA_OutputText(0, 1, 1, "SAVEGAME DATA OK.");
		PA_OutputText(0, 1, 2, "STARTING...");
		PA_OutputText(0, 1, 12, "CHECKSUM: %d", chk);
		PA_OutputText(0, 1, 14, "%s", savegamefile);
		FadeIn();

	}

	FadeOut();

}



// Funcion ShowRecords();	// Muestra los records
void ShowRecords(void) {

	u8 opcion = 1;		// Control de acciones
	u8 page = opcion;	// Pagina actual

	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);		// Brillo a 0
	PA_SetBrightness(1, -16);
	PA_InitCustomText(0, 0, fuente_records);			// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_records);			// En la capa 0
	PA_SetTextCol(0, 31, 31, 31);
	PA_WaitForVBL();	// Espera un ciclo


	while (!Pad.Newpress.B) {

		switch (opcion) {

			case 1:		// Carga pagina 1
				// Carga la pagina 1 de records
				// Carga los fondos
				LoadRamBg(1, 3, "showrecordsp1_up");	
				LoadRamBg(0, 3, "showrecordsp1_down");
				// Carga los textos (superior)
				PA_OutputText(1, 4, 9, "%s", savegame[0].nombre);	// Mejor Puntuacion absoluta
				PA_OutputText(1, 22, 9, "%07d", savegame[0].score);
				PA_OutputText(1, 4, 14, "%s", savegame[1].nombre);	// Mejor Combo absoluto
				PA_OutputText(1, 22, 14, "%03d", savegame[1].combo);
				PA_OutputText(1, 4, 19, "%s", savegame[2].nombre);	// Mejor Comboscore absoluto
				PA_OutputText(1, 22, 19, "%07d", savegame[2].comboscore);
				// Carga los textos (inferior)
				PA_OutputText(0, 4, 9, "%s", savegame[3].nombre);	// Mejor Puntuacion 1
				PA_OutputText(0, 22, 9, "%07d", savegame[3].score);
				PA_OutputText(0, 4, 11, "%s", savegame[4].nombre);	// Mejor Puntuacion 2
				PA_OutputText(0, 22, 11, "%07d", savegame[4].score);
				PA_OutputText(0, 4, 13, "%s", savegame[5].nombre);	// Mejor Puntuacion 3
				PA_OutputText(0, 22, 13, "%07d", savegame[5].score);
				PA_OutputText(0, 4, 15, "%s", savegame[6].nombre);	// Mejor Puntuacion 4
				PA_OutputText(0, 22, 15, "%07d", savegame[6].score);
				PA_OutputText(0, 4, 17, "%s", savegame[7].nombre);	// Mejor Puntuacion 5
				PA_OutputText(0, 22, 17, "%07d", savegame[7].score);

				FadeIn();
				page = opcion;
				opcion = 0;
				break;

			case 2:		// Carga pagina 2
				// Carga la pagina 2 de records
				// Carga los fondos
				LoadRamBg(1, 3, "showrecordsp2_up");	
				LoadRamBg(0, 3, "showrecordsp2_down");
				// Carga los textos (superior)
				PA_OutputText(1, 4, 9, "%s", savegame[8].nombre);	// Mejor combo 1
				PA_OutputText(1, 22, 9, "%03d", savegame[8].combo);
				PA_OutputText(1, 4, 11, "%s", savegame[9].nombre);	// Mejor combo 2
				PA_OutputText(1, 22, 11, "%03d", savegame[9].combo);
				PA_OutputText(1, 4, 13, "%s", savegame[10].nombre);	// Mejor combo 3
				PA_OutputText(1, 22, 13, "%03d", savegame[10].combo);
				PA_OutputText(1, 4, 15, "%s", savegame[11].nombre);	// Mejor combo 4
				PA_OutputText(1, 22, 15, "%03d", savegame[11].combo);
				PA_OutputText(1, 4, 17, "%s", savegame[12].nombre);	// Mejor combo 5
				PA_OutputText(1, 22, 17, "%03d", savegame[12].combo);
				// Carga los textos (inferior)
				PA_OutputText(0, 4, 9, "%s", savegame[13].nombre);	// Mejor Comboscore 1
				PA_OutputText(0, 22, 9, "%07d", savegame[13].comboscore);
				PA_OutputText(0, 4, 11, "%s", savegame[14].nombre);	// Mejor Comboscore 2
				PA_OutputText(0, 22, 11, "%07d", savegame[14].comboscore);
				PA_OutputText(0, 4, 13, "%s", savegame[15].nombre);	// Mejor Comboscore 3
				PA_OutputText(0, 22, 13, "%07d", savegame[15].comboscore);
				PA_OutputText(0, 4, 15, "%s", savegame[16].nombre);	// Mejor Comboscore 4
				PA_OutputText(0, 22, 15, "%07d", savegame[16].comboscore);
				PA_OutputText(0, 4, 17, "%s", savegame[17].nombre);	// Mejor Comboscore 5
				PA_OutputText(0, 22, 17, "%07d", savegame[17].comboscore);
				FadeIn();
				page = opcion;
				opcion = 0;
				break;

			default:
				// No hagas nada
				break;

		}

		if (	// Cambio a la pagina 2
			(
			(Pad.Newpress.A)
			||
			(Pad.Newpress.R)
			||
			(Pad.Newpress.Right)
			)
			&&
			(page == 1)
			) {
				FadeOut();				// Fade Out
				PA_ClearTextBg(1);		// Borra los textos
				PA_ClearTextBg(0);
				opcion = 2;				// Flag para Pagina 2
		}

		if (	// Cambio a la pagina 1
			(
			(Pad.Newpress.Y)
			||
			(Pad.Newpress.L)
			||
			(Pad.Newpress.Left)
			)
			&&
			(page == 2)
			) {
				FadeOut();				// Fade Out
				PA_ClearTextBg(1);		// Borra los textos
				PA_ClearTextBg(0);
				opcion = 1;				// Flag para Pagina 1
		}

		PA_WaitForVBL();	// Espera un ciclo

	}

	FadeOut();	// Efecto Fade Out al Salir

}


// Funcion EnterYourName();		// Entra tu nombre en la tabla de records 
void EnterYourName(u8 game) {

	// Variables
	u16 cursor_grafico;			// Almacena el grafico del cursor
	u8 cursor_paleta = 0;		// Paleta del cursor
	u8 cursor_x = 100;			// Coordenadas del cursor
	u8 cursor_y = 100;
	u8 cursor_speed = 0;		// Animacion del cursor
	u8 cursor_frame = 0;
	u8 cursor_line[5];			// Coordenada Y del teclado
	u8 cursor_row[5];			// Coordenada X del teclaso
	u8 cursor_movex[5];			// Ancho de columna del telcado
	u8 teclado_x = 1;			// Posicion del teclado
	u8 teclado_y = 1;
	u8 ascii[28];				// Alfabeto (en ASCII code)
	char entername[17];			// Nombre del nuevo record (0-15)
	u8 n;
	u8 m;
	u8 posicion;				// Posicion relativa en la tabla
	s8 c = 0;					// Posicion dentro del nombre
	s8 blink = 0;				// Parpadeo del cursor
	u16 display = 0;			// Dislay de la puntuacion
	u16 lineas_grafico;			// Almacena el grafico del cursor
	u8 lineas_paleta = 1;		// Paleta de las lineas
	u8 lineas_x = 64;			// Coordenadas lineas
	float lineas_y = 76;


	s8 in = 1;
	s8 out = 0;
	s8 fade = 0;

	s32 chk = 0;				// Checksum del savedata
	u8 contador = 0;			// Contador de desplazamiento (ordenar records)

	typedef struct {
		char control[27];		// Control
		char nombre[17];		// Nombre del jugador
		u32 score;				// Score
		u8 level;				// Nivel
		u16 combo;				// Combo
		s32 comboscore;			// Combo Score
		u8 complete;			// Juego Completado
		bool options;			// Opciones del juego			
		u32 checksum;			// Checksum
	} datainfo;
	datainfo olddata[18];		

	u8 done = 1;				// Control del bucle EnterName

	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);	// Brillo a 0
	PA_SetBrightness(1, -16);
	PA_InitCustomText(0, 0, fuente_entername);			// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_entername);			// En la capa 0
	PA_SetTextCol(0, 31, 31, 31);
	PA_WaitForVBL();			// Espera un ciclo


	// Inicializa variables de posicion del teclado
	cursor_line[1] = 0;
	cursor_row[1] = 26;
	cursor_movex[1] = 23;
	cursor_line[2] = 30;
	cursor_row[2] = 20;
	cursor_movex[2] = 25;
	cursor_line[3] = 60;
	cursor_row[3] = 12;
	cursor_movex[3] = 28;
	cursor_line[4] = 100;
	cursor_row[4] = 3;
	cursor_movex[4] = 31;

	// Resetea las cadenas de texto
	for (n = 0; n < 18; n ++) {
		for (m = 0; m < 16; m ++) {
			olddata[n].nombre[m] = 32;
		}
		olddata[n].nombre[16] = '\0';
	}
	for (n = 0; n < 16; n ++) {
		entername[n] = 32;
	}
	entername[16] = '\0';

	// Inicializa el alfabeto
	for (n = 0; n < 26; n ++) {
		ascii[n] = (n + 65);
	}
	ascii[26] = int('!');
	ascii[27] = 32;

	// Inicializa la variable del nombre
	for (n = 0; n < 16; n ++) {
		entername[n] = 32;
	}


	// Graficos de las pantallas
	PA_EnableSpecialFx(0, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_OBJ | SFX_BD); // Habilita canal Alpha en la pantalla
	PA_SetSFXAlpha(0, 10, 15);	// Nivel de Alpha
	PA_EnableSpecialFx(1, SFX_ALPHA, 0, SFX_BG0 | SFX_BG1 | SFX_BG2 | SFX_BG3 | SFX_OBJ | SFX_BD); // Habilita canal Alpha en la pantalla
	PA_SetSFXAlpha(1, 10, 15);	// Nivel de Alpha

	LoadRamBg(0, 3, "entername_down");
	LoadRamBg(1, 3, "entername_up");			// Carga fondos

	PA_LoadSpritePal(0, cursor_paleta, (void*)enamecursor_Pal);		// Carga cursor
	cursor_grafico = PA_CreateGfx(0, (void*)enamecursor_Sprite, OBJ_SIZE_64X64, 1);
	PA_CreateSpriteFromGfx(0, 1, cursor_grafico, OBJ_SIZE_64X64, 1, cursor_paleta, cursor_x, cursor_y);
	PA_SetSpriteMode(0, 1, 1);		// Habilita el Alpha
	cursor_x = (cursor_row[teclado_y] + (cursor_movex[teclado_y] * (teclado_x - 1)));
	cursor_y = cursor_line[teclado_y];
	PA_SetSpriteXY(0, 1, cursor_x, cursor_y);

	PA_LoadSpritePal(1, lineas_paleta, (void*)enamelines_Pal);		// Carga lineas monitor
	lineas_grafico = PA_CreateGfx(1, (void*)enamelines_Sprite, OBJ_SIZE_64X64, 1);

	PA_CreateSpriteFromGfx(1, 1, lineas_grafico, OBJ_SIZE_64X64, 1, lineas_paleta, lineas_x, int(lineas_y));
	PA_SetSpriteMode(1, 1, 1);		// Habilita el Alpha
	PA_SetSpriteXY(1, 1, lineas_x, int(lineas_y));
	PA_SetSpritePrio(1, 1, 1);

	PA_CreateSpriteFromGfx(1, 2, lineas_grafico, OBJ_SIZE_64X64, 1, lineas_paleta, (lineas_x + 64), int(lineas_y));
	PA_SetSpriteMode(1, 2, 1);		// Habilita el Alpha
	PA_SetSpriteXY(1, 2, (lineas_x + 64), int(lineas_y));
	PA_SetSpritePrio(1, 2, 1);
	PA_SetSpriteHflip(1, 2, 1);


	// Escribe los textos
	PA_OutputText(1, 8, 13, "%s ", entername);

	// Carga musica ambiental
	AS_MP3StreamPlay("/sfx/compnoise.mp3");
	AS_SetMP3Loop(true);
	AS_SetMP3Volume(0);

	PA_WaitForVBL();		// Espera un ciclo

	while (done) {

		if (in) {		// Fade in
			fade ++;
			PA_SetBrightness(0, (int(((fade * 16) / 60)) - 16));
			PA_SetBrightness(1, (int(((fade * 16) / 60)) - 16));
			AS_SetMP3Volume(int(((fade * 127) / 60)));
			if (fade == 60) {
				in = 0;
			}
		}

		if (out) {		// Fade out
			fade --;
			PA_SetBrightness(0, (int(((fade * 16) / 60)) - 16));
			PA_SetBrightness(1, (int(((fade * 16) / 60)) - 16));
			AS_SetMP3Volume(int(((fade * 127) / 60)));
			if (fade == 0) {
				out = 0;
				done = 0;	// Sal de la rutina
			}
		}

		// Posicion del cursor
		if (Pad.Newpress.Right) {	// Si pulsas derecha
			AS_SoundQuickPlay(click);
			teclado_x += 1;
			if (teclado_x > 7) {
				teclado_x = 1;
			}
		}
		if (Pad.Newpress.Left) {	// Si pulsas izquierda
			AS_SoundQuickPlay(click);
			teclado_x -= 1;
			if (teclado_x < 1) {
				teclado_x = 7;
			}
		}
		if (Pad.Newpress.Up) {		// Si pulsas arriba
			AS_SoundQuickPlay(click);
			teclado_y -= 1;
			if (teclado_y < 1) {
				teclado_y = 4;
			}
		}
		if (Pad.Newpress.Down) {	// Si pulsas abajo
			AS_SoundQuickPlay(click);
			teclado_y += 1;
			if (teclado_y > 4) {
				teclado_y = 1;
			}
		}
		cursor_x = (cursor_row[teclado_y] + (cursor_movex[teclado_y] * (teclado_x - 1)));
		cursor_y = cursor_line[teclado_y];
		PA_SetSpriteXY(0, 1, cursor_x, cursor_y);


		// Animacion cursor
		cursor_speed += 1;
		if (cursor_speed > 7) {
			cursor_speed = 0;
			cursor_frame += 1;
			if (cursor_frame > 3) {
				cursor_frame = 0;
			}
			PA_SetSpriteAnimEx(0, 1, 64, 64, 1, cursor_frame);
		}



		// Almacena las letras en la variable del nombre

		n = ((teclado_x + ((teclado_y - 1) * 7)) - 1);

		if (Pad.Newpress.A && out == 0) {	// Añade la letra
			AS_SoundQuickPlay(click);
			entername[c] = ascii[n];
			c += 1;
			if (c > 15) {
				c = 15;
			}
		}

		if (Pad.Newpress.B && out == 0) {	// Borra la letra
			AS_SoundQuickPlay(click);
			entername[c] = 32;
			c -= 1;
			if (c < 0) {
				c = 0;
			}
			entername[c] = 32;
		}

		if (Pad.Newpress.Start && out == 0) {			// Termina la entrada
			
			PlayFatSfx("done");	// Sonido aceptacion
			out = 1;			// Sal de la rutina

		}

		// Escribe los textos
		PA_OutputText(1, 8, 13, "%s ", entername);
		
		// Cursor parpadeando
		blink ++;
		if (blink > 15) {
			PA_OutputText(1, (8 + c), 13, "_");
		}
		if (blink > 30) {
			blink = 0;
		}

		// Efecto interferencias
		lineas_y -= 0.2;
		if (lineas_y < 60) {
			lineas_y = 76;
		}
		PA_SetSpriteXY(1, 1, lineas_x, int(lineas_y));
		PA_SetSpriteXY(1, 2, (lineas_x + 64), int(lineas_y));

		// Puntuaciones obtenidas
		display ++;
		switch (display) {
			case 1:
				PA_OutputText(1, 8, 16, "               ");
				break;
			case 120:
				PA_OutputText(1, 8, 16, "               ");
				PA_OutputText(1, 8, 16, "Score: %07d", score);
				break;
			case 300:
				PA_OutputText(1, 8, 16, "               ");
				PA_OutputText(1, 8, 16, "Level: %02d", level);
				break;
			case 480:
				PA_OutputText(1, 8, 16, "               ");
				PA_OutputText(1, 8, 16, "Max Combo: %03d", maxcombo);
				break;
			default:
				// Nada
				break;
		}
		if (display > 660) {
			display = 0;
		}

		PA_WaitForVBL();	// Espera al v_sync

	}

	// Deten el MP3 de fondo
	AS_MP3Stop();
	PA_WaitForVBL();

	// Gestion de records 
	for (n = 0; n < 18; n++) {	// Haz una copia de seguridad de la tabla de records
		sprintf(olddata[n].nombre, "%s", savegame[n].nombre);		// Nombre - (16 char max)
		olddata[n].score = savegame[n].score;				// Score
		olddata[n].level = savegame[n].level;				// Level
		olddata[n].combo = savegame[n].combo;				// Max Combo
		olddata[n].comboscore = savegame[n].comboscore;		// Max Comboscore
		olddata[n].complete = savegame[n].complete;			// Juego completado ?
	}

	// Records "Mejores con juego completado" (0 - 2)
	if (level == 25) {	// Si has completado el juego
		if (score > olddata[0].score) {					// Si tienes puntuacion record
			sprintf(savegame[0].nombre, "%s", entername);		// Nombre - (16 char max)
			savegame[0].score = score;					// Score
			savegame[0].level = level;					// Level
			savegame[0].combo = maxcombo;				// Max Combo
			savegame[0].comboscore = maxcomboscore;		// Max combo score
			savegame[0].complete = game;				// Juego completado ?
		}
		if (maxcombo > olddata[1].combo) {				// Si tienes combo record
			sprintf(savegame[1].nombre, "%s", entername);		// Nombre - (16 char max)
			savegame[1].score = score;					// Score
			savegame[1].level = level;					// Level
			savegame[1].combo = maxcombo;				// Max Combo
			savegame[1].comboscore = maxcomboscore;		// Max combo score
			savegame[1].complete = game;				// Juego completado ?
		}
		if (maxcomboscore > olddata[2].comboscore) {	// Si tienes combo record
			sprintf(savegame[2].nombre, "%s", entername);		// Nombre - (16 char max)
			savegame[2].score = score;					// Score
			savegame[2].level = level;					// Level
			savegame[2].combo = maxcombo;				// Max Combo
			savegame[2].comboscore = maxcomboscore;		// Max combo score
			savegame[2].complete = game;				// Juego completado ?
		}
	}

	// Records Score (3 - 7)
	posicion = 0;						// Resetea el contador
	for (n = 7; n > 2; n --) {
		if (score > olddata[n].score) {
			posicion = n;				// Registra la posicion
		}
	}
	if (posicion != 0) {				// Si debes actualizar estos records
		contador = 3;					// Inicializa el contador
		for (n = 3; n < 8; n ++) {		// Actualiza los records
			if (n == posicion) {		// Si es este, actualizalo
				sprintf(savegame[n].nombre, "%s", entername);		// Nombre - (16 char max)
				savegame[n].score = score;					// Score
				savegame[n].level = level;					// Level
				savegame[n].combo = maxcombo;				// Max Combo
				savegame[n].comboscore = maxcomboscore;		// Max combo score
				savegame[n].complete = game;				// Juego completado ?
			} else {
				sprintf(savegame[n].nombre, "%s", olddata[contador].nombre);		// Nombre - (16 char max)
				savegame[n].score = olddata[contador].score;				// Score
				savegame[n].level = olddata[contador].level;				// Level
				savegame[n].combo = olddata[contador].combo;				// Max Combo
				savegame[n].comboscore = olddata[contador].comboscore;		// Max Comboscore
				savegame[n].complete = olddata[contador].complete;			// Juego completado ?
				contador ++;
			}
		}
	}

	// Records Combo (8 - 12)
	posicion = 0;						// Resetea el contador
	for (n = 12; n > 7; n --) {
		if (maxcombo > olddata[n].combo) {
			posicion = n;				// Registra la posicion
		}
	}
	if (posicion != 0) {				// Si debes actualizar estos records
		contador = 8;					// Inicializa el contador
		for (n = 8; n < 13; n ++) {		// Actualiza los records
			if (n == posicion) {		// Si es este, actualizalo
				sprintf(savegame[n].nombre, "%s", entername);		// Nombre - (16 char max)
				savegame[n].score = score;					// Score
				savegame[n].level = level;					// Level
				savegame[n].combo = maxcombo;				// Max Combo
				savegame[n].comboscore = maxcomboscore;		// Max combo score
				savegame[n].complete = game;				// Juego completado ?
			} else {
				sprintf(savegame[n].nombre, "%s", olddata[contador].nombre);		// Nombre - (16 char max)
				savegame[n].score = olddata[contador].score;				// Score
				savegame[n].level = olddata[contador].level;				// Level
				savegame[n].combo = olddata[contador].combo;				// Max Combo
				savegame[n].comboscore = olddata[contador].comboscore;		// Max Comboscore
				savegame[n].complete = olddata[contador].complete;			// Juego completado ?
				contador ++;
			}
		}
	}


	// Records Combo Score (13 - 17)
	posicion = 0;						// Resetea el contador
	for (n = 17; n > 12; n --) {
		if (maxcomboscore > olddata[n].comboscore) {
			posicion = n;				// Registra la posicion
		}
	}
	if (posicion != 0) {				// Si debes actualizar estos records
		contador = 13;					// Inicializa el contador
		for (n = 13; n < 18; n ++) {	// Actualiza los records
			if (n == posicion) {		// Si es este, actualizalo
				sprintf(savegame[n].nombre, "%s", entername);		// Nombre - (16 char max)
				savegame[n].score = score;					// Score
				savegame[n].level = level;					// Level
				savegame[n].combo = maxcombo;				// Max Combo
				savegame[n].comboscore = maxcomboscore;		// Max combo score
				savegame[n].complete = game;				// Juego completado ?
			} else {
				sprintf(savegame[n].nombre, "%s", olddata[contador].nombre);		// Nombre - (16 char max)
				savegame[n].score = olddata[contador].score;				// Score
				savegame[n].level = olddata[contador].level;				// Level
				savegame[n].combo = olddata[contador].combo;				// Max Combo
				savegame[n].comboscore = olddata[contador].comboscore;		// Max Comboscore
				savegame[n].complete = olddata[contador].complete;			// Juego completado ?
				contador ++;
			}
		}
	}


	// Cheksum final
	chk = 0;
	for (n = 0; n < 18; n ++) {
		chk += (int(savegame[n].nombre) + savegame[n].score + savegame[n].level + savegame[n].combo + savegame[n].comboscore + savegame[n].complete + savegame[n].options);
	}
	savegame[0].checksum = chk;

	FILE* savefile;				// Buffer para el archivo de guardado

	if (SLOT == 1) {	// Si es SLOT 1

		// Guarda los datos en un archivo en FAT
		chdir("fat:/");
		PA_WaitForVBL();
		savefile = fopen(savegamefile, "wb");
		PA_WaitForVBL();
		fwrite(&savegame, sizeof(savegame), 1, savefile);
		PA_WaitForVBL();
		fclose(savefile);
		PA_WaitForVBL();
		chdir("efs:/");
		PA_WaitForVBL();

	} else {
		// Guarda los archivos en SRAM
		// PA_SaveData(1, &savegame, sizeof(savegame));
	}

	PA_WaitForVBL();			// Espera al sincronismo

}



// Funcion PlayFatSfx();	// Reproduce un archivo RAW desde la FAT
void PlayFatSfx(const char* nombre) {

	free(buffer_sfx);		// Vacia el buffer
	buffer_sfx = NULL;

	u32 medida_sfx = 0;	// Tamaño del archivo a cargar
	u32 buffer_sfx_size = 0;

	FILE* file_sfx;		// Nombres de archivo

	char filename[256];		// Nombre del archivo

	// Archivo Voice
	sprintf(filename, "/sfx/%s.raw", nombre);	// Abre el RAW
	file_sfx = fopen(filename, "rb");
	if (file_sfx) {		// Si el archivo "RAW" existe...
		// Obten el tamaño del archivo
		fseek(file_sfx, 0, SEEK_END);
		medida_sfx = ftell(file_sfx);
		rewind(file_sfx);
		// Reserva el espacio en RAM
		buffer_sfx = (char*) calloc (medida_sfx, sizeof(char));
		if (buffer_sfx == NULL) {
			Error(2, NULL);
		}
		// Carga el archivo en RAM
		fread (buffer_sfx, 1, medida_sfx, file_sfx);
	} else {			// Si no existe...
		Error(1, nombre);
	}
	fclose(file_sfx);		// Cierra el archivo
	PA_WaitForVBL();		// Espera al cierre de los archivos

	buffer_sfx_size = medida_sfx;	// Opten el tamaño del archivo
	AS_SoundQuickPlay(buffer_sfx);	// Reproduce el archivo

}



// Funcion LoadNewStage();		Carga un nuevo decorado y musica
void LoadNewStage(u8 newstage) {

	switch (newstage) {		// Segun el nivel pasado
		case 4:
			LoadRamBg(1, 3, "bg02_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg02_down");
			AS_MP3StreamPlay("/bgm/bgm02.mp3");	// Y la musica
			break;

		case 8:
			LoadRamBg(1, 3, "bg03_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg03_down");
			AS_MP3StreamPlay("/bgm/bgm03.mp3");	// Y la musica
			break;

		case 12:
			LoadRamBg(1, 3, "bg04_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg04_down");
			AS_MP3StreamPlay("/bgm/bgm04.mp3");	// Y la musica
			break;

		case 16:
			LoadRamBg(1, 3, "bg05_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg05_down");
			AS_MP3StreamPlay("/bgm/bgm05.mp3");	// Y la musica
			break;

		case 20:
			LoadRamBg(1, 3, "bg06_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg06_down");
			AS_MP3StreamPlay("/bgm/bgm06.mp3");	// Y la musica
			break;

		case 24:
			LoadRamBg(1, 3, "bg07_up");				// Carga los nuevos fondos
			LoadRamBg(0, 3, "bg07_down");
			AS_MP3StreamPlay("/bgm/bgm07.mp3");	// Y la musica
			break;

		default:
			Error(3, "LoadNewStage");
			break;
	}

	// Loop del Mp3
	PA_WaitForVBL();
	AS_SetMP3Loop(true);

	// Espera un ciclo (Necesario para el cambio de MP3)
	PA_WaitForVBL();

}



// Funcion CheckRecord();		// Verifica si has logrado algun record
u8 CheckRecord(void) {

	u8 test = 0;		// Variable de uso de control

	// Si has logrado completado el juego...
	if (level == 25) {
		if (
			(score > savegame[0].score)		// Si has logrado la maxima puntuacion
			||
			(maxcombo > savegame[1].combo)	// o el maximo combo
			||
			(maxcomboscore > savegame[2].comboscore) // o el maximo comboscore
			) {
				test = 1;
		}

	}

	// Si tienes record de score
	if (score > savegame[7].score) {
		test = 1;
	}

	// Si tienes record de combo
	if (maxcombo > savegame[12].combo) {
		test = 1;
	}

	// Si tienes record de comboscore
	if (maxcomboscore > savegame[17].comboscore) {
		test = 1;
	}

	return test;	// Devuelve el resultado

}



// Funcion ShowCombo();		// Muestra el combo
void ShowCombo(u8 numero) {

	// Variables
	u8 centenas;
	u8 decenas;
	u8 unidades;

	u8 n;

	// Muestra el texto
	PA_SetSpriteGfx(0, spr_combo, gfx_combo[1]);

	// Si el combo es de 1 cifra
	if (numero < 10) {
		PA_SetSpriteX(0, (spr_combo + 1), 64);
		PA_SetSpriteGfx(0, (spr_combo + 1), gfx_numero[0][10]);
		PA_SetSpriteX(0, (spr_combo + 2), 80);
		PA_SetSpriteGfx(0, (spr_combo + 2), gfx_numero[0][11]);
		PA_SetSpriteX(0, (spr_combo + 3), 96);
		PA_SetSpriteGfx(0, (spr_combo + 3), gfx_numero[0][numero]);
		PA_SetSpriteX(0, (spr_combo + 4), 112);
		PA_SetSpriteGfx(0, (spr_combo + 4), gfx_numero[0][10]);
	}

	// Si el numero es de 2 cifras
	if (
		(numero > 9)
		&&
		(numero < 100)
		) {
			decenas = (numero / 10);
			unidades = numero - (decenas * 10);
			PA_SetSpriteX(0, (spr_combo + 1), 56);
			PA_SetSpriteGfx(0, (spr_combo + 1), gfx_numero[0][10]);
			PA_SetSpriteX(0, (spr_combo + 2), 72);
			PA_SetSpriteGfx(0, (spr_combo + 2), gfx_numero[0][11]);
			PA_SetSpriteX(0, (spr_combo + 3), 88);
			PA_SetSpriteGfx(0, (spr_combo + 3), gfx_numero[0][decenas]);
			PA_SetSpriteX(0, (spr_combo + 4), 104);
			PA_SetSpriteGfx(0, (spr_combo + 4), gfx_numero[0][unidades]);
	}

	// Si el numero es de 3 cifras
	if (
		(numero > 99)
		) {
			centenas = (numero / 100);
			n = numero - (centenas * 100);
			decenas = (n / 10);
			unidades = n - (decenas * 10);
			PA_SetSpriteX(0, (spr_combo + 1), 64);
			PA_SetSpriteGfx(0, (spr_combo + 1), gfx_numero[0][11]);
			PA_SetSpriteX(0, (spr_combo + 2), 80);
			PA_SetSpriteGfx(0, (spr_combo + 2), gfx_numero[0][centenas]);
			PA_SetSpriteX(0, (spr_combo + 3), 96);
			PA_SetSpriteGfx(0, (spr_combo + 3), gfx_numero[0][decenas]);
			PA_SetSpriteX(0, (spr_combo + 4), 112);
			PA_SetSpriteGfx(0, (spr_combo + 4), gfx_numero[0][unidades]);
	}

}



// Funcion ShowComboScore();	// Muestra el Combo Score
void ShowComboScore(u32 numero) {

	// Variables
	u32 unidades = 0;
	u32 decenas = 0;
	u32 centenas = 0;
	u32 millares = 0;
	u32 decenasmillar = 0;
	u32 centenasmillar = 0;
	u32 millones = 0;

	u32 valor;

	// Calcula el valor
	millones = (numero / 1000000);			// Calcula millones
	valor = numero - (millones * 1000000);
	centenasmillar = (valor / 100000);		// Calcula centenas de millar
	valor -= (centenasmillar * 100000);
	decenasmillar = (valor / 10000);		// Calcula decenas de millar
	valor -= (decenasmillar * 10000);
	millares = (valor / 1000);				// Calculas millares
	valor -= (millares * 1000);
	centenas = (valor / 100);				// Calcula centenas
	valor -= (centenas * 100);
	decenas = (valor / 10);					// Calcula las decenas
	unidades = valor - (decenas * 10);		// Calcula las unidades

	HideComboScore();		// Resetea el marcador

	// Muestra el texto
	PA_SetSpriteGfx(1, spr_comboscore, gfx_comboscore[1]);

	// Muestreo
	if (numero < 10) {		// 1 digito
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][unidades]);
	}
	if ((numero > 9) && (numero < 100)) {	// 2 digitos
		PA_SetSpriteX(1, (spr_comboscore + 3), 80);
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][decenas]);
		PA_SetSpriteX(1, (spr_comboscore + 4), 96);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][unidades]);
	}
	if ((numero > 99) && (numero < 1000)) {		// 3 digitos
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][centenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][decenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 5), gfx_numero[1][unidades]);
	}
	if ((numero > 999) && (numero < 10000)) {		// 4 digitos
		PA_SetSpriteX(1, (spr_comboscore + 2), 64);
		PA_SetSpriteGfx(1, (spr_comboscore + 2), gfx_numero[1][millares]);
		PA_SetSpriteX(1, (spr_comboscore + 3), 80);
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][centenas]);
		PA_SetSpriteX(1, (spr_comboscore + 4), 96);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][decenas]);
		PA_SetSpriteX(1, (spr_comboscore + 5), 112);
		PA_SetSpriteGfx(1, (spr_comboscore + 5), gfx_numero[1][unidades]);
	}
	if ((numero > 9999) && (numero < 100000)) {		// 5 digitos
		PA_SetSpriteGfx(1, (spr_comboscore + 2), gfx_numero[1][decenasmillar]);
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][millares]);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][centenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 5), gfx_numero[1][decenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 6), gfx_numero[1][unidades]);
	}
	if ((numero > 99999) && (numero < 1000000)) {		// 6 digitos
		PA_SetSpriteX(1, (spr_comboscore + 1), 48);
		PA_SetSpriteGfx(1, (spr_comboscore + 1), gfx_numero[1][centenasmillar]);
		PA_SetSpriteX(1, (spr_comboscore + 2), 64);
		PA_SetSpriteGfx(1, (spr_comboscore + 2), gfx_numero[1][decenasmillar]);
		PA_SetSpriteX(1, (spr_comboscore + 3), 80);
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][millares]);
		PA_SetSpriteX(1, (spr_comboscore + 4), 96);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][centenas]);
		PA_SetSpriteX(1, (spr_comboscore + 5), 112);
		PA_SetSpriteGfx(1, (spr_comboscore + 5), gfx_numero[1][decenas]);
		PA_SetSpriteX(1, (spr_comboscore + 6), 128);
		PA_SetSpriteGfx(1, (spr_comboscore + 6), gfx_numero[1][unidades]);
	}
	if (numero > 999999) {		// 7 digitos
		PA_SetSpriteGfx(1, (spr_comboscore + 1), gfx_numero[1][millones]);
		PA_SetSpriteGfx(1, (spr_comboscore + 2), gfx_numero[1][centenasmillar]);
		PA_SetSpriteGfx(1, (spr_comboscore + 3), gfx_numero[1][decenasmillar]);
		PA_SetSpriteGfx(1, (spr_comboscore + 4), gfx_numero[1][millares]);
		PA_SetSpriteGfx(1, (spr_comboscore + 5), gfx_numero[1][centenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 6), gfx_numero[1][decenas]);
		PA_SetSpriteGfx(1, (spr_comboscore + 7), gfx_numero[1][unidades]);
	}

}



// Funcion HideCombo();		// Oculta el marcador de combo
void HideCombo(void) {
	PA_SetSpriteGfx(0, spr_combo, gfx_combo[0]);
	PA_SetSpriteX(0, (spr_combo + 1), 64);
	PA_SetSpriteGfx(0, (spr_combo + 1), gfx_numero[0][10]);
	PA_SetSpriteX(0, (spr_combo + 2), 80);
	PA_SetSpriteGfx(0, (spr_combo + 2), gfx_numero[0][10]);
	PA_SetSpriteX(0, (spr_combo + 3), 96);
	PA_SetSpriteGfx(0, (spr_combo + 3), gfx_numero[0][10]);
	PA_SetSpriteX(0, (spr_combo + 4), 112);
	PA_SetSpriteGfx(0, (spr_combo + 4), gfx_numero[0][10]);
}



// Funcion HideComboScore();		// Oculta el marcador de combo score
void HideComboScore(void) {
	u8 s = 0;
	u8 sx = 40;
	PA_SetSpriteGfx(1, spr_comboscore, gfx_comboscore[0]);
	for (s = 1; s < 8; s ++) {
		PA_SetSpriteX(1, (spr_comboscore + s), sx);
		PA_SetSpriteGfx(1, (spr_comboscore + s), gfx_numero[1][10]);
		sx += 16;
	}
}



// Funcion ShowCredits();		// Muestra los creditos
void ShowCredits(u8 endgame) {

	// Variables
	float y[2];
	s16 yr[2];
	u8 stage[2];
	u8 n;
	char archivo[256];
	u8 repeat = 0;
	u8 in = 0;
	u8 out = 0;
	float contador = 0;

	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);		// Brillo a 0
	PA_SetBrightness(1, -16);

	PA_WaitForVBL();	// Espera un ciclo


	// Carga los fondos
	LoadRamBg(0, 3, "credits_text_1");	// Pantalla inferior
	LoadRamBg(0, 2, "credits_bgd");
	LoadRamBg(1, 3, "credits_text_1");	// Pantalla superior
	LoadRamBg(1, 2, "credits_bgu");

	// Inicializa las posiciones
	y[0] = 64;
	y[1] = -192;
	stage[0] = 1;
	stage[1] = 1;

	// Inicializa variables
	in = 1;
	out = 0;
	contador = -16;
	repeat = 1;

	while (repeat) {

		for (n = 0; n < 2; n ++) {	// Mueve ambas pantallas
			y[n] += 0.3;
			if (y[n] > 255) {		// Si debes cambiar de fondo...
				y[n] -= 255;
				stage[n] ++;
				switch (stage[n]) {		// Siguiente fondo a cargar
					case 2:
						sprintf(archivo, "%s", "credits_text_2");
						break;
					case 3:
						sprintf(archivo, "%s", "credits_text_3");
						break;
					case 4:
						sprintf(archivo, "%s", "credits_text_4");
						break;
					case 5:
						sprintf(archivo, "%s", "credits_text_5");
						break;
					default:
						sprintf(archivo, "%s", "credits_text_1");
						break;
				}
				LoadRamBg(n, 3, archivo);	// Cargalo
			}
			if (stage[n] > 5) {		// Si ya has mostrado todos los fondos
				y[n] = 0;			// No te muevas
			}
			yr[n] = int(y[n]);		// Pasa la variable real a la relativa
			if (yr[n] < 0) {		// Si es menor de 0, es 0
				yr[n] = 0;
			}
			PA_BGScrollY(n, 3, yr[n]);	// Mueve el fondo
		}

		if (stage[1] > 5) {			// Si la pantalla superior ha mostrado todos los fondos
			out = 1;				// Fade out y sal
		}

		if (Pad.Newpress.B && !out) {		// Si pulsas B
			out = 1;	// Fade out y sal
		}

		if (in && !out) {			// Efecto Fade In
			contador += .5;
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (contador == 0) {
				in = 0;
			}
		}

		if (out) {			// Efecto Fade Out
			contador -= .5;
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (endgame == 1) {
				AS_SetMP3Volume(int(((16 + contador) * 127) / 16));
			}
			if (contador == -16) {
				repeat = 0;
			}
		}

		PA_WaitForVBL();			// Espera al refresco

	}

}



// Funcion Game Over		// Funcion para el game over
void GameOver(void) {

	// Variables

	u8 s;					// Sprite
	u8 x;					// Coordenada X
	u8 y;					// Coordenada Y
	u8 f;					// Frame animacion
	u8 n;					// Contador de retardo
	u8 contador;
	u16 gfx_gameover[2];	// Graficos de los textos [pantalla]

	for (contador = 60; contador > 0; contador --) {	// Baja el volumen
		AS_SetMP3Volume(int(((contador * 127) / 60)));
		PA_WaitForVBL();
	}

	AS_MP3Stop();		// Deten el MP3
	PA_WaitForVBL();

	// Efecto gris

	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (f != 0) {
				PA_SetSpriteGfx(1, s, gfx_ficha[1][12]);
			}
			s ++;
		}
		AS_SoundQuickPlay(clash);		// Sonido al borrar las piezas
		for (n = 0; n < 10; n ++) {		// Espera 10 frames
			PA_WaitForVBL();
		}
	}

	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			f = tablero[x][y];
			if (f != 0) {
				PA_SetSpriteGfx(0, s, gfx_ficha[0][12]);
			}
			s ++;
		}
		AS_SoundQuickPlay(clash);		// Sonido al borrar las piezas
		for (n = 0; n < 10; n ++) {		// Espera 10 frames
			PA_WaitForVBL();
		}
	}

	for (n = 0; n < 30; n ++) {			// Espera 30 frames
		PA_WaitForVBL();
	}

	// Texto Game Over
	gfx_gameover[0] = PA_CreateGfx(0, (void*)gameover0_Sprite, OBJ_SIZE_64X64, 1);
	gfx_gameover[1] = PA_CreateGfx(1, (void*)gameover1_Sprite, OBJ_SIZE_64X64, 1);
	PA_CreateSpriteFromGfx(0, 1, gfx_gameover[0], OBJ_SIZE_64X64, 1, pal_combo, 64, 52);
	PA_CreateSpriteFromGfx(1, 1, gfx_gameover[1], OBJ_SIZE_64X64, 1, pal_combo, 64, 80);
	PA_WaitForVBL();
	PlayFatSfx("gameover");		// Sonido de game over
	PA_WaitForVBL();

}



// Funcion GameOptions();		// Opciones del juego
void GameOptions(void) {

	// Variables
	u8 n = 0;
	u8 item = 0;
	u32 chk = 0;
	float contador = 0;
	u8 in = 0;
	u8 out = 0;
	u8 repeat = 0;

	u16 gfx_boton[8];			// Graficos de los botones [sprite]
	u8 paleta_flag = 1;			// Paletas
	u8 paleta_combo = 2;
	u8 paleta_score = 3;
	u8 paleta_ok = 4;
	s8 sx[3];					// Coordenadas de los botones
	s8 sy[3]; 

	// Resetea el modo grafico
	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);	// Brillo a 0
	PA_SetBrightness(1, -16);

	PA_WaitForVBL();	// Espera un ciclo

	// Carga los Sprites a usar
	// Graficos
	gfx_boton[0] = PA_CreateGfx(0, (void*)options_flag_Sprite, OBJ_SIZE_32X32, 1);
	gfx_boton[1] = PA_CreateGfx(0, (void*)options_combo_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[2] = PA_CreateGfx(0, (void*)options_combo_r_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[3] = PA_CreateGfx(0, (void*)options_flag_Sprite, OBJ_SIZE_32X32, 1);
	gfx_boton[4] = PA_CreateGfx(0, (void*)options_score_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[5] = PA_CreateGfx(0, (void*)options_score_r_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[6] = PA_CreateGfx(0, (void*)options_ok_l_Sprite, OBJ_SIZE_64X32, 1);
	gfx_boton[7] = PA_CreateGfx(0, (void*)options_ok_r_Sprite, OBJ_SIZE_64X32, 1);
	// Paletas
	PA_LoadSpritePal(0, paleta_flag, (void*)options_flag_Pal);
	PA_LoadSpritePal(0, paleta_combo, (void*)options_combo_Pal);
	PA_LoadSpritePal(0, paleta_score, (void*)options_score_Pal);
	PA_LoadSpritePal(0, paleta_ok, (void*)options_ok_Pal);
	// Coordenadas iniciales de los sprites
	sx[0] = 40;
	sy[0] = 24;
	sx[1] = 40;
	sy[1] = 72;
	sx[2] = 64;
	sy[2] = 136;
	// Crea los Sprites
	PA_CreateSpriteFromGfx(0, 1, gfx_boton[0], OBJ_SIZE_32X32, 1, paleta_flag, sx[0], sy[0]);
	PA_CreateSpriteFromGfx(0, 11, gfx_boton[1], OBJ_SIZE_64X32, 1, paleta_combo, (sx[0] + 48), sy[0]);
	PA_CreateSpriteFromGfx(0, 12, gfx_boton[2], OBJ_SIZE_64X32, 1, paleta_combo, (sx[0] + 112), sy[0]);
	PA_CreateSpriteFromGfx(0, 2, gfx_boton[3], OBJ_SIZE_32X32, 1, paleta_flag, sx[1], sy[1]);
	PA_CreateSpriteFromGfx(0, 13, gfx_boton[4], OBJ_SIZE_64X32, 1, paleta_score, (sx[1] + 48), sy[1]);
	PA_CreateSpriteFromGfx(0, 14, gfx_boton[5], OBJ_SIZE_64X32, 1, paleta_score, (sx[1] + 112), sy[1]);
	PA_CreateSpriteFromGfx(0, 15, gfx_boton[6], OBJ_SIZE_64X32, 1, paleta_ok, sx[2], sy[2]);
	PA_CreateSpriteFromGfx(0, 16, gfx_boton[7], OBJ_SIZE_64X32, 1, paleta_ok, (sx[2] + 64), sy[2]);
	// Y establece prioridad sobre el layer 0 y el frame de animacion
	for (n = 1; n < 3; n ++){
		PA_SetSpritePrio(0, n, 0);
		PA_SetSpriteAnimEx(0, n, 32, 32, 1, 0);
	}
	for (n = 11; n < 17; n ++){
		PA_SetSpritePrio(0, n, 0);
		PA_SetSpriteAnimEx(0, n, 64, 32, 1, 0);
	}

	// Carga los fondos a usar
	LoadRamBg(1, 3, "titleup");
	LoadRamBg(0, 3, "titledown");
	PA_WaitForVBL();	// Espera un ciclo

	in = 1;				// Inicializa variables
	out = 0;
	contador = -16;
	repeat = 1;
	item = 0;

	while (repeat) {

		// Gestion de Botones
		for (n = 1; n < 3; n ++){
			PA_SetSpriteAnimEx(0, n, 32, 32, 1, 0);
		}
		for (n = 11; n < 17; n ++){
			PA_SetSpriteAnimEx(0, n, 64, 32, 1, 0);
		}

		if (!out) {		// Si no estas en Fade Out ...

			// Lee el pad
			if ((Pad.Newpress.Up)&&(item > 0)) {	// Si pulsas arriba
				AS_SoundQuickPlay(menumove);		// Sonido
				item --;
			}
			if ((Pad.Newpress.Down)&&(item < 2)) {	// Si pulsas abajo
				AS_SoundQuickPlay(menumove);		// Sonido
				item ++;
			}

			// Lee el Stylus (Pulsaciones)
			if (PA_SpriteTouched(11) || PA_SpriteTouched(12)) {	// Boton 1
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					savegame[0].options = (!savegame[0].options);
					AS_SoundQuickPlay(click);
				}
				item = 0;
			}
			if (PA_SpriteTouched(13) || PA_SpriteTouched(14)) {	// Boton 2
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					savegame[1].options = (!savegame[1].options);
					AS_SoundQuickPlay(click);
				}
				item = 1;
			}
			if (PA_SpriteTouched(15) || PA_SpriteTouched(16)) {	// Boton 3
				if (Stylus.Newpress) {		// Si lo presionas, acepta
					out = 1;
					PlayFatSfx("moveon");	// Sonido aceptacion
				}
				item = 2;
			}

		}


		switch (item) {		// Opciones
			case 0:
				PA_SetSpriteAnimEx(0, 11, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 12, 64, 32, 1, 1);
				break;

			case 1:
				PA_SetSpriteAnimEx(0, 13, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 14, 64, 32, 1, 1);
				break;

			case 2:
				PA_SetSpriteAnimEx(0, 15, 64, 32, 1, 1);
				PA_SetSpriteAnimEx(0, 16, 64, 32, 1, 1);
				break;
		}

		if (Pad.Newpress.A && !out) {		// Si presionas A (y no estas en fade out)
			if (item < 2) {					// y es una opcion valida
				savegame[item].options = (!savegame[item].options);
				AS_SoundQuickPlay(click);
			} else {
				out = 1;				// si no sal
				PlayFatSfx("moveon");	// Sonido aceptacion
			}
		}

		if (savegame[0].options) {	// Si la opcion combo esta activada
			PA_SetSpriteAnimEx(0, 1, 32, 32, 1, 1);
		}
		if (savegame[1].options) {	// Si la opcion combo score esta activada
			PA_SetSpriteAnimEx(0, 2, 32, 32, 1, 1);
		}

		// Gestion de fondos

		if (in && !out) {	// Efecto Fade In
			contador += .5;
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (contador == 0) {
				in = 0;
			}
		}

		if (out) {			// Efecto Fade In
			contador -= .5;
			PA_SetBrightness(0, int(contador));
			PA_SetBrightness(1, int(contador));
			if (contador == -16) {
				repeat = 0;
			}
		}

		PA_WaitForVBL();	// Espera un ciclo

	}

	// Cheksum final
	chk = 0;
	for (n = 0; n < 18; n ++) {
		chk += (int(savegame[n].nombre) + savegame[n].score + savegame[n].level + savegame[n].combo + savegame[n].comboscore + savegame[n].complete + savegame[n].options);
	}
	for (n = 0; n < 18; n ++) {
		savegame[n].checksum = chk;
	}

	AS_MP3Pause();				// Pausa la musica
	PA_WaitForVBL();			// Espera al sincronismo

	FILE* savefile;				// Buffer para el archivo de guardado

	if (SLOT == 1) {	// Si es SLOT 1

		// Guarda los datos en un archivo en FAT
		chdir("fat:/");
		PA_WaitForVBL();
		savefile = fopen(savegamefile, "wb");
		PA_WaitForVBL();
		fwrite(&savegame, sizeof(savegame), 1, savefile);
		PA_WaitForVBL();
		fclose(savefile);
		PA_WaitForVBL();
		chdir("efs:/");
		PA_WaitForVBL();

	} else {
		// Guarda los archivos en SRAM
		// PA_SaveData(1, &savegame, sizeof(savegame));
	}

	AS_MP3Unpause();			// Quita la pausa a la musica
	PA_WaitForVBL();			// Espera al sincronismo

}



// Funcion GameAborted();		// Animacion al abortar el juego
void GameAborted(void) {

	// Variables
	u8 x;
	u8 y;
	u8 contador;
	u8 s;
	u16 gfx_aborted[2];

	for (contador = 60; contador > 0; contador --) {	// Baja el volumen
		AS_SetMP3Volume(int(((contador * 127) / 60)));
		PA_WaitForVBL();
	}

	AS_MP3Stop();		// Deten el MP3
	PA_WaitForVBL();

	// Efecto gris
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 2; y < 12; y ++) {		// Pantalla Superior (1)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			PA_SetSpriteGfx(1, s, gfx_ficha[1][12]);
			s ++;
		}
		PA_WaitForVBL();
	}
	s = spr_ficha;					// Resetea el numero de sprite
	for (y = 12; y < 22; y ++) {	// Pantalla Inferior (0)
		for (x = 0; x < 8; x ++) {
			// Actualiza el frame de animacion
			PA_SetSpriteGfx(0, s, gfx_ficha[0][12]);
			s ++;
		}
		PA_WaitForVBL();
	}

	// Texto Game Over
	gfx_aborted[0] = PA_CreateGfx(0, (void*)aborted0_Sprite, OBJ_SIZE_64X64, 1);
	gfx_aborted[1] = PA_CreateGfx(1, (void*)aborted1_Sprite, OBJ_SIZE_64X64, 1);
	PA_CreateSpriteFromGfx(0, 1, gfx_aborted[0], OBJ_SIZE_64X64, 1, pal_combo, 64, 52);
	PA_CreateSpriteFromGfx(1, 1, gfx_aborted[1], OBJ_SIZE_64X64, 1, pal_combo, 64, 80);
	PA_WaitForVBL();
	PlayFatSfx("gameover");		// Sonido de game complete
	PA_WaitForVBL();

}



// Funcion GameComplete();		// Animacion al terminar el juego
void GameComplete(void) {

	// Variables
	u8 x;
	u8 y;
	u8 contador;
	u16 gfx_complete[2];

	for (contador = 60; contador > 0; contador --) {	// Baja el volumen
		AS_SetMP3Volume(int(((contador * 127) / 60)));
		PA_WaitForVBL();
	}

	AS_MP3Stop();		// Deten el MP3
	PA_WaitForVBL();

	if (special == 1) {
		ficha[0] -= 5;	// Vuelve las fichas en normales
		ficha[1] -= 5;
		ficha[2] -= 5;
		tablero[fx][fy] = ficha[0];
		tablero[fx][fy + 1] = ficha[1];
		tablero[fx][fy + 2] = ficha[2];
	}

	CleanArray();		// Vacia el array de borrado

	for (y = 2; y < 22; y ++) {		// Marca las piezas a borrar
		for (x = 0; x < 8 ; x ++) {
			if (tablero[x][y] != 0) {
				borrado[x][y] = 1;
			}
		}
	}

	DestroyPieces();	// Y destruyelas

	for (y = 2; y < 22; y ++) {		// Marca las piezas borradas
		for (x = 0; x < 8 ; x ++) {
			tablero[x][y] = 0;
		}
	}

	RefreshSprites();		// Y refresca la parrilla

	// Texto Game Over
	gfx_complete[0] = PA_CreateGfx(0, (void*)complete0_Sprite, OBJ_SIZE_64X64, 1);
	gfx_complete[1] = PA_CreateGfx(1, (void*)complete1_Sprite, OBJ_SIZE_64X64, 1);
	PA_CreateSpriteFromGfx(0, 1, gfx_complete[0], OBJ_SIZE_64X64, 1, pal_combo, 64, 52);
	PA_CreateSpriteFromGfx(1, 1, gfx_complete[1], OBJ_SIZE_64X64, 1, pal_combo, 64, 80);
	PA_WaitForVBL();
	PlayFatSfx("gameover");		// Sonido de game complete
	PA_WaitForVBL();

}



// LoadGameData()		Carga todos los fondos en RAM
void LoadGameData(void){

	PA_SetVideoMode(0, 0);		// Inicializa pantalla
	PA_SetVideoMode(1, 0);
	PA_ResetBgSysScreen(0);		// Reinicia el sistema de pantalla
	PA_ResetBgSysScreen(1);
	PA_ResetSpriteSys();		// Reinicia el sistema de Sprites
	PA_DisableSpecialFx(0);		// Deshabilita los efectos especiales
	PA_DisableSpecialFx(1);
	PA_SetBrightness(0, -16);
	PA_SetBrightness(1, -16);

	PA_InitCustomText(0, 0, fuente_system);		// Inicializa el texto en ambas pantallas
	PA_InitCustomText(1, 0, fuente_system);		// En la capa 0

	PA_OutputText(1, 1, 1, "CARGANDO ARCHVOS... 000 %");		// Mensage de error en Castellano
	PA_OutputText(0, 1, 1, "LOADING FILES... 000 %");			// Mensage de error en ingles
	FadeIn();

	LoadScreenAnim(1, BG_SLOTS);
	LoadFatBg("palib", "splash/palib");
	LoadScreenAnim(2, BG_SLOTS);
	LoadFatBg("devkitpro", "splash/devkitpro");
	LoadScreenAnim(3, BG_SLOTS);
	LoadFatBg("nfoxup", "splash/nfoxup");
	LoadScreenAnim(4, BG_SLOTS);
	LoadFatBg("nfoxdown", "splash/nfoxdown");
	LoadScreenAnim(5, BG_SLOTS);
	LoadFatBg("titleup", "menu/titleup");
	LoadScreenAnim(6, BG_SLOTS);
	LoadFatBg("titledown", "menu/titledown");
	LoadScreenAnim(7, BG_SLOTS);
	LoadFatBg("showrecordsp1_up", "records/showrecordsp1_up");
	LoadScreenAnim(8, BG_SLOTS);
	LoadFatBg("showrecordsp1_down", "records/showrecordsp1_down");
	LoadScreenAnim(9, BG_SLOTS);
	LoadFatBg("showrecordsp2_up", "records/showrecordsp2_up");
	LoadScreenAnim(10, BG_SLOTS);
	LoadFatBg("showrecordsp2_down", "records/showrecordsp2_down");
	LoadScreenAnim(11, BG_SLOTS);
	LoadFatBg("entername_down", "records/entername_down");
	LoadScreenAnim(12, BG_SLOTS);
	LoadFatBg("entername_up", "records/entername_up");
	LoadScreenAnim(13, BG_SLOTS);
	LoadFatBg("bg02_up", "st04/bg02_up");
	LoadScreenAnim(14, BG_SLOTS);
	LoadFatBg("bg02_down", "st04/bg02_down");
	LoadScreenAnim(15, BG_SLOTS);
	LoadFatBg("bg03_up", "st08/bg03_up");
	LoadScreenAnim(16, BG_SLOTS);
	LoadFatBg("bg03_down", "st08/bg03_down");
	LoadScreenAnim(17, BG_SLOTS);
	LoadFatBg("bg04_up", "st12/bg04_up");
	LoadScreenAnim(18, BG_SLOTS);
	LoadFatBg("bg04_down", "st12/bg04_down");
	LoadScreenAnim(19, BG_SLOTS);
	LoadFatBg("bg05_up", "st16/bg05_up");
	LoadScreenAnim(20, BG_SLOTS);
	LoadFatBg("bg05_down", "st16/bg05_down");
	LoadScreenAnim(21, BG_SLOTS);
	LoadFatBg("bg06_up", "st20/bg06_up");
	LoadScreenAnim(22, BG_SLOTS);
	LoadFatBg("bg06_down", "st20/bg06_down");
	LoadScreenAnim(23, BG_SLOTS);
	LoadFatBg("bg07_up", "st24/bg07_up");
	LoadScreenAnim(24, BG_SLOTS);
	LoadFatBg("bg07_down", "st24/bg07_down");
	LoadScreenAnim(25, BG_SLOTS);
	LoadFatBg("credits_text_1", "credits/credits_text_1");
	LoadScreenAnim(26, BG_SLOTS);
	LoadFatBg("credits_text_2", "credits/credits_text_2");
	LoadScreenAnim(27, BG_SLOTS);
	LoadFatBg("credits_text_3", "credits/credits_text_3");
	LoadScreenAnim(28, BG_SLOTS);
	LoadFatBg("credits_text_4", "credits/credits_text_4");
	LoadScreenAnim(29, BG_SLOTS);
	LoadFatBg("credits_text_5", "credits/credits_text_5");
	LoadScreenAnim(30, BG_SLOTS);
	LoadFatBg("credits_bgd", "credits/credits_bgd");
	LoadScreenAnim(31, BG_SLOTS);
	LoadFatBg("credits_bgu", "credits/credits_bgu");
	LoadScreenAnim(32, BG_SLOTS);
	LoadFatBg("titleup", "menu/titleup");
	LoadScreenAnim(33, BG_SLOTS);
	LoadFatBg("titledown", "menu/titledown");
	LoadScreenAnim(34, BG_SLOTS);
	LoadFatBg("bg01_up", "st01/bg01_up");
	LoadScreenAnim(35, BG_SLOTS);
	LoadFatBg("bg01_down", "st01/bg01_down");
	LoadScreenAnim(36, BG_SLOTS);
	LoadFatBg("black", "bg/black");
	LoadScreenAnim(37, BG_SLOTS);
	LoadFatBg("bg00_up", "bg/bg00_up");
	LoadScreenAnim(38, BG_SLOTS);
	LoadFatBg("bg00_down", "bg/bg00_down");

	LoadScreenAnim(39, BG_SLOTS);
	LoadFatBg("neocompo_up", "splash/neocompo_up");
	LoadScreenAnim(40, BG_SLOTS);
	LoadFatBg("neocompo_down", "splash/neocompo_down");

	PA_OutputText(1, 1, 3, "CARGA COMPLETADA...");		// Mensage de error en Castellano
	PA_OutputText(0, 1, 3, "LOAD COMPLETE...");			// Mensage de error en ingles
	FadeOut();

}



// Funcion LoadScreenAnim(u8 actual, u8 total);		// Actualiza la animacion de carga
void LoadScreenAnim(u8 actual, u8 total) {

	u8 z;

	z = int((actual * 100) / total);	// Calcula el porcentaje cargado

	PA_OutputText(1, 1, 1, "CARGANDO ARCHVOS... %03d %", z);		// Mensage de error en Castellano
	PA_OutputText(0, 1, 1, "LOADING FILES... %03d %", z);			// Mensage de error en ingles

}






/***********************************************************************************
	Nucleo del juego
***********************************************************************************/

s8 MainGame(void) {

	// Variables locales
	u8 flag = 1;		// Flag del recheck de borrado
	s32 getscore;		// Obten el score de la jugada
	u8 checkdrop;		// Control de si debes de bajar
	u8 playgame = 1;	// Control del fin de partida

	u8 cambionivel = 0;		// Flag del cambio de decorado
	s8 contador = 0;		// Contador para los fades
	s8 newlevel = 0;		// Backup del nivel al que tienes que cambiar
	u8 stage = 1;			// Estadio del cambio de decorado
	u8 combotime = 62;		// Contador de muestreo del combo

	u8 n = 0;				// Variable de uso general

	// Habilita canal Alpha en la pantalla (BG3)
	PA_EnableSpecialFx(0, SFX_ALPHA, SFX_BG2, SFX_BG3 | SFX_BD);
	PA_EnableSpecialFx(1, SFX_ALPHA, SFX_BG2, SFX_BG3 | SFX_BD);

	// Carga los fondos iniciales
	LoadRamBg(1, 3, "bg01_up");
	LoadRamBg(0, 3, "bg01_down");
	LoadRamBg(1, 2, "black");
	LoadRamBg(0, 2, "black");
	LoadRamBg(1, 1, "bg00_up");
	LoadRamBg(0, 1, "bg00_down");

	// Ajusta el canal alpha
	PA_SetSFXAlpha(0, 16, 16);
	PA_SetSFXAlpha(1, 16, 16);

	// Carga musica inicial
	AS_MP3StreamPlay("/bgm/bgm01.mp3");
	AS_SetMP3Loop(true);
	AS_SetMP3Volume(127);

	// Crea la parrilla de sprites
	CreateSprites();
	RefreshSprites();

	// Dibuja los marcadores
	PA_SetTextCol(0, 31, 31, 31);
	PA_OutputText(0, 23, 4, "%07d", score);
	PA_OutputText(0, 25, 13, "%02d", level);
	PA_OutputText(1, 25, 18, "%03d", (nextlevel - currentlevel));

	// Efecto Fade In
	FadeIn();

	while (playgame == 1) {		// Juega hasta terminar la partida o gameover

		// Mueve la ficha y
		// Auto caida de la ficha
		checkdrop = 0;
		checkdrop = (MovePiece() + AutoDrop());

		// Si has caido hacia abajo
		if (checkdrop != 0) {			// Mira si hay game over
			if (fy < 2) {				// Si hay drop, pero no hay espacio
				playgame = 2;			// Devuelve "Game Over"
				checkdrop = 0;
			}
		}

		// Aborta la partida ?
		if (Pad.Newpress.Select) {		// Mira si se aborta la partida (Press Select)
			playgame = 3;				// Devuelve "Partida abortada"
			checkdrop = 0;
		}

		// Rutina de eliminacion de fichas colindantes (si debes)
		if (checkdrop != 0) {			// Comprueba si tienes que cambiar de ficha
			delflag = 0;				// Flag de borrado bajada
			if (special == 1) {			// Si eres la ficha cambiadora...
				TurnColor();			// Haz tu trabajo
			}	
			CleanArray();				// Vacia el array de borrado
			if (special == 2) {
				WhiteErase();			// Borra las fichas tocadas por el blanco
			} else {
				CheckRows();			// Mira las filas
				CheckColumns();			// Mira las columnas
				CheckToRight();			// Miras las diagonales (>)
				CheckToLeft();			// Miras las diagonales (<)
			}

			tempscore = 0;			// Contador temporal de fichas borradas
			if (delflag) {			// Si has de borrar alguna ficha...
				AS_SoundQuickPlay(boom);	// Sonido al borrar las piezas
				DestroyPieces();	// Anima las piezas a destruir...
				getscore = CleanPieces();
				if (getscore) {		// Si has borrado fichas
					flag = 1;			// Flag de repeticion
					tempscore += getscore;		// Calcula el total de fichas borradas
					while (flag) {		// Repite mientras queden fichas a eliminar
						flag = 0;				// Flag abajo
						RefreshSprites();		// Actualiza la parrilla de sprites
						if (savegame[0].options) {	// Muestra el combo ?
							ShowCombo(tempscore);
						}
						if (savegame[1].options) {	// Muestra el combo score?
							ShowComboScore(((tempscore * tempscore) * level));
						}
						combotime = 0;
						WaitForVsync();
						Delay();
						//
						DropDown();				// Baja las piezas si hace falta
						RefreshSprites();		// Actualiza la parrilla de sprites
						WaitForVsync();
						Delay();				// Espera
						CleanArray();			// Vacia el array de borrado
						delflag = 0;			// Flag de borrado abajo
						CheckRows();			// Mira las filas
						CheckColumns();			// Mira las columnas
						CheckToRight();			// Miras las diagonales (>)
						CheckToLeft();			// Miras las diagonales (<)
						if (delflag) {			// Si has de borrar mas fichas...
							AS_SoundQuickPlay(boom);	// Sonido al borrar las piezas
							DestroyPieces();	// Anima las piezas a destruir...
							getscore = CleanPieces();
							if (getscore) {			// Si has borrado fichas
								flag = 1;			// flag arriba
								tempscore += getscore;		// Calcula el total de fichas borradas
							}
						}
					}
				}
			}



			if (tempscore > maxcombo) {		// Si hay record de combo, guardalo
				maxcombo = tempscore;
			}
			if (((tempscore * tempscore) * level) > maxcomboscore) {
				maxcomboscore = ((tempscore * tempscore) * level);
			}

			score += ((tempscore * tempscore) * level);	// Suma puntos al score
			currentlevel += tempscore;

			if (currentlevel >= nextlevel) {	// Si has logrado el numero necesario de fichas
				currentlevel = 0;				// Resetea el contador
				nextlevel += 10;				// Pide 10 fichas mas para avanzar de nivel
				level += 1;						// Avanza de nivel
				cambionivel = 1;				// Flag de cambio de decorado arriba
				if (level == 25) {				// Si alcanzas el nivel 25, fin de partida
					playgame = 4;
				}
				nextdrop -= 2;					// Acelera la velocidad de caida
				if (nextdrop < 5) {			// Si has llegado al limite, mantenlo
					nextdrop = 5;
				}
			}

			PA_OutputText(0, 25, 13, "%02d", level);	// Actuliza marcadores
			PA_OutputText(1, 25, 18, "%03d", (nextlevel - currentlevel));
			PA_OutputText(0, 23, 4, "%07d", score);

			ChangePiece();		// Calcula la siguiente ficha

		}


		// Actualiza los graficos en pantalla
		RefreshSprites();		// Actualiza la parrilla de sprites

		// Debes de cambiar de decorado y musica ?
		if (cambionivel) {		// Si el Flag de cambio de decorado esta arriba

			switch (stage) {	// Estadios del cambio

				case 1:			// Filtro del cambio de decorados
					if (
						(level != 4)
						&&
						(level != 8)
						&&
						(level != 12)
						&&
						(level != 16)
						&&
						(level != 20)
						&&
						(level != 24)
						) {
							cambionivel = 0;	// Flag abajo
					} else {
						newlevel = level;	// Realiza un backup del nivel al que cambiaras
						stage ++;			// Avanza de estadio
					}
					break;

				case 2:		// Prepara el fondo para el fade out
					contador = 60;
					stage ++;
					break;

				case 3:		// Realiza el fade out de fondo y musica
					contador --;
					PA_SetSFXAlpha(0, 16, int(((contador * 16) / 60)));
					PA_SetSFXAlpha(1, 16, int(((contador * 16) / 60)));
					AS_SetMP3Volume(int(((contador * 127) / 60)));
					if (contador == 0) {
						stage ++;
					}
					break;

				case 4:		// Deten la musica y haz el cambio de decorados
					AS_MP3Stop();			// Deten el MP3
					for (n = 0; n < 5; n ++) {
						PA_WaitForVBL();		// Espera a que se detenga
					}
					LoadNewStage(newlevel);	// Solicita la carga del nuevo decorado
					stage ++;
					break;

				case 5:		// Realiza el fade in de fondo y musica
					contador ++;
					PA_SetSFXAlpha(0, 16, int(((contador * 16) / 60)));
					PA_SetSFXAlpha(1, 16, int(((contador * 16) / 60)));
					AS_SetMP3Volume(int(((contador * 127) / 60)));
					if (contador == 60) {
						stage ++;
					}
					break;

				case 6:		// Sal de la rutina
					PA_SetSFXAlpha(0, 16, 16);		// Restaura brillo
					PA_SetSFXAlpha(1, 16, 16);
					AS_SetMP3Volume(127);			// Restaura el volumen
					cambionivel = 0;				// Deshabilita la rutina
					stage = 1;						// Y restaura el contador
					break;
			}

		}


		// Oculta el marcador de combos si es necesario
		if (combotime < 62) {
			combotime ++;
			if (combotime == 60) {
				HideCombo();
				HideComboScore();
			}
		}

		// Debug
		/*
		if (Pad.Newpress.Y) {
			level = 24;
		}
		*/
		

		// Espera al refresco vertical
		WaitForVsync();			// Espera al sincronismo vertical (60fps)

	}

	HideCombo();	// Borra el mensage al salir
	HideComboScore();


	// Decide que hacer al salir del bucle principal
	switch (playgame) {

		case 2:	// Game Over
			GameOver();
			while (!Pad.Newpress.A) {		// Espera que se pulse "A"
				PA_WaitForVBL();
			}
			FadeOut();
			break;

		case 3:	// Abort
			GameAborted();
			while (!Pad.Newpress.A) {		// Espera que se pulse "A"
				PA_WaitForVBL();
			}
			FadeOut();
			break;

		case 4:	// Game Completed
			GameComplete();
			while (!Pad.Newpress.A) {		// Espera que se pulse "A"
				PA_WaitForVBL();
			}
			FadeOut();			// Fade out
			AS_MP3StreamPlay("/bgm/bgm08.mp3");		// Musica final de juego
			AS_SetMP3Loop(true);
			AS_SetMP3Volume(127);
			ShowCredits(1);		// Muestra los creditos
			AS_MP3Stop();		// y deten el MP3
			PA_WaitForVBL();
			break;
	}


	// Hay que grabar el record ?
	if (CheckRecord()) {			// Si hay record
		EnterYourName(playgame);	// Graba el nombre en la tabla
	}

	return playgame;		// Devuelve el estado de la partida


}

