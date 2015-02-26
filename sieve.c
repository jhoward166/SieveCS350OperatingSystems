#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

int MAX_WORKERS = 16;
int shm_fd;
int *shared_mem;
int *primes;
int *curr;
sem_t sem;
 
int main(int argc, char **argv){
	int pid;
	int workers;
	int size;
	int i=0;
	int k=0;
	int h=0;
	int j= 0;
	int l= 3;
	int loop =1;
	int proc = 1;
	int cap = 0;
	int pList[16]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53};
	char w[2] = "-w";
  	char s[2] = "-s";
	sem_init(&sem,1,1);
	if(argc != 5){
		printf("Error: Arguments invalid.\n");
		return 0;
	}
	if(strcmp(argv[1],"-s")==0){
		if(strcmp(argv[3],"-w")==0){
			workers = atoi(argv[4]);
			size = atoi(argv[2]);
		}else{
			printf("Error: Second flag not recognized.\n");
			return 0;
		}
	}else if(strcmp(argv[1],"-w")==0){
		if(strcmp(argv[3],"-s")==0){
			workers = atoi(argv[2]);
			size = atoi(argv[4]);
			size=size+1;
		}else{
			printf("Error: Second flag not recognized.\n");
			return 0;
		}
	}else{
		printf("Error: First flag not recognized.\n");
		return 0;
	}
	if(workers > 16){
		printf("Number of workers must be below %d.\n", MAX_WORKERS);
		return 0;
	}
	int test = 1;
	int flag = 0;
	while((test<=workers) && (flag ==0)){
		if(test == workers){
			flag =1;
		}else{
			test= test<<1;
			cap++;
		}
	}
	if(flag==0){
		printf("Number of workers must be a power of 2.\n");
		return 0;
	}
	//printf("%d %d\n", workers, size);
	//if ((shm_fd = shm_open("name", O_CREAT | O_RDWR, 0666)) == -1) { 
	if((shm_fd=open("name", O_RDWR, 0666)) == -1){
      perror("shm_open");
      exit(1);
  	}
 	if (ftruncate(shm_fd, size * sizeof(int)) == -1) { 
      perror("ftruncate");
      exit(1);
   	}

   	shared_mem = mmap(NULL, size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

   	if (shared_mem == MAP_FAILED) {
  	  perror("mmap");
      exit(1);
   	} 
    if (close(shm_fd) == -1) {
       perror("close");
       exit(1);
    }
	for(k=0; k<size+1; k++){
		shared_mem[k]=0;
	}
	shared_mem[0]=workers;
	for(j; j<cap; j++){
		pid = fork();
		if(pid==0){
			proc=(proc*2);
		}
		if(pid != 0){
			pid=fork();
			if(pid==0){
				proc=((proc*2)+1);
			}
		}
		if(pid != 0){
			j = cap;
		}
	}
	wait(NULL);
	if(pid == 0){
		int gate = 0;
		int workNum = (workers+1)-((workers*2)-proc);
		int prime = pList[workNum-1];
		//printf("%d \n", prime);
		while(gate == 0){
			int counter = 2;
			//printf("%d %d %d \n", prime, counter, (prime*counter));
			while((prime*counter) <= size){
				if(shared_mem[(prime*counter)]==0){
					shared_mem[(prime*counter)] = workNum;		
				}
				counter++;
			}
			int last = -1;
			int z = prime+1;
			for(z; z<size; z++){
				if(z<size){
					j=0;
					if(shared_mem[z]==0){
						last =prime;
						prime = z;
						z=size;
					}
				}
			}
			if(last == -1){
				gate = 1;
			}
		}
	}
	wait(NULL);
	if(proc != 1){
		exit(0);
	}
	j=size-1;
	int accum = 0;
	int y=0;
	int workCount[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for(j; j>1; j--){
		if(shared_mem[j]==0){
			accum++;
		}else{
			workCount[(shared_mem[j]-1)]++;
		}
	}
	printf("Number of total primes:%d \n\n", accum);
	for(y; y<workers; y++){	
		printf("Worker %d completed %d.\n", y+1, workCount[y]);
	}
	printf("\n");
	j=size-1;
	y=0;
	printf("Top 10 primes: \n");
	for(y; y<10;){	
		for(j; j>1; j--){
			if(shared_mem[j]==0){
				printf("%d \n", j);
				y++;
			}
			if(y==10){
				j=0;
			}
		}
	}
	printf("\n");
	j=size-1;
	y=0;
	printf("Workers the found the last 20 non-primes: \n");
	for(y; y<20;){	
		for(j; j>1; j--){
			if(shared_mem[j]!=0){
				printf("%d \n", shared_mem[j]);
				y++;
			}
			if(y==20){
				j=0;
			}
		}
	}
	printf("\n");
	if (munmap(shared_mem, size * sizeof(int)) == -1) {
      perror("munmap");
      exit(1);
   	}
	exit(0);
}
