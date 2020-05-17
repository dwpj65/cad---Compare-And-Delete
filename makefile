
cad:	cad.c
	gcc -v -o cad cad.c

cad_test_brd_dir: cad
#	@echo "Starting"
	-ls -l /home/osboxes/ripping /home/osboxes/burner
#	diff -qr /home/osboxes/ripping /home/osboxes/burner
#	@echo "Ending"
	sudo ./cad /home/osboxes/ripping /home/osboxes/burner
	-ls -l /home/osboxes/ripping /home/osboxes/burner

cad_test_new: cad
	./cad

archive: cad.rar

cad.rar: cad.c makefile
	rar a -ag_yyyymmddhhmmss cad.rar -x*.rar *

cad.i: cad.c
	cpp cad.c > cad.i

cad.s: cad.i
	gcc -S cad.i

cad.o: cad.s
	as -o cad.o cad.s



