/*
 * mlabel.c
 * Make an MSDOS volume label
 */

#include "sysincludes.h"
#include "msdos.h"
#include "mainloop.h"
#include "vfat.h"
#include "mtools.h"
#include "nameclash.h"

struct OldDos_t old_dos[]={
{   40,  9,  1, 4, 1, 2, 0xfc },
{   40,  9,  2, 7, 2, 2, 0xfd },
{   40,  8,  1, 4, 1, 1, 0xfe },
{   40,  8,  2, 7, 2, 1, 0xff },
{   80,  9,  2, 7, 2, 3, 0xf9 },
{   80, 15,  2,14, 1, 7, 0xf9 },
{   80, 18,  2,14, 1, 9, 0xf0 },
{   80, 36,  2,15, 2, 9, 0xf0 },
{    1,  8,  1, 1, 1, 1, 0xf0 },
};

void init_clash_handling(ClashHandling_t *ch)
{
	ch->ignore_entry = -1;
	ch->source_entry = -2;
	ch->nowarn = 0;	/*Don't ask, just do default action if name collision */
	ch->namematch_default[0] = NAMEMATCH_AUTORENAME;
	ch->namematch_default[1] = NAMEMATCH_NONE;
	ch->name_converter = dos_name; /* changed by mlabel */
	ch->source = -2;
}

char *label_name(char *filename, int verbose, 
		 int *mangled, char *ans)
{
	int len;
	int i;
	int have_lower, have_upper;

	strcpy(ans,"           ");
	len = strlen(filename);
	if(len > 11){
		*mangled = 1;
		len = 11;
	} else
		*mangled = 0;
	strncpy(ans, filename, len);
	have_lower = have_upper = 0;
	for(i=0; i<11; i++){
		if(islower((unsigned char)ans[i]))
			have_lower = 1;
		if(isupper(ans[i]))
			have_upper = 1;
		ans[i] = toupper((unsigned char)ans[i]);

		if(strchr("^+=/[]:,?*\\<>|\".", ans[i])){
			*mangled = 1;
			ans[i] = '~';
		}
	}
	if (have_lower && have_upper)
		*mangled = 1;
	return ans;
}

int labelit(char *dosname,
	    char *longname,
	    void *arg0,
	    direntry_t *entry)
{
	time_t now;

	/* find out current time */
	getTimeNow(&now);
	mk_entry(dosname, 0x8, 0, 0, now, &entry->dir);
	return 0;
}

static void usage(void)
{
	fprintf(stderr, "Mtools version %s, dated %s\n",
		mversion, mdate);
	fprintf(stderr, "Usage: %s [-vscVn] [-N serial] drive:\n", progname);
	exit(1);
}



void mlabel(int argc, char **argv, int type)
{
    
	char *newLabel;
	int verbose, clear, interactive, show;
	direntry_t entry;
	int result=0;
	char longname[VBUFSIZE];
	char shortname[13];
	ClashHandling_t ch;
	struct MainParam_t mp;
	Stream_t *RootDir;
	int c;
	int mangled;
	enum { SER_NONE, SER_RANDOM, SER_SET }  set_serial = SER_NONE;
	long serial = 0;
	int need_write_boot = 0;
	int have_boot = 0;
	char *eptr;
	struct bootsector boot;
	Stream_t *Fs=0;
	int r;
	struct label_blk_t *labelBlock;
	int isRo=0;
	int *isRop=NULL;

	init_clash_handling(&ch);
	ch.name_converter = label_name;
	ch.ignore_entry = -2;

	verbose = 0;
	clear = 0;
	show = 0;

	memset(shortname, 0, sizeof(shortname));

	while ((c = getopt(argc, argv, "i:vcsnN:")) != EOF) {
		switch (c) {
			case 'i':
				set_cmd_line_image(optarg, 0);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'c':
				clear = 1;
				break;
			case 's':
				show = 1;
				break;
			case 'n':
				set_serial = SER_RANDOM;
				srandom((long)time (0));
				serial=random();
				break;
			case 'N':
				set_serial = SER_SET;
				serial = strtol(optarg, &eptr, 16);
				if(*eptr) {
					fprintf(stderr,
						"%s not a valid serial number\n",
						optarg);
					exit(1);
				}
				break;
			default:
				usage();
			}
	}

	if (argc - optind != 1 || !argv[optind][0] || argv[optind][1] != ':') 
		usage();

	init_mp(&mp);
	newLabel = argv[optind]+2;
	if(strlen(newLabel) > VBUFSIZE) {
		fprintf(stderr, "Label too long\n");
		FREE(&RootDir);
		exit(1);
	}

	interactive = !show && !clear &&!newLabel[0] && 
		(set_serial == SER_NONE);
	if(!clear && !newLabel[0]) {
		isRop = &isRo;
	}
	RootDir = open_root_dir(argv[optind][0], isRop ? 0 : O_RDWR, isRop);
	if(isRo) {
		show = 1;
		interactive = 0;
	}	    
	if(!RootDir) {
		fprintf(stderr, "%s: Cannot initialize drive\n", argv[0]);
		exit(1);
	}

	initializeDirentry(&entry, RootDir);
	r=vfat_lookup(&entry, 0, 0, ACCEPT_LABEL | MATCH_ANY,
		      shortname, longname);
	if (r == -2) {
		FREE(&RootDir);
		exit(1);
	}

	if(show || interactive){
		if(isNotFound(&entry)){;}
		else {
			int i;
			for(i=sizeof(shortname)-2;i>=0;i--) {
				if(shortname[i]==' ') {
					shortname[i] = '\0';
				}
				if(shortname[i]=='\0')
					continue;
				break;
			}
			fprintf(stdout,"%s",shortname);
			fflush(stdout);
		}
	}

	FREE(&RootDir);
	exit(result);
}
