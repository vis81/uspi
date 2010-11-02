#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*************************************************************************
				TYPEDEFS
**************************************************************************/
typedef enum _OUT_FORMAT{
    FORMAT_VOLTAGE,
    FORMAT_INTEGER,
    FORMAT_COUNT
}OUT_FORMAT;



/*************************************************************************
				PRIVATE VARS
**************************************************************************/
char* strformat[FORMAT_COUNT]={"voltage","integer"};



/*************************************************************************
				FUNCTION DEFINITIONS
**************************************************************************/

//Print usage information
void usage()
{
    printf("Usage: uspi_check filename [options]\n");
    printf(" -header [headersize in bytes 1..4]\n");
    printf(" -format [voltage/integer]\n");
    printf(" -o [output file name]\n");
    printf(" -help  - display this help\n");
}

//Output to the text file
void output(FILE* ftxt,OUT_FORMAT format,unsigned timestamp,char* data){
    unsigned i;
    if(format==0)
        fprintf(ftxt,"%.012f",(double)timestamp/32000.0);
    else
        fprintf(ftxt,"%8u",timestamp);
    for(i=0;i<3;i++)
    {
        unsigned char *padcval;
        int int_val;
        double double_val;

        padcval=(unsigned char*)&data[i*3];
        int_val=((padcval[0]<<16)&0xFF0000) |
                ((padcval[1]<<8)&0xFF00) |
                (padcval[2]&0xFF);
        if(int_val&0x800000)//extend sign
            int_val|=0xFF000000;

        if(int_val)
            double_val=int_val*5.0/16777216.0;
        else
            double_val=0.0;

        if(format==FORMAT_VOLTAGE)
            fprintf(ftxt,"\t%.12f",double_val);
        else
            fprintf(ftxt,"\t%8d",int_val);
    }
    fprintf(ftxt,"\n");
}

int main(int argc, char** argv)
{
    char        tmp[20];            //temp buffer
    FILE*       f_input=0;          //input file
    FILE*       ftxt=0;             //output file
    unsigned    timestamp;          //timestamp
    unsigned    totalmiss=0;        //total packets missed
    unsigned    total=0;            //total packets parsed
    unsigned    hsize=4;            //header size
    unsigned    mask;               //header mask
    OUT_FORMAT  format=FORMAT_VOLTAGE;//output format
    unsigned    arg=1;              //args loop variable

    //Parse argumens
    while(arg<argc)
    {
        if(!strcmp(argv[arg],"-help"))
        {
            usage();
            return 1;
        }else
        if(!strcmp(argv[arg],"-header"))
        {
            arg++;
            hsize=atoi(argv[arg]);
            if(hsize>4){
                printf("Header size must be in range 1..4\n");
                return 1;
            }
            arg++;
        }else
        if(!strcmp(argv[arg],"-format"))
        {
            arg++;
            if(!strcmp(argv[arg],"voltage"))
                format=0;
            else
                if(!strcmp(argv[arg],"integer"))
                    format=1;
                else
                {
                    printf("Unknown format %s\n",argv[arg]);
                    return 1;
                }
            arg++;
        }else
        if(!strcmp(argv[arg],"-o"))
        {
            arg++;
            ftxt=fopen(argv[arg],"w");
            if(!ftxt)
            {
                printf("Can't open %s for writing\n",argv[arg]);
                return 1;
            }
            printf("Output file: %s\n",argv[arg]);
            arg++;
        }else{
            f_input=fopen(argv[arg],"rb");
            if(!f_input)
            {
                printf("Can't open %s for reading\n",argv[arg]);
                usage();
                return 1;
            }
            printf("Source file: %s\n",argv[arg]);
            arg++;
        }
    }

    if(!f_input){
        usage();
        return 1;
    }

    if(ftxt)
        printf("Output format: %s\n",strformat[format]);
    printf("Header size: %u\n",hsize);


    //Calculate mask
    if(hsize<4)
        mask=(1<<(8*hsize))-1;
    else
        mask=0xffffffff;


    //main loop
    timestamp=-1;
    while(fread(tmp,hsize+9,1,f_input)==1)
    {
        unsigned curts=*(unsigned*)&tmp[0];
        unsigned miss;
        total++;
        curts&=mask;
        miss=curts-timestamp-1;
        miss&=mask;
        if(miss)
            printf("Prev=%u Cur=%u miss=%d\n",timestamp,curts,miss);
        totalmiss+=miss;
        timestamp=curts;

        //if output required
        if(ftxt)
            output(ftxt,format,timestamp,&tmp[hsize]);
    };

    printf("Total packets parsed= %u, missed=%u\n",total,totalmiss);

    if(ftxt)
        fclose(ftxt);

    fclose(f_input);
    return 0;
}
