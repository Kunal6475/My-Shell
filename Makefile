CFlags= -Wall -ansi -pedantic
MyShell: MyShell.c myls.c
	 ${CC} ${CFlags} MyShell.c -o MyShell 
	 ${CC} ${CFlags} myls.c -o myls 
