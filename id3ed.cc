#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
//apparantly some systems, such as cygwin, do not have getopt defined in unistd
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_LIBREADLINE
#ifdef OLD_READLINE
extern "C" {
#endif
#include <readline/readline.h>
#include <readline/history.h>
#ifdef OLD_READLINE
}
#endif
#ifdef HAVE_LIBTERMCAP
#include <termcap.h>
#endif
#endif //HAVE_LIBREADLINE
#ifdef HAVE_LIBCURSES
#include <curses.h>
//stupid curses macros.
#undef erase
#undef clear
#endif
#include "misc.h"

/* Field Length offsets
 *  Tag 3 0-2
 *  Songname 30 3-32
 *  Artist 30 33-62
 *  Album 30 63-92
 *  Year 4 93-96
 *  Comment 30 97-126
 *  or{
 *    Comment 28 97-124
 *    zero 1 125
 *    Tracknum 1 126
 *  }
 *  Genre 1 127
 */
struct s_v11sub {
	char comment[28];
	unsigned char empty;
	unsigned char tracknum;
};
struct s_id3{
	char tag[3];
	char songname[30];
	char artist[30];
	char album[30];
	char year[4];
	union{
		char comment[30];
		s_v11sub v11;
	};
	//char comment[28];
	//unsigned char empty;
	//unsigned char tracknum;
	unsigned char genre;
}id3;

#define NUM_GENRE 148

#define QUIET_PROGRESS	(1 << 0) /* 'File ...' output */
#define QUIET_SONG	(1 << 1) /* s */
#define QUIET_ARTIST	(1 << 2) /* n */
#define QUIET_ALBUM	(1 << 3) /* a */
#define QUIET_YEAR	(1 << 4) /* y */
#define QUIET_COMMENT	(1 << 5) /* c */
#define QUIET_GENRE	(1 << 6) /* g */
#define QUIET_TRACK	(1 << 7) /* k */


const char *genre_list[NUM_GENRE+1]={
	"Blues", "Classic Rock", "Country", "Dance", "Disco", "Funk",
	"Grunge", "Hip-Hop", "Jazz", "Metal", "New Age", "Oldies",
	"Other", "Pop", "R&B", "Rap", "Reggae", "Rock",
	"Techno", "Industrial", "Alternative", "Ska", "Death Metal", "Pranks",
	"Soundtrack", "Euro-Techno", "Ambient", "Trip-Hop", "Vocal", "Jazz+Funk",
	"Fusion", "Trance", "Classical", "Instrumental", "Acid", "House",
	"Game", "Sound Clip", "Gospel", "Noise", "AlternRock", "Bass",
	"Soul", "Punk", "Space", "Meditative", "Instrumental Pop", "Instrumental Rock",
	"Ethnic", "Gothic", "Darkwave", "Techno-Industrial", "Electronic", "Pop-Folk",
	"Eurodance", "Dream", "Southern Rock", "Comedy", "Cult", "Gangsta",
	"Top 40", "Christian Rap", "Pop/Funk", "Jungle", "Native American", "Cabaret",
	"New Wave", "Psychadelic", "Rave", "Showtunes", "Trailer", "Lo-Fi",
	"Tribal", "Acid Punk", "Acid Jazz", "Polka", "Retro", "Musical",
	"Rock & Roll", "Hard Rock", "Folk", "Folk/Rock", "National Folk", "Swing",
	"Fast-Fusion", "Bebob", "Latin", "Revival", "Celtic", "Bluegrass", "Avantgarde",
	"Gothic Rock", "Progressive Rock", "Psychedelic Rock", "Symphonic Rock", "Slow Rock", "Big Band",
	"Chorus", "Easy Listening", "Acoustic", "Humour", "Speech", "Chanson",
	"Opera", "Chamber Music", "Sonata", "Symphony", "Booty Bass", "Primus",
	"Porn Groove", "Satire", "Slow Jam", "Club", "Tango", "Samba",
	"Folklore", "Ballad", "Power Ballad", "Rhythmic Soul", "Freestyle", "Duet",
	"Punk Rock", "Drum Solo", "A capella", "Euro-House", "Dance Hall",
	"Goa", "Drum & Bass", "Club House", "Hardcore", "Terror",
	"Indie", "BritPop", "NegerPunk", "Polsk Punk", "Beat",
	"Christian Gangsta", "Heavy Metal", "Black Metal", "Crossover", "Contemporary Christian",
	"Christian Rock", "Merengue", "Salsa", "Thrash Metal", "Anime", "JPop",
	"SynthPop",
	NULL
};

