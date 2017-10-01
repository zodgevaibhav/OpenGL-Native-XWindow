#include<stdio.h>
#include<conio.h>
#include<string.h>
int main()
{
	union MyUnion
	{
		int a;
		float b;
		char str[12];
	};

	MyUnion myUnion;
	printf("************************* Learning of Union ********************\n");
		myUnion.a = 10;
	printf("Value of member a is %d\n", myUnion.a);
	myUnion.b = 5;
	printf("Value of member b is %f\n", myUnion.b);
	strcpy(myUnion.str, "Hello World ;) \n");
	printf("Value of member str is %s \n", myUnion.str);

	printf("************************* Only one member can hold the data at a time *******************\n");

	printf("Value of member a is %d\n", myUnion.a);
	printf("Value of member b is %f\n", myUnion.b);
	printf("Value of member str is %s \n\n", myUnion.str);

	printf("************************* Size of union will be the size of member which need max memory *******************\n");

	printf("Value of member a is %d \n", sizeof(myUnion));
	getch();
}