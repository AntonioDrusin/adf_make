/*
 *  ADF Library. (C) 1997-1998 Laurent Clevy
 *
 *  adf_util.c
 *
 */

#include<stdlib.h>
#include<time.h>

#include "adf_util.h"
#include "adf_err.h"

extern struct Env adfEnv;


/*
 * swLong
 *
 * write an unsigned long value (val) (in) 
 * to an unsigned char* buffer (buf) (out)
 * 
 * used in adfWrite----Block() functions
 */
void swLong(unsigned char* buf, unsigned long val)
{
	buf[0]= (unsigned char)((val & 0xff000000) >>24UL);
	buf[1]= (unsigned char)((val & 0x00ff0000) >>16UL);
	buf[2]= (unsigned char)((val & 0x0000ff00) >>8UL);
	buf[3]= (unsigned char)(val & 0x000000ff);
}

void swShort(unsigned char* buf, unsigned short val)
{
	buf[0]= (val & 0xff00) >>8UL;
	buf[1]= (val & 0x00ff) ;
}

/*
 * newCell
 *
 * adds a cell at the end the list
 */
struct List* newCell(struct List* list, void* content)
{
    struct List* cell;

    cell=(struct List*)malloc(sizeof(struct List));
    if (!cell) {
        (*adfEnv.eFct)("newCell : malloc");
        return NULL;
    }
    cell->content = content;
    cell->next = cell->subdir = 0;
    if (list!=NULL)
        list->next = cell;

    return cell;
}


/*
 * freeList
 *
 */
void freeList(struct List* list)
{
    if (list==NULL) 
        return;
    
    if (list->next)
        freeList(list->next);
    free(list);
}




/*
 * Days2Date
 *
 * amiga disk date format (days) to normal dd/mm/yy format (out)
 */

void 
adfDays2Date(long days, int *yy, int *mm, int *dd)
{
    int y,m;
    int nd;
    int jm[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    /* 0 = 1 Jan 1978,  6988 = 18 feb 1997 */

    /*--- year ---*/
    y=1978;
    if (adfIsLeap(y))
        nd=366;
    else
        nd=365;
    while( days >= nd ) {
        days-=nd;
        y++;
        if (adfIsLeap(y))
            nd=366;
        else
            nd=365;
    }


    /*--- month ---*/
    m=1;
    if (adfIsLeap(y))
        jm[2-1]=29;
    while( days >= jm[m-1] ) {
        days-=jm[m-1];
        m++;
    }

    *yy=y;
    *mm=m;
    *dd=days+1;
}


/*
 * IsLeap
 *
 * true if a year (y) is leap
 */

    BOOL 
adfIsLeap(int y)
{
    return( (BOOL) ( !(y%100) ? !(y%400) : !(y%4) ) );
}


/*
 * adfCurrentDateTime
 *
 * return the current system date and time
 */
    struct DateTime
adfGiveCurrentTime( void )
{
    struct tm *local;
    time_t cal;
    struct DateTime r;

    time(&cal);
    local=localtime(&cal);

    r.year=local->tm_year;         /* since 1900 */
    r.mon=local->tm_mon+1;
    r.day=local->tm_mday;
    r.hour=local->tm_hour;
    r.min=local->tm_min;
    r.sec=local->tm_sec;

    return(r);
}


/*
 * adfTime2AmigaTime
 *
 * converts date and time (dt) into Amiga format : day, min, ticks
 */
    void
adfTime2AmigaTime(struct DateTime dt, long *day, long *min, long *ticks )
{
    int jm[12]={ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


    *min= dt.hour*60 + dt.min;                /* mins */
    *ticks= dt.sec*50;                        /* ticks */

    /*--- days ---*/

    *day= dt.day-1;                         /* current month days */

    /* previous months days downto january */
    if (dt.mon>1) {                      /* if previous month exists */
        dt.mon--;
        if (dt.mon>2 && adfIsLeap(dt.year))    /* months after a leap february */
            jm[2-1]=29;
        while(dt.mon>0) {
            *day=*day+jm[dt.mon-1];
            dt.mon--;
        }
    }

    /* years days before current year downto 1978 */
    if (dt.year>78) {
        dt.year--;
        while(dt.year>=78) {
            if (adfIsLeap(dt.year))
                *day=*day+366;
            else
                *day=*day+365;
            dt.year--;
        }
    }
}


/*
 * dumpBlock
 *
 * debug function : to dump a block before writing the check its contents
 *
 */
void dumpBlock(unsigned char *buf)
{
	int i, j;

	for(i=0; i<32; i++) {
		printf("%5x ",i*16);
        for (j=0; j<4; j++) {
			printf("%08x ",Long(buf+j*4+i*16));
		}
        printf("    ");
        for (j=0; j<16; j++)
            if (buf[i*16+j]<32 || buf[i*16+j]>127)
                putchar('.');
            else
                putchar(buf[i*16+j]);
        putchar('\n');
    }
}



/*################################################################################*/