void i3info(const char *file,int quiet);
void i3remove(const char *file,int quiet);
void i3rename(const char *file,int quiet,int test);
void i3edit(const char * file, const char * defsongname,
		const char * defartist, const char * defalbum,
		const char * defyear, const char * defcomment,
		const char * deftracknum, const char * defgenre,int quiet);
void stredit(const char * name, int maxlen, char * buf);
void print_genre_list(int mode);

void version()
{
	printf("id3ed v1.10.4 - mpeg layer 3 file information editor\n");
}

void license()
{
	printf("Copyright 1998-2001,2003 Matthew Mueller <donut@azstarnet.com>\n");
	printf("\n\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\n\
You should have received a copy of the GNU General Public License\n\
along with this program; if not, write to the Free Software\n\
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n");
}

void print_help(void){
	version();
	printf("\
Usage: id3ed [-s songname] [-n artist] [-a album] [-y year] [-c comment]\n\
	     [-k tracknum] [-g genre] [-q] [-SNAYCKG] [-l/-L] [-r]\n\
	     [-i] <mp3files> [-v]\n\n\
  -q			no line interface; only set tags specified on command\n\
  			line. Use twice to suppress all output except errors.\n\
  -SNAYCKG		prompt to edit the specified tags only\n\
  			Other tags can still be set with the -[snaycg] options.\n\
  -l/-L			display list of genres\n\
  -r			remove id3 tag from files\n\
  -i			show current id3 tag only, don't edit\n\
  -v			output program version/license\n");
}

int main(int argc,char ** argv){
	int c,quiet=0,alreadyquiet=0,mode=0;
	char *songname = NULL, *artist = NULL, *album = NULL, *year = NULL,
	*comment = NULL, *tracknum = NULL, *genre = NULL;

	if (argc<2){
		print_help();
	}
	else {
		while (EOF != (c = getopt(argc, argv, "s:n:a:y:c:k:g:qSNAYCKGlLrimMv"))){
			switch (c){
				case 's':
					songname = optarg;
					break;
				case 'n':
					artist = optarg;
					break;
				case 'a':
					album = optarg;
					break;
				case 'y':
					year = optarg;
					break;
				case 'c':
					comment = optarg;
					break;
				case 'g':
					genre = optarg;
					break;
				case 'k':
					tracknum = optarg;
					break;
				case 'q':
#define REALQUIET() {if (!alreadyquiet) { quiet = 0xffff & ~QUIET_PROGRESS; alreadyquiet=1;}}
#define UNQUIET(b) { REALQUIET(); quiet &= ~b;}
					if(alreadyquiet)
						quiet |= QUIET_PROGRESS;
					else {
						REALQUIET();
					}
					break;
				case 'S':
					UNQUIET(QUIET_SONG);
					break;
				case 'N':
					UNQUIET(QUIET_ARTIST);
					break;
				case 'A':
					UNQUIET(QUIET_ALBUM);
					break;
				case 'Y':
					UNQUIET(QUIET_YEAR);
					break;
				case 'C':
					UNQUIET(QUIET_COMMENT);
					break;
				case 'G':
					UNQUIET(QUIET_GENRE);
					break;
				case 'K':
					UNQUIET(QUIET_TRACK);
					break;
				case 'l':
					print_genre_list(0);
					break;
				case 'L':
					print_genre_list(1);
					break;
				case 'r':
					mode=1;
					break;
				case 'i':
					mode=2;
					break;
				case 'v':
					version();
					license();
					break;
				case ':':
				case '?':
					//fprintf(stderr, "Illegal parameters\n");
					print_help();
					return 1;
				case 'm':
					mode=3;
					break;
				case 'M':
					mode=4;
					break;
			}
		}
		//ntf("quiet=%i alreadyquiet=%i\n",quiet,alreadyquiet);
		for (int i=optind;i<argc;i++)
			switch (mode){
				case 3:
				case 4:
					i3rename(argv[i],quiet,mode==3);
					break;
				case 2:
					i3info(argv[i],quiet);
					break;
				case 1:
					i3remove(argv[i],quiet);
					break;
				default:
					i3edit(argv[i],songname,artist,album,year,comment,tracknum,genre,quiet);
			}
	}
	return 0;
}

