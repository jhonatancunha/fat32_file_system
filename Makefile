all:
	gcc -g -c utils/utils.c
	gcc -g -c ListDirEntry/listDirEntry.c
	gcc -g -c StackDirectory/stackDirectory.c
	gcc -g -c main.c
	gcc -g -c commands/ls/ls.c
	gcc -g -c commands/cluster/cluster.c
	gcc -g -c commands/rename/rename.c
	gcc -g -c commands/touch/touch.c
	gcc -g -c commands/attr/attr.c
	gcc -g -c commands/rm/rm.c
	gcc -g -c commands/cd/cd.c
	gcc -g -c commands/pwd/pwd.c
	gcc -g -c commands/mv/mv.c
	gcc -g -c commands/cp/cp.c
	gcc -g -c commands/info/info.c
	gcc -g -c commands/mkdir/mkdir.c
	gcc -g -c commands/rmdir/rmdir.c
	gcc -g -c commands/help/help.c
	gcc -o main *.o -lm

clean:
	rm *.o main