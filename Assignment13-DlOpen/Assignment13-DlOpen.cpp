#include<stdio.h>
#include<dlfcn.h>
#include<stdlib.h>

int main()
{
	typedef int (*MySqaure)(int);
	//typedef double (*MySqaure)(double);  //for libm.so
	MySqaure pFn;
	void *hDll;	
	
	hDll=dlopen("./square.so",RTLD_LAZY);
	//hDll=dlopen("/lib/x86_64-linux-gnu/libm.so.6",RTLD_LAZY); //for libm.so
	if(!hDll)
	{
		fputs(dlerror(),stderr);
	}
	pFn=(MySqaure) dlsym(hDll,"sqr");
	//pFn=(MySqaure) dlsym(hDll,"sqrt"); //for libm.so
	if(!pFn)
	{
		fputs(dlerror(),stderr);
	}
	int intSquare=pFn(16);
	printf("\n ******* Output of function is %d \n",intSquare);
	dlclose(hDll);
}
