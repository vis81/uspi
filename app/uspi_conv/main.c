#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char            RiffID [4] ;
	unsigned int    RiffSize ;
	char            WaveID [4] ;
	char            FmtID  [4] ;
	unsigned int    FmtSize ;
	unsigned short  wFormatTag ;
	unsigned short  nChannels ;
	unsigned int    nSamplesPerSec ;
	unsigned int    nAvgBytesPerSec ;
	unsigned short  nBlockAlign ;
	unsigned short  wBitsPerSample ;
	char            Subchunk2ID[4];
	unsigned int    Subchunk2Size;//== NumSamples * NumChannels * BitsPerSample/8
	char            Data[0];
} wave_header;

typedef struct {
    unsigned int timestamp;
    char data[3][3];
} uspi_block;

FILE *infile=NULL;
FILE *outfile[3]={NULL,NULL,NULL};
int split=0;
uspi_block uspi_data;

wave_header wheader={
    .RiffID="RIFF",
    .RiffSize=-1,
    .WaveID="WAVE",
    .FmtID="fmt ",
    .FmtSize=16,
    .wFormatTag=1,
    .nChannels=3,
    .nSamplesPerSec=32000,
    .nAvgBytesPerSec=32000*3*3,
    .nBlockAlign=3*3,
    .wBitsPerSample=24,
    .Subchunk2ID="data",
    .Subchunk2Size=-1
    };

//Print usage information
void usage()
{
    printf("Usage: uspi_conv -i input_filename [-split]\n");
    printf(" -split - splits output to 3 mono wav files\n");
}

void parse(int argc, char** argv)
{
    int arg=1,i;
    char* outfilename=NULL;
    char* infilename=NULL;
    char infilebuf[100],outfilebuf[100];
    char* pos;

    if(argc==1)
    {
        usage();
        exit(1);
    }
    //Parse argumens
    while(arg<argc)
    {
        if(!strcmp(argv[arg],"-help"))
        {
            usage();
            exit(1);
        }else
        if(!strcmp(argv[arg],"-i"))
        {
            arg++;
            infilename=argv[arg];
            arg++;
        }else
        if(!strcmp(argv[arg],"-split"))
        {
            split=1;
            arg++;
        }else
        {
            usage();
            exit(1);
        }
    }

    infile=fopen(infilename,"rb");
    if(!infile)
    {
        printf("Can't open %s for reading\n",infilename);
        exit(1);
    }
    printf("Input file: %s\n",infilename);

    pos=strchr(infilename,'.');
    if(!pos)
        pos=infilename+strlen(infilename);

    strncpy(infilebuf,infilename,pos-infilename);
    infilebuf[pos-infilename]=0;

    if(split)
    {
        for(i=0;i<3;i++)
        {
            sprintf(outfilebuf,"%s_ch%u.wav",infilebuf,i);
            printf("Output file: %s\n",outfilebuf);
            outfile[i]=fopen(outfilebuf,"wb");
            if(!outfile[i])
            {
                printf("Can't open %s for writing\n",outfilebuf);
                exit(1);
            }
        }
    }else{
        sprintf(outfilebuf,"%s.wav",infilebuf);
        printf("Output file: %s\n",outfilebuf);
        outfile[0]=fopen(outfilebuf,"wb");
        if(!outfile[0])
        {
            printf("Can't open %s for writing\n",outfilebuf);
            exit(1);
        }
    }
}

int main(int argc, char** argv)
{
    unsigned sample_count=0;
    unsigned i;
    parse(argc,argv);

    for(i=0;i<(split?3:1);i++)
        fseek(outfile[i],sizeof(wave_header),SEEK_SET);

    while(fread(&uspi_data,13,1,infile)==1)
    {
        unsigned char data[3][3];
        for(i=0;i<3;i++)
        {
            data[i][0]=uspi_data.data[i][2];
            data[i][1]=uspi_data.data[i][1];
            data[i][2]=uspi_data.data[i][0];
            fwrite(data[i],3,1,outfile[(split?i:0)]);
        }
        sample_count++;
    }
    for(i=0;i<(split?3:1);i++)
    {
        fseek(outfile[i],0,SEEK_SET);
        if(!split){
            wheader.Subchunk2Size=sample_count*3*3;
            wheader.RiffSize=wheader.Subchunk2Size+36;
            wheader.nChannels=3;
            wheader.nAvgBytesPerSec=32000*3*3;
            wheader.nBlockAlign=3*3;
            fwrite(&wheader,sizeof(wave_header),1,outfile[i]);
            break;
        }else{
            wheader.Subchunk2Size=sample_count*3;
            wheader.RiffSize=wheader.Subchunk2Size+36;
            wheader.nChannels=1;
            wheader.nAvgBytesPerSec=32000*3;
            wheader.nBlockAlign=3;
            fwrite(&wheader,sizeof(wave_header),1,outfile[i]);
        }
    }
    return 0;
}
