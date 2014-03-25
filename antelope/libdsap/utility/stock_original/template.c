/****
  name - purpose %%%%

  $log$
  ****/



/****  SCCS uses this...  ****/
static char    *SCCS_ID = "$Name $Revision: 1.1.1.1 $ $Date: 1997/04/12 04:19:03 $";



/****  Standard include files  ****/
#include <stdio.h>
#include <stdlib.h>



/****  Local include files  ****/
#include "tmpl.h"
#include "elog.h"



/****  Constant macro definitions  ****/
#define	MAXSTR        (1024)



/****  Function macro definitions  ****/



/****  Type definitions / Enumerations  ****/



/****  Usage information  ****/
/* %%%% Fill in the following option list with your own %%%% */
static option_t option_list[] =
{
    'i', "input", NORMAL_ARG,
    'o', "output", NORMAL_ARG,
    'd', "debug", NORMAL_ARG,
    'v', "version", NO_ARGUMENT,
    'h', "help", NO_ARGUMENT
};

#define NUM_OPTIONS (sizeof(option_list) / sizeof(option_t))

#ifdef lint
#define USAGE()
#else
#define USAGE() \
  do   { fprintf(stderr, \
	       "usage: %s %%%%\n", \
	       Program_Name); \
       exit(1); \
       } while (0)
#endif


/****  Global variables  ****/
extern char    *Program_Name;	       /* Required for error messages...  */

int             Debug_Level	       /* Adjust verbosity by this amount  */
  = 0;



/****  Forward declarations  ****/
int             process();
char           *get_next_input();



/* ======================================================================= *
 * main - parse arguments, handle options, and call the routine that
 * actually does the work.
 * ======================================================================= */

int
main(argc, argv)

int             argc;
char           *argv[];

{
    char            option	       /* Terse option specifier  */
      = NULL;
    char           *new_arg	       /* Temporary holder for arguments  */
      = NULL;
    int             have_in_file       /* Flag for the -i option  */
      = 0;
    int             have_out_file      /* Flag for the -o option  */
      = 0;
    char           *input_name	       /* Name of the input file  */
      = "stdin";
    FILE           *input_file	       /* File descriptor for input  */
      = stdin;
    char           *output_name	       /* Name of the output file  */
      = "stdout";
    FILE           *output_file	       /* File descriptor for output  */
      = stdout;

    /****  Remember who you are  ****/
    Program_Name = argv[0];

    /****  Parse every option in the command line  ****/
    while ((option = get_option(&argc, argv, option_list, NUM_OPTIONS))
	   != NULL)
      {
	/* %%%% Add/replace your options below %%%% */
	switch (option)
	  {
	case ('i'):
	    if (have_in_file
		|| ((input_name = get_input_file(&input_file)) == NULL))
	      {
		USAGE();
	      }
	    have_in_file = 1;
	    break;
	case ('o'):
	    if (have_out_file
		|| ((output_name = get_output_file(&output_file)) == NULL))
	      {
		USAGE();
	      }
	    have_out_file = 1;
	    break;
	case ('d'):
	    if ((new_arg = get_argument()) != NULL)
	      {
		Debug_Level = atoi(new_arg);
	      }
	    else
	      {
		USAGE();
	      }
	    break;
	case ('v'):
	    (void) printf("Version: %s\n", SCCS_ID + 4);
	    exit(0);
	case ('h'):
	    USAGE();
	default:
	    clear_register(1);
	    USAGE();
	  }
      }

    /****  Use the first file specified, or stdin  ****/
    /****  %%%%  Do you want this?  %%%%  ****/
    if (!have_in_file
	&& ((input_name = get_input_file(&input_file)) == NULL))
      {
	clear_register(1);
	input_name = "stdin";
	input_file = stdin;
      }

    /****  Use the first file specified, or stdout  ****/
    /****  %%%%  Do you want this?  %%%%  ****/
    /**
      if (!have_out_file
      && ((output_name = get_output_file(&output_file)) == NULL))
        {
      clear_register(1);
      output_name = "stdout";
      output_file = stdout;
        }
      **/

    /****  Select the last file as the output, or stdout  ****/
    /****  %%%%  Do you want this?  %%%%  ****/
    if (!have_out_file
	&& ((output_name = last_output_file(&output_file)) == NULL))
      {
	clear_register(1);
	output_name = "stdout";
	output_file = stdout;
      }

    /****  Do the work here  ****/
    /* %%%% Replace / edit process with your main routine. %%%% */
    return process(input_name, input_file, output_name, output_file);
}



/* ======================================================================= *
 * process - the main routine goes here.
 * ======================================================================= */

int
process(input_name, input_file, output_name, output_file)

char           *input_name;
FILE           *input_file;
char           *output_name;
FILE           *output_file;

{
    char            line[MAXSTR];

    /****  Ensure that we did get an input / output file  ****/
    if ((input_name != NULL) && (output_name != NULL))
      {
	while ((input_name = get_next_input(line, (int) sizeof (line),
					    &input_file, input_name))
	       != NULL)
	  {
	    (void) fprintf(output_file, "%s", line);
	  }

	(void) fclose(input_file);
	(void) fclose(output_file);

	return 0;
      }

    /****  Something went terribly wrong  ****/
    (void) fclose(input_file);
    (void) fclose(output_file);

    clear_register(1);
    return 1;
}



/* ======================================================================= *
 * get_next_input - get more input.  Go to the next file as required.
 * ======================================================================= */

char           *
get_next_input(ptr, size, input_file, input_name)

char           *ptr;
int             size;
FILE          **input_file;
char           *input_name;

{

    /****  Replace fgets as appropriate to get more data.  ****/
    while (fgets(ptr, size, *input_file) == NULL)
      {
	if ((input_name = get_input_file(input_file)) == NULL)
	  {
	    return NULL;
	  }
      }

    return input_name;
}

/* $Id: template.c,v 1.1.1.1 1997/04/12 04:19:03 danq Exp $ */