void print_genre_list(int mode){
	int c=0,w=0;
#ifdef HAVE_LIBCURSES
	c=COLS;
#endif
	if (c<=0)c=80;
	w=(c/26);
	if (w<1)w=1;
//	printf("c=%i w=%i\n",c,w);
	//for (int x=0, y=0, i=0;genre_list[i];mode?i++:(i=y+x*((NUM_GENRE+w-1)/w))){
	for (int x=0, y=0, i=0;genre_list[i];mode?i++:(i=y+x*((NUM_GENRE)/w))){
		printf("%3i: %-20s",i,genre_list[i]);
		//			       printf("x%i,y%i,i%i\n",x,y,i);
		if (x>=w-1){
			x=0;y++;
		}else{
			x++;
		}
		if (x==0){
			if (i==NUM_GENRE-1)
				break;
			printf("\n");
		}
		else
			printf(" ");
	}
	printf("\n");
}

int agenretoi(const char * g){
	int cmpl=g?strlen(g):0;
	while (cmpl>0 && isspace(g[cmpl-1]))
		--cmpl;//ignore trailing spaces
	if (cmpl>0){
		int i;
		char * errpos;
		for(i=0;i<NUM_GENRE;i++)
			if(strncasecmp(genre_list[i],g,cmpl)==0)
				return i;
		i=strtol(g,&errpos,0);
		if (!*errpos)
			return i;
	}
	printf("'%s' is not a valid genre name, nor a number between 0 and 255\n",g);
	return -1;
}

#ifdef HAVE_LIBREADLINE
char *my_rl_default;

int set_rl_default(void){
	rl_insert_text(my_rl_default);
	//rl_forced_update_display();
	return 0;
}
#ifdef HAVE_SET_H
#include <set.h>
struct ltstr{  bool operator()(const char* s1, const char* s2) const {return strcasecmp(s1, s2) < 0;}  };
const char *genre_generator(const char *text,int state){
	static set<const char*, ltstr> matches;
	static set<const char*, ltstr>::iterator curm=matches.end();
	const char *ret;
	if (state==0){
		int i;
		int l=strlen(text);
		matches.erase(matches.begin(),matches.end());
		for (i=0;i<NUM_GENRE;i++)
			if (strncasecmp(genre_list[i],text,l)==0)
				matches.insert(genre_list[i]);
		curm=matches.begin();
	}
	if (curm==matches.end())return NULL;
	ret=(*curm);
	++curm;
	return strdup(ret);//return must be malloc()'d
}
#endif //HAVE_SET_H
#endif //HAVE_LIBREADLINE
void genreedit(unsigned char &d){
	char def[40];
	int g=-1;
	if (d<NUM_GENRE)
		strcpy(def,genre_list[d]);
	else
		sprintf(def,"%i",d);
#ifdef HAVE_LIBREADLINE
	char *str=NULL;
	my_rl_default=def;
	rl_startup_hook=(Function*)set_rl_default;
#ifdef HAVE_SET_H
	(const char *(*)(const char *,int))rl_completion_entry_function=genre_generator;
#endif
//	rl_attempted_completion_function = (CPPFunction *)fileman_completion;
#else
	char str[40];
#endif

	do {
#ifdef HAVE_LIBREADLINE
		if (str){strcpy(def,str);free(str);}
		if((str=readline("genre[0-255/name]: "))){
			if (*str){
				add_history(str);
			}
		}
#else
		printf("genre[0-255/name def:%s]: ",def);
		fflush(stdout);
		if (fgets(str,40,stdin)){
			char *t;
			if ((t=strchr(str,'\n')))
				t[0]=0;
		}
		if(!*str) {
			// Blank; accept default.
			g = d;
			break;
		}

#endif
		if(str) g = agenretoi(str);
	} while (g == -1);

#ifdef HAVE_LIBREADLINE
	free(str);
	rl_completion_entry_function=NULL;//use normal completer again.
#endif
	d=g;
}
void stredit(const char * name, int maxlen, char * buf){
	char def[40];
	strncpy(def,buf,maxlen);
	def[maxlen]=0;
#ifdef HAVE_LIBREADLINE
	char prompt[40];
	char *str;
	sprintf(prompt,"%s[max:%i]: ",name,maxlen);
	my_rl_default=def;
	rl_startup_hook=(Function*)set_rl_default;
	if((str=readline(prompt))){
		strncpy(buf,str,maxlen);
		if (*str){
			add_history(str);
		}
		free(str);
	}
#else
	char str[40];
	printf("%s[max:%i def:%s]: ",name,maxlen,def);
	fflush(stdout);
	if (fgets(str,40,stdin)){
		char *t;
		if ((t=strchr(str,'\n')))
			t[0]=0;
		if (str[0])
			strncpy(buf,str,maxlen);
	}else if (ferror(stdin))
		perror("fgets");
	else
		strncpy(buf,"",maxlen);
#endif
}

