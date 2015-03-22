void fillrand(char *buf, int len)
{   static unsigned long count = 4;
    static char          r[4];
    int                  i;

    for(i = 0; i < len; ++i)
    {
        if(count == 4)
        {
            count = 0;
        }

        buf[i] = r[count++];
    }
}    

int encfile(FILE *fin, FILE *fout, aes *ctx, char* fn)
{   
	  char            outbuf[16];

    fillrand(outbuf, 16);           /* set an IV for CBC mode           */

		return 0;
}

int main(int argc, char *argv[])
{   
    err = encfile(fin, fout, ctx, argv[1]);
    return err;
}


