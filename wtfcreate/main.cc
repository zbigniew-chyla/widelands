/*
 * Copyright (C) 2002 by Florian Bluemel
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdio.h>
#include <string.h>
//#include "../src/mydirent.h"
#include "../src/tribefiletypes.h"
#include "../src/worldfiletypes.h"
#include "../src/myfile.h"
#include "confreader.h"
#include "../wwfcreate/picturereader.h"

#define CONF_NAME		"conf"
#define PICTURE_DIR		"pics"
#define WTF_EXT			".wtf"
#ifdef WIN32
#define SEPERATOR		'\\'
#else
#define SEPERATOR		'/'
#endif

#define VERSION			"0.9pre"

char dirname[1024];
char outfile[1024];

void printHelp()
{
	printf("creates wide lands tribe files.\n");
	printf("usage:\n");
	printf("\twtfcreate <inputdir> [outputfile]\n");
}

void wtfcreate(Binary_file* output)
{
	char confName[1024];
	strcpy(confName, dirname);
	strcat(confName, CONF_NAME);
	char picsName[1024];
	strcpy(picsName, dirname);
	strcat(picsName, PICTURE_DIR);
	Conf_Reader* conf = new Conf_Reader(confName);
	Picture_Reader* pics = new Picture_Reader(picsName);
	
	// first, the header.
	const TribeFileHeader* head = conf->get_header();
	output->write(head, sizeof(TribeFileHeader));

	for (uint i=0; i<head->buildings; i++)
		output->write(conf->get_building(i), sizeof(BuildingDesc));

	for (uint j=0; j<head->creatures; j++)
		output->write(conf->get_creature(j), sizeof(CreatureDesc));

	for (uint k=0; k<head->wares; k++)
		output->write(conf->get_ware(k), sizeof(Ware));
	
	for (uint m=0; m<conf->textures(); m++)
	{
		const char* picName = conf->get_texture(m);
		// read + write pics
		PictureInfo info;
		ushort* pixels;
		pics->read_picture(picName, &info, &pixels);
		output->write(&info, sizeof(PictureInfo));
		output->write(pixels, info.width*info.height*sizeof(ushort));
		if (!info.name[0])
			printf("problems with picture #%i (%s)\n", m, picName);
		delete pixels;
		//printf("picture writing not implemented:\t#%i\t(%s)\n", m, picName);
	}
	delete conf;
}

int main(int argc, char* argv[])
{
	printf("wtfCreate v%s\n", VERSION);
	if (argc == 1 || argc > 3)
	{
		printHelp();
		return -1;
	}

	// get input dir name
	strcpy(dirname, argv[1]);
	if (dirname[strlen(dirname) - 1] != SEPERATOR)
	{
		dirname[strlen(dirname)] = SEPERATOR;
		dirname[strlen(dirname) + 1] = 0;
	}
	// get output file name
	if (argc == 3)
		strcpy(outfile, argv[2]);
	else
	{
		strcpy(outfile, dirname);
		outfile[strlen(outfile)-1] = 0;
		strcat(outfile, WTF_EXT);
	}
	
	Binary_file* output = new Binary_file();
	output->open(outfile, File::WRITE);
	wtfcreate(output);
	delete output;

	getchar();
	return 0;
}