//##### TODO: make renaming configurable?
void i3rename(const char *file,int quiet,int test){
	int f;
	if (doopen(f,file,RDMODE))return;
	if ((!quiet) || test){
		printf("%s->",file);fflush(stdout);
	}
	off_t end=lseek(f,0,SEEK_END);
	if (end>=0 && end<128) {
		close (f);
		if ((!quiet))
			printf("(no tag)\n");
		return;
	}
	if (lseek(f,-128,SEEK_END)<0){
		close (f);
		perror("lseek");return;
	}
	if (doread(f,&id3,sizeof(id3),"id3buf"))return;
	close (f);
	if (!strncmp(id3.tag,"TAG",3)){
		char newname[256];
		int track=0;
		if (id3.v11.empty==0)
			track=id3.v11.tracknum;
		sprintf(newname,"%02i-%.*s.mp3",track,(int)sizeof(id3.songname),id3.songname);//##### TODO: handle directories in file, handle invalid chars in songname
		if ((!quiet) || test){
			printf("%s\n",newname);
		}
		if (!test){
			if (rename(file,newname))
				perror("rename");return;
		}
	}else
		if ((!quiet))
			printf("(no tag)\n");
}

void i3info(const char *file,int quiet){
	int f;
	if (doopen(f,file,RDMODE))return;
	printf("%s: ",file);fflush(stdout);
	off_t end=lseek(f,0,SEEK_END);
	if (end>=0 && end<128) {
		close (f);
		if ((!quiet))
			printf("(no tag)\n");
		return;
	}
	if (lseek(f,-128,SEEK_END)<0){
		close (f);
		perror("lseek");return;
	}
	if (doread(f,&id3,sizeof(id3),"id3buf"))return;
	if (!strncmp(id3.tag,"TAG",3)){
		if (id3.v11.empty==0)
			printf("(tag v1.1%s)\n",id3.v11.tracknum?"":"?");
		else
			printf("(tag v1.0)\n");
#define PRINTINFO(f) printf("%s: %.*s\n",#f, (int)sizeof(id3.f), id3.f)
		PRINTINFO(songname);
		PRINTINFO(artist);
		PRINTINFO(album);
		PRINTINFO(year);
		PRINTINFO(comment);
		if (id3.v11.empty==0)
			printf("tracknum: %i\n", id3.v11.tracknum);
		printf("genre: %s(%i)\n\n",(id3.genre<NUM_GENRE)?genre_list[id3.genre]:"unknown",id3.genre);

	}else
		if ((!quiet))
			printf("(no tag)\n");

	close (f);
}

void i3remove(const char *file,int quiet){
	int f;
	if (doopen(f,file,RDWRMODE))return;
	off_t end=lseek(f,0,SEEK_END);
	if (end>=0 && end<128) {
		close (f);
		if ((!quiet))
			printf("no tag in %s\n",file);
		return;
	}
	if ((end=lseek(f,-128,SEEK_END))<0){
		close (f);
		perror("lseek");return;
	}
	if (doread(f,&id3,sizeof(id3),"id3buf"))return;
	if (!strncmp(id3.tag,"TAG",3)){
		ftruncate(f,end);
		if ((!quiet))
			printf("tag removed from %s\n",file);
	}else
		if ((!quiet))
			printf("no tag in %s\n",file);

	close (f);
}

