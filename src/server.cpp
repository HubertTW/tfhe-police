#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <getopt.h>
#include "binfhecontext-ser.h"
#include "encode.h"
#include "header
#include "str_cmp.h"
#include <time.h>
#include <future>

int help();
int query();
int count();


int main(int argc , char* argv[])
{
	int c = 0;
	option opt[] = 
	{
		{"help" , 0 , NULL , 'h'} ,
		{"query" , 0 , NULL , 'q'},
	};
	while((c = getopt_long(argc , argv , "heq" , opt , NULL)) != -1)
	{
		switch(c)
		{
			case 'h':
				help();
				break;
			case 'q':
				query();
				break;
			case '?':
				std::cout << "unknown argument.\n";
				break;
			default:
				std::cout << "unknown error.\n";
		}
	}
	return 0;
}



