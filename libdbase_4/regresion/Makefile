#
# Makefile libdollybase
#
#

CC = gcc
all: test.c
	@echo "**************************************************************"
	@echo "*					                    *"
	@echo "*  Compiling regresion test for libDollyBASE Alpha release   *"
	@echo "*			                                    *"
	@echo "**************************************************************"
	@$(CC) test.c ../.libs/libdbase_0.4_s.a -o tester -g -O0

clean: 
	rm -f tester test.dbf test.dbt test.csv test.sql a.ndx test.html test.lbl