void i3edit(const char * file, const char * defsongname,
		const char * defartist, const char * defalbum,
		const char * defyear, const char * defcomment,
		const char * deftracknum, const char * defgenre, int quiet){
	int f,hasid3,commentsize=28;
	if (!(quiet&QUIET_PROGRESS))
		printf("\nFile %s: ", file);
	if (doopen(f,file,RDWRMODE))return;
	off_t end=lseek(f,0,SEEK_END);
	if (end>=0 && end<128) {
		memset(&id3,0,sizeof(id3));
		id3.tag[0]='T';id3.tag[1]='A';id3.tag[2]='G';
		id3.genre=255;
		hasid3=0;
		if (!(quiet&QUIET_PROGRESS))
			printf("(no tag)\n");
	} else {
		if (lseek(f,-128,SEEK_END)<0){
			close(f);
			perror("lseek");return;
		}
		if (doread(f,&id3,sizeof(id3),"id3buf"))return;
		if (strncmp(id3.tag,"TAG",3)){
			memset(&id3,0,sizeof(id3));
			id3.tag[0]='T';id3.tag[1]='A';id3.tag[2]='G';
			id3.genre=255;
			hasid3=0;
			if (!(quiet&QUIET_PROGRESS))
				printf("(no tag)\n");
		} else {
			if (id3.v11.empty==0){
				if (!(quiet&QUIET_PROGRESS))
					printf("(tag v1.1)\n");
				hasid3=2;//id3 v1.1
			}else{
				if (!(quiet&1))
					printf("(tag v1.0");
				if (deftracknum){
					hasid3=2;
					if (!(quiet&QUIET_PROGRESS))
						printf("->1.1)\n");
				}else{
					hasid3=1;//id3 v1.0
					commentsize=30;
					if (!(quiet&QUIET_PROGRESS))
						printf(")\n");
				}
			}
		}
	}
	if (defsongname){
		memset(id3.songname,0,sizeof(id3.songname));
		strncpy(id3.songname,defsongname,sizeof(id3.songname));
	}
	if (defartist){
		memset(id3.artist,0,sizeof(id3.artist));
		strncpy(id3.artist,defartist,sizeof(id3.artist));
	}
	if (defalbum){
		memset(id3.album,0,sizeof(id3.album));
		strncpy(id3.album,defalbum,sizeof(id3.album));
	}
	if (defyear){
		memset(id3.year,0,sizeof(id3.year));
		strncpy(id3.year,defyear,sizeof(id3.year));
	}
	if (defcomment){
		memset(id3.comment,0,commentsize);
		strncpy(id3.comment,defcomment,commentsize);
	}
	if (deftracknum){
		//memset(id3.tracknum,0,sizeof(id3.tracknum));
		id3.v11.empty=0;
		id3.v11.tracknum = atoi(deftracknum);
		//strncpy(id3.tracknum,deftracknum,sizeof(id3.tracknum));
	}
	if (defgenre){
		id3.genre = agenretoi(defgenre);
		//unsigned tmp;
		//sscanf(defgenre,"%u",&tmp);
		//id3.genre = tmp;
	}
	if (!(quiet&QUIET_SONG))
		stredit("songname",30,id3.songname);
	if (!(quiet&QUIET_ARTIST))
		stredit("artist",30,id3.artist);
	if (!(quiet&QUIET_ALBUM))
		stredit("album",30,id3.album);
	if (!(quiet&QUIET_YEAR))
		stredit("year",4,id3.year);
	if (!(quiet&QUIET_COMMENT))
		stredit("comment",commentsize,id3.comment);
	if (hasid3!=1 && !(quiet&QUIET_TRACK)) {
		char tbuf[3];
		sprintf(tbuf, "%i", id3.v11.tracknum);
		stredit("tracknum", 3, tbuf);
		id3.v11.empty=0;
		id3.v11.tracknum=atoi(tbuf);
	}
	if (!(quiet&QUIET_GENRE)){
		//char sbuf[4];
		//sprintf(sbuf,"%i",id3.genre);
		//stredit("genre",3,sbuf);
		//id3.genre=atoi(sbuf);
		genreedit(id3.genre);
	}
	if (hasid3){
		if (lseek(f,-128,SEEK_END)<0){
			perror("lseek");return;
		}
	}else{
		if (lseek(f,0,SEEK_END)<0){
			perror("lseek");return;
		}
	}
	if (dowrite(f,&id3,sizeof(id3),"id3buf"))return;
	close(f);
}
