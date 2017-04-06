#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/syscall.h>

long bitXOR(long x, long y){
	return x ^ y;
}

int main(int argc, char* argv[]){

	char * someArray = "How do I do this?";
	int someArrayLen = strlen(someArray);
	int arrayLenDivisibleBy4 = someArrayLen;
	arrayLenDivisibleBy4 += (someArrayLen % 4 == 0) ? 0 : 4 - someArrayLen % 4;

	printf("someArrayLen: %ld\n", someArrayLen);
	printf("arrayLenDivisibleBy4: %ld\n", arrayLenDivisibleBy4);

	long * someByteArray = malloc(arrayLenDivisibleBy4);
	someByteArray = (long *)someArray;

	printf("Number of elements in someByteArray: %ld\n", arrayLenDivisibleBy4/sizeof(long));

	int i;
	for (i = 0; i<5; i++){
		printf("Element %d: %ld\n", i,someByteArray[i]);
	}

	printf("\n");

	long * encryptedByteArray = malloc(arrayLenDivisibleBy4);

	long key = 22;


	for (i = 0; i<5; i++){
		encryptedByteArray[i] = bitXOR(someByteArray[i], key);
		printf("Element %d: %ld\n", i,encryptedByteArray[i]);
	}

	printf("\n");

	for (i = 0; i<5; i++){
		encryptedByteArray[i] = bitXOR(encryptedByteArray[i], key);
		printf("Element %d: %ld\n", i,encryptedByteArray[i]);
	}

	char * someNewArray = malloc(sizeof(char)*arrayLenDivisibleBy4/sizeof(long));

	someNewArray = (char *)encryptedByteArray;

	printf("\n");

	printf("The original string: \n%s", someNewArray);


	return 0;

}