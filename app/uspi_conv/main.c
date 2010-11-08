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
    char data[9];
} uspi_block;

FILE *infile=NULL;
FILE *outfile=NULL;
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
    printf("Usage: uspi_conv -i input_filename -o output_filename\n");
}

void parse(int argc, char** argv)
{
    int arg=1;

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
            infile=fopen(argv[arg],"rb");
            if(!infile)
            {
                printf("Can't open %s for reading\n",argv[arg]);
                exit(1);
            }
            printf("Input file: %s\n",argv[arg]);
            arg++;
        }else
        if(!strcmp(argv[arg],"-o"))
        {
            arg++;
            outfile=fopen(argv[arg],"wb");
            if(!outfile)
            {
                printf("Can't open %s for writing\n",argv[arg]);
                exit(1);
            }
            printf("Output file: %s\n",argv[arg]);
            arg++;
        }else
        {
            usage();
            exit(1);
        }
    }
    if(!infile){
        printf("Please specify input file\n");
        exit(1);
    }
    if(!outfile){
        printf("Please specify output file\n");
        exit(1);
    }

}

int main(int argc, char** argv)
{
    unsigned sample_count=0;
    parse(argc,argv);
    fseek(outfile,sizeof(wave_header),SEEK_SET);
    while(fread(&uspi_data,13,1,infile)==1)
    {
        fwrite(uspi_data.data,9,1,outfile);
        sample_count++;
    }
    fseek(outfile,0,SEEK_SET);

    wheader.Subchunk2Size=sample_count*3*3;
    wheader.RiffSize=wheader.Subchunk2Size+36;
    fwrite(&wheader,sizeof(wave_header),1,outfile);
    return 0;
}
