#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <time.h>

#define SIZE 1024
#define READ 1
#define WRITE 0

void io_test(int fp, int len, int op)
{
	char buf[SIZE];
	char type[8];
	clock_t begin, end;
	double cost_time;
	int i;

	if(op==READ)
	{
		strcpy(type, "read");
		memset(buf, 0, SIZE*sizeof(char));

		begin = clock();
		for(i=0; i<len; ++i)
		{
			if( read(fp, buf, SIZE)<0 ){
				printf( "File Read Error!\n");
				exit(1);
			}
//			printf(" read:\n%s\n", buf);
		}
		end = clock();
	}
	else
	{
		strcpy(type, "write");
		srand(time(NULL));
		for(i=0; i<SIZE; ++i)
			buf[i]=rand()%95+32;

		begin = clock();
		for(i=0; i<len; ++i)
		{
			if( write(fp, buf, SIZE)!=SIZE ){
				printf( "File Write Error!\n"); 
				exit(1);
			}
//			printf("write:\n%s\n", buf);
		}
		end = clock();
	}

	cost_time = ((double)(end-begin))/CLOCKS_PER_SEC;
	
	printf("%5s %d KB, ", type, len);
	printf("spent %f sce, ", cost_time);
	printf("speed %f MB/s\n", len/(cost_time*1024));
}

int main(int argc, char* argv[])
{
	int fp;
	int len;

	fp = open("/dev/chardevFile", O_RDWR);
//	fp = open("/opt/myTest", O_RDWR);
	if(fp<0)
	{
		printf("can't open device!\n");
		return 0;
	}

	if(argc<2)
		len = 1;
	else
		len = atoi(argv[1]);

	io_test(fp, len, WRITE);
	io_test(fp, len,  READ);

	close(fp);

	return 0;
}
