// Debut global_value.h
#include <stdint.h>
#include <efi.h>
#include <efilib.h>
#ifndef GLOBAL_VALUE_H
#define GLOBAL_VALUE_H

typedef enum {
	SCANCODE_SET_UNKNOWN = 0,
	SCANCODE_SET_1,
	SCANCODE_SET_2,
	SCANCODE_SET_3
} scancode_set_t;

struct KEYBOARD_CONFIG {
	scancode_set_t set;
	uint8_t translation_enabled;
	unsigned char actuel_position[3][128];
};

struct KNOPF
{
	char *name;
	uint8_t value;
};

struct KNOPF_VARIABLE
{
	struct KNOPF pong;
	struct KNOPF shell;
	struct KNOPF shutdown;
};

struct KEY
{
	uint8_t value;
	uint8_t SCC_press;
	uint8_t SCC_release;
	uint8_t need_ethendu;
};

struct KEY_POSITION {
	const unsigned char qwertz_german[3][128];
	const unsigned char qwertz_german_swiss[3][128];
	const unsigned char qwertz_french_swiss[3][128];
};

struct KEY_VARIABLE
{
	struct KEYBOARD_CONFIG conf;
	struct KEY_POSITION pos;
	struct KEY ENTER;
	struct KEY TAB;
	struct KEY CAPS_LOCK;
	struct KEY DELET;
	struct KEY MAJ_L;
	struct KEY MAJ_R;
	struct KEY CTRL_L;
	struct KEY CTRL_R;
	struct KEY ALT_L;
	struct KEY ALT_R;
	struct KEY FLECHE_L;
	struct KEY FLECHE_R;
	struct KEY FLECHE_T;
	struct KEY FLECHE_B;
};

struct ETHENDU
{
	uint8_t value;
	uint8_t SCC_prefix;
};

struct GPU_CONFIG {
	UINT64 FrameBuffer;
	UINT64 FrameBuffer_Size;
	UINT32 width;
	UINT32 height;
	UINT32 pitch;
};

struct LOADER_PARAMS {
	UINT32					UEFI_Version;
	UINT32					Bootloader_MajorVersion;
	UINT32					Bootloader_MinorVersion;

	UINT32					Memory_Map_Descriptor_Version;
	UINTN					Memory_Map_Descriptor_Size;
	EFI_MEMORY_DESCRIPTOR	*Memory_Map;
	UINTN					Memory_Map_Size;

	EFI_PHYSICAL_ADDRESS	Kernel_BaseAddress;
	UINTN					Kernel_Pages;

	EFI_RUNTIME_SERVICES	*RTServices;
	struct GPU_CONFIG		GPU_Configs;
	EFI_FILE_INFO			*FileMeta;
	EFI_CONFIGURATION_TABLE	*ConfigTables;
	UINTN					Number_of_ConfigTables;

	EFI_TIME				BootTime;
};

struct TIME_VALUE
{
	int second;
	int minute;
	int hour;
	int year;
	int month;
	int day;
};

struct CURSOR
{
	int position;
	int y;
	int x;
};

struct SCREEN_VALUE
{
	int VBWidth;
	int VBHeight;
	int VBSize; // Video Buffer Size
	uint32_t *VB;
	int FBWidth;
	int FBHeight;
	int FBSize; // Frame Buffer Size
	int FBPitch;
	uint32_t *FB;
	int caracter_width;
	int caracter_height;
	uint8_t (*police)[12][8];
	int pixel_position;
};


struct VARIABLE
{
	struct CURSOR cursor;
	struct ETHENDU ethendu;
	struct KNOPF_VARIABLE knopf;
	struct KEY_VARIABLE key;
	struct {const char *shutdown; const char *exit; const char *clear; const char *clist;} command;
	struct {char *msg; int value;} status;
	struct TIME_VALUE time;
	struct SCREEN_VALUE screen;
	struct LOADER_PARAMS *params;
	uint8_t isQEMU;
	volatile uint8_t last_scancode;
	volatile uint8_t scancode;
};

extern struct VARIABLE var;
void set_loader_params(struct LOADER_PARAMS *params);

#endif
// Fin global_value.h
