/* Program to convert Sun Sparc sound file (*.au) to NeXT sound file (*.snd) */
/* Izumi Ohzawa, 8-14-90 */
/* cc -O -o au2snd au2snd.c -lsys_s */

#include <stdio.h>
#include <sound/sound.h>
#include <sys/stat.h>

main(argc, argv)
int argc;
char *argv[];
{
int filesize, bcnt;
char buff[1024];
SNDSoundStruct sh;
struct stat filestat;
FILE *fpin, *fpout;

	if(argc != 3)
	{
		printf("Usage: au2snd Sparcfile.au NeXTfile.snd\n");
		exit(1);
	}
	if(stat(argv[1], &filestat))
	{
		printf("Can't get stat for file: %s\n", argv[1]);
		exit(2);
	}
	else
		filesize = filestat.st_size;

	fpin = fopen(argv[1], "r");
	if(fpin == NULL)
	{
		printf("Can't open %s\n", argv[1]);
		exit(1);
	}
	fpout = fopen(argv[2], "w");
	if(fpout == NULL)
	{
		printf("Can't open %s\n", argv[2]);
		exit(1);
	}

	sh.magic = SND_MAGIC;
	sh.dataLocation = sizeof(SNDSoundStruct);
	sh.dataSize = filesize;
	sh.dataFormat = SND_FORMAT_MULAW_8;
	sh.samplingRate = SND_RATE_CODEC;
	sh.channelCount = 1;
	bzero(sh.info, 4);

	fwrite((void *)&sh, sizeof(SNDSoundStruct), 1, fpout);

	while((bcnt = fread((void *)buff, 1, sizeof(buff), fpin)) > 0)
		fwrite((void *)buff, 1, (size_t)bcnt, fpout);
	fclose(fpin);
	fclose(fpout);
	exit(0);
}
