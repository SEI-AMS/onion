/*
	Onion HTTP server library
	Copyright (C) 2010 David Moreno Montero

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

void print_help();
char *funcname(const char *filename);
void parse_file(const char *filename, FILE *outfd);

int main(int argc, char **argv){
	if (argc==1)
		print_help(argv[0]);
	int i;
	char *outfile=NULL;
	// First pass cancel out the options, let only the files
	for (i=1;i<argc;i++){
		if (strcmp(argv[i],"--help")==0)
			print_help();
		if (strcmp(argv[i],"-o")==0){
			if (i>=argc-1){
				fprintf(stderr,"ERROR: Need an argument for -o");
				exit(2);
			}
			outfile=strdup(argv[i+1]);
			argv[i]=NULL; // cancel them out.
			argv[i+1]=NULL; 
			i++;
		}
	}
	
	FILE *outfd=stdout;
	if (outfile){
		outfd=fopen(outfile,"w");
		if (!outfd){
			perror("ERROR: Could not open output file");
			exit(2);
		}
	}
	
	// Some header...
	fprintf(outfd,"/** File autogenerated by opack **/\n\n");
	fprintf(outfd,"#include <onion_response.h>\n\n");
	
	for (i=1;i<argc;i++){
		if (argv[i]){
			parse_file(argv[i], outfd);
		}
	}
	
	if (outfile){
		free(outfile);
		fclose(outfd);
	}
	
	return 0;
}

/// Parses a filename to a function name (changes non allowed chars to _)
char *funcname(const char *filename){
	char *ret=malloc(strlen(filename)+6);
	strcpy(ret,"opack_");
	int i=0;
	while(filename[i]!='\0'){
		if (!((filename[i]>='a' && filename[i]<='z') || (filename[i]>='A' && filename[i]<='Z')))
			ret[i+6]='_';
		else
			ret[i+6]=filename[i];
		i++;
	}
	return ret;
}

/**
 * @short Generates the necesary data to the output stream.
 */
void parse_file(const char *filename, FILE *outfd){
	FILE *fd=fopen(filename, "r");
	if (!fd){
		fprintf(stderr,"ERROR: Cant open file %s",filename);
		perror("");
		exit(3);
	}
	char *fname=funcname(basename((char*)filename));
	char buffer[1024];
	
	fprintf(stderr, "Parsing: %s to 'int %s(onion_response *res);'.\n",filename, fname);
	fprintf(outfd,"int %s(onion_response *res){\n  char data[]={\n",fname);
	int r, i;
	while ( (r=fread(buffer,1,sizeof(buffer),fd)) !=0 ){
		for (i=0;i<r;i++){
			fprintf(outfd,"0x%02X, ", buffer[i]&0x0FF);
		}
	}
	fprintf(outfd,"};\n");

	fprintf(outfd,"  return onion_response_write(res, data, sizeof(data));\n}\n");

	
	fclose(fd);
	free(fname);
}

/// Shows the help
void print_help(const char *name){
	fprintf(stderr,"%s -- Packs a given file or files into a C function that will print it out\n\n", name);
	fprintf(stderr,"Usage: %s <file1> <file2> -o <outfile.c> | --help\n\n", name);
	fprintf(stderr,"It later creates a series of functions, with the name of the file, and with the following signature.\n");
	fprintf(stderr,"   int opacked_[file_extension](onion_response *response);\n\n");
	fprintf(stderr,"This way this function is very easily used from onion handlers as needed.\n");
	exit(1);
}
