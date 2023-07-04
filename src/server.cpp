#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <getopt.h>
#include "binfhecontext-ser.h"
#include "counter.h"
#include "query.h"
#include "data.h"
#include "str_cmp.h"
#include <time.h>
#include <future>



int main(int argc , char* argv[])
{
	int c = 0;
	option opt[] = 
	{
		{"counter" , 0 , NULL , 'c'},
        {"query" , 0 , NULL , 'q'},
	};
	while((c = getopt_long(argc , argv , "cq" , opt , NULL)) != -1)
	{
		switch(c)
		{
			case 'c':
				counter();
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



