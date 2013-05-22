/*   RKImageSplitter.c
 *      
 *   Copyright 2013 Bartosz Jankowski
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>

#pragma pack(1)

struct RKFWHeader
{
	unsigned int 	header;
	unsigned short 	size;
	unsigned int	version;
	unsigned int	code;
	
	struct RKFWTime
	{
		unsigned short	year;
		unsigned char	month;
		unsigned char	day;
		unsigned char	hour;
		unsigned char	minute;
		unsigned char	second;
	} rktime;
	unsigned int	chip;
	unsigned char _padding[3];
};
#pragma pack()

static void usage(char **argv)
{
	printf("Usage: %s file\n", argv[0]);
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int c, i_opt = 0;
	FILE * f;
	char *fname = 0;
	
	if (argc == 2 )
	{
		f = fopen(argv[1], "rb");
		if(!f)
		{
			printf("File %s doesn't exists!\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
		fname = argv[optind];
	}
	else	
		usage(argv);
		
	printf("===========================================\n"
		   "Rockchip Firmware Splitter v. 0.2 by lolet\n"
		   "===========================================\n");
	printf ("Input file: %s\n", fname);
	
    fseek (f , 0 , SEEK_END);
	int s = ftell (f);
	rewind (f);
	if(s<sizeof(struct RKFWHeader)) 
	{
		printf("File is too small!\n");
		fclose(f);
		exit(EXIT_FAILURE);
	}
	
	struct RKFWHeader rkh = {0};
	fread(&rkh, sizeof(rkh), 1, f);
	char *cs = (char*)&rkh.header;
	printf("File signature: %c%c%c%c\n", cs[0],cs[1],cs[2],cs[3]);
	
	if(rkh.header == 0x46414B52)
	{
		printf("File is already splited!\n");
		fclose(f);
		exit(EXIT_FAILURE);
	}
	
	if(rkh.header != 0x57464B52) 
	{
		printf("Wrong file signature!\n");
		fclose(f);
		exit(EXIT_FAILURE);
	}
	
	printf("Firmware version:\t%d.%d.%.2d\n", rkh.version >> 24 & 255, rkh.version >> 16 & 255, rkh.version & 0xFFFF);
	printf("Firmware date:\t\t%d.%.2d.%.2d %.2d:%.2d:%.2d\n", rkh.rktime.year, rkh.rktime.month, rkh.rktime.day, rkh.rktime.hour, rkh.rktime.minute, rkh.rktime.second);
	
	int sig = 0;
	while(!feof(f))
	{
		fread(&sig,1,4,f);
			if(sig == 0x46414B52)
			{
				printf("Found RKAF signature!\n");
				break;
			}
		if(feof(f)) 
		{
			printf("RKAF signature not found!\n");
			fclose(f);
			exit(EXIT_FAILURE);
		}
	}
	
	char nf[255] = {0};
	char *f_ext = strrchr(fname,(int)'.');
	f_ext[0] = 0;
	
	sprintf(nf,"%s_u.img",fname);
	
	FILE * ft = fopen(nf,"wb");
	
	if(!ft)
	{
		printf("Cannot create temporary file (%s)!", nf);
		fclose(f);
		exit(EXIT_FAILURE);
	}
	printf ("Output file: %s\n", nf);
	printf("%d / %d", (int)ftell(f), s-32);
	fwrite(&sig,1,4,ft);
	
	int t = time(NULL);
	int smb = s/1024;
	
	while(!feof(f))
	{
		char buff[8192] = {0};
		int n = fread(&buff,1,8192,f);
		fwrite(buff, 1, feof(f) ? n-32 : n, ft);
		if(time(NULL) - t % 8192) 
		printf("\r%d M / %d M", (int)(ftell(f) / 1024), smb);
	}
	
	fclose(ft);
	fclose(f);
	
	printf("\nDone.\n");
	return EXIT_SUCCESS;
}

