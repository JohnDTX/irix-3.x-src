/*
 *
 *	mktemplate  - make the template data for the SGI mc68020
 *		      assembler.
 *
 *	    The input file is read and used to creat two output 
 *	.c files - one for the templates themselves, and one for
 *	the hash entries.
 *
 */

#include <stdio.h>
#include <ctype.h>
FILE *inputfile,*hashfile,*templatefile;

char *hashfname = "insthash.c";
char *templatefname = "template.c";
char *infname;
char buf[0x100];
char curinst[0x20];
int comment=0,verbatim=0,doinginst=0,tindx=0,templindx=0;
int lasttemplindx=0;


main(argc,argv)
int argc;
char **argv;
{

	char c,*new,*temp;
	argv++;
	/* single argument - the input filename */
	infname = *argv;
	if (infname == (char *)0) {
		fatal("no input filename\n");
	}
	

	/* comments are delimited by a tilde (~) as the only character
	   on a line.  Lines between v[erbatim] and e[nd] lines are copied
	   verbatim to both output files.  Other lines are considered
	   to be templates.
	*/

	if ((inputfile = fopen(infname,"r")) == NULL)
		fatal("cant open input file");
	if ((templatefile = fopen(templatefname,"w")) == NULL)
		fatal("cant open template file");
	if ((hashfile = fopen(hashfname,"w")) == NULL)
		fatal("cant open hash file");

	while (fgets(buf,0x100,inputfile) != NULL)
	{
		c = buf[0];
		if (verbatim && (c != 'e')) goto doverbatim;
		if (comment && (c != '~')) continue;
		switch (c)
		{
			case 'v':
				if (verbatim) goto doverbatim;
				verbatim++;
				break;

			case 'e':
				verbatim = 0;
				break;

			case '~':
				if (comment) comment = 0;
				else comment++;
				break;

			case '-':
				if (doinginst)
				{
					/* new instruction.  Make the hash 
					   table entry.
					*/
					makeinst(curinst,lasttemplindx);
					if (templindx != 0) gen0template();
					lasttemplindx=templindx;
				}
				doinginst++;
				temp = &buf[1];
				new = curinst;
				while (isspace(c = *temp)) temp++;
				while (isalnum(c = *temp))
				{
					*new++ = *temp++;
				}
				*new = 0;
				break;

			default:
				if (!doinginst)
					fatal("unrecognizable line");
				/* okay, make the template entry */
				gentemplate(buf,curinst);
				break;

		}
		continue;
		
		doverbatim:
			fputs(buf,hashfile);
			fputs(buf,templatefile);
		next: ;
	}
	if (doinginst)
	{
		/* new instruction.  Make the hash 
		   table entry.
		*/
		makeinst(curinst,lasttemplindx);
		gen0template();
		lasttemplindx=templindx;
	}
	fputs("};\n",hashfile);
	fputs("};\n",templatefile);

	fclose(hashfile);
	fclose(templatefile);
	fclose(inputfile);
	exit(0);
}

fatal(str)
char *str;
{
	fprintf(stderr,str);
	fclose(hashfile);
	fclose(templatefile);
	fclose(inputfile);
	exit(-1);
}
gen0template()
{
	fputs("\t{0,\t0,\t0,\t0,\t0,},\n",templatefile);
	templindx++;
}

gentemplate(buf,curinst)
char *buf;
char *curinst;
{
	char *index();
	char *temp;
	if (templindx == 0)
	{
		init_hash();
		init_template();
	}
	while (isspace(*buf)) buf++;
	temp = index(buf,'\n');
	if (temp != (char *)0) *temp = ' ';
	fprintf(templatefile,"\t{%s},\/* %s *\/\n",buf,curinst);
	templindx++;
}

makeinst(curinst,lasttemplindx)
char *curinst;
{
	fprintf(hashfile,"\t{\"%s\",   \t0,\t&template[%d],},\n",
		curinst,lasttemplindx);
}

init_hash()
{
	fprintf(hashfile,"inst_t base_inst[]  = \n{\n");
}

init_template()
{
	fprintf(templatefile,"struct template_s template[] = \n{\n");
}
