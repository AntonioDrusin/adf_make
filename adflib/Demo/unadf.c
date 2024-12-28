
/*
 * unadf
 *
 */

#define UNADF_VERSION "0.5"


#include<stdlib.h>
#include<errno.h>
#include<string.h>

#include "adflib.h"



void MyVer(char *msg)
{
}

void help()
{
    puts("unadf [-lrcs -v n] dumpname.adf");
    puts("    -l : lists root directory contents");
    puts("    -r : lists directory tree contents");
    puts("    -c : use dircache data (must be used with -l)");
    puts("    -s : display entries logical block pointer (must be used with -l)");
    puts("    -v n : mount volume #n instead of default #0 volume");
}

void printEnt(struct Volume *vol, struct Entry* entry, char *path, BOOL sect)
{
/*    long realSize=0;
    char *realName=NULL;
*/
    if (entry->type==ST_LFILE || entry->type==ST_LDIR || entry->type==ST_LSOFT)
        return;
    if (entry->type==ST_DIR)
        printf("         ");
    else
        printf("%7ld  ",entry->size);

	printf("%4d/%02d/%02d  %2d:%02d:%02d ",entry->year, entry->month, entry->days,
        entry->hour, entry->mins, entry->secs);
    if (sect)
        printf(" %06ld ",entry->sector);

    if (strlen(path)>0)
        printf(" %s/",path);
    else
        printf(" ");
    if (entry->type==ST_DIR)
        printf("%s/",entry->name);
    else
        printf("%s",entry->name);
    if (entry->comment!=NULL && strlen(entry->comment)>0)
        printf(", %s",entry->comment);
    putchar('\n');

}


void printTree(struct Volume *vol, struct List* tree, char* path, BOOL sect)
{
    char *buf;
    struct Entry* entry;

    while(tree) {
//printf("pathtree=%s\n",path);
        printEnt(vol, tree->content, path, sect);
        if (tree->subdir!=NULL) {
            entry = (struct Entry*)tree->content;
            if (strlen(path)>0) {
                buf=(char*)malloc(sizeof(strlen(path)+1+strlen(entry->name)+1));
                if (!buf) {
                    fprintf(stderr,"printTree : malloc error\n");
                    return;
                }
                sprintf(buf,"%s/%s", path, entry->name);
                printTree(vol, tree->subdir, buf, sect);
                free(buf);
            }
            else
                printTree(vol, tree->subdir, entry->name, sect);
        }
        tree = tree->next;
    }
}


void printDev(struct Device* dev)
{
    printf("Device : ");

    switch(dev->devType){
    case DEVTYPE_FLOPDD:
        printf("Floppy DD"); break;
    case DEVTYPE_FLOPHD:
        printf("Floppy HD"); break;
    case DEVTYPE_HARDDISK:
        printf("Harddisk"); break;
    case DEVTYPE_HARDFILE:
        printf("Hardfile"); break;
    default:
        printf("???"); break;
    }

    printf(". Cylinders = %ld, Heads = %ld, Sectors = %ld",dev->cylinders,dev->heads,dev->sectors);

    printf(". Volumes = %d\n",dev->nVol);
}


void printVol(struct Volume* vol, int volNum)
{
    printf("Volume : ");
	
    switch(vol->dev->devType) {
    case DEVTYPE_FLOPDD:
        printf ("Floppy 880 KBytes,");
        break;
    case DEVTYPE_FLOPHD:
        printf ("Floppy 1760 KBytes,");
        break;
    case DEVTYPE_HARDDISK:
        printf ("HD partition #%d %3.1f KBytes,", volNum, (vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
        break;
    case DEVTYPE_HARDFILE:
        printf ("HardFile %3.1f KBytes,", (vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
        break;
    default:
        printf ("???,");
    }

    printf(" between sectors [%ld-%ld].",vol->firstBlock, vol->lastBlock);

    printf(" %s ",isFFS(vol->dosType) ? "FFS" : "OFS");
    if (isINTL(vol->dosType))
        printf ("INTL ");
    if (isDIRCACHE(vol->dosType))
        printf ("DIRCACHE ");

    printf(". Filled at %2.1f%%.\n", 100.0-
	(adfCountFreeBlocks(vol)*100.0)/(vol->lastBlock - vol->firstBlock +1) );

}


int main(int argc, char* argv[])
{
    int i, j;
    BOOL rflag, lflag, xflag, cflag, vflag, sflag;
    char *filename, *devname;
    char strbuf[80];

    struct Device *dev;
    struct Volume *vol;
    struct List *list, *cell;
    int volNum;
    BOOL true = TRUE;
	
    if (argc<2) {
        help();
        exit(0);
    }

    rflag = lflag = xflag = cflag = vflag = sflag = FALSE;
    filename = "";
    devname = "";
    volNum = 0;

    printf("unADF v%s : a unzip like for .ADF files, powered by ADFlib (v%s - %s)\n", 
        UNADF_VERSION, adfGetVersionNumber(),adfGetVersionDate());

    /* parse options */
    i=1;
    while(i<argc) {
        if (argv[i][0]=='-')
            for (j=1; j<(int)strlen(argv[i]); j++)
                switch(argv[i][j]) {
                case 'v': 
                    vflag = TRUE;
                    if ((i+1)<(argc-1)) {
                        i++;
                        errno = 0;
                        volNum = atoi(argv[i]);
                        if (errno!=0 || volNum<0) {
                            fprintf(stderr,"invalid volume number, aborting.\n");
                            exit(1);
                        }
                    }
                    else
                        fprintf(stderr,"no volume number, -v option ignored.\n");
                    break;
                case 'l': 
                    lflag = TRUE;
                    break;
                case 's': 
                    sflag = TRUE;
                    break;
                case 'c': 
                    cflag = TRUE;
                    break;
                case 'r':
                    rflag = TRUE;
                    break;
                case 'x':
                    xflag = TRUE;
                    break;
                case 'h':
                default:
                    help();
                    exit(0);
                }
        else
			/* the last non option string is taken as the filename */
            if (i==(argc-1))
                if (strlen(devname)==0)
                    devname = argv[i];
                else
					filename = argv[i];
        
        i++;
    }

    putchar('\n');

    adfChgEnvProp(PR_VFCT, MyVer);

    /* initialize the library */
    adfEnvInitDefault();


    dev = adfMountDev( devname );
    if (!dev) {
        sprintf(strbuf,"Can't mount the dump device '%s'.\n", devname);
        fprintf(stderr, strbuf);
        adfEnvCleanUp(); exit(1);
    }
    printDev(dev);

    if (volNum>=dev->nVol) {
        fprintf(stderr,"This device has only %d volume(s), aborting.\n",dev->nVol);
        exit(1);
    }

    vol = adfMount(dev, volNum, TRUE);
    if (!vol) {
        adfUnMountDev(dev);
        fprintf(stderr, "Can't mount the volume\n");
        adfEnvCleanUp(); exit(1);
    }
    printVol(vol, volNum);

    putchar('\n');
    if (cflag && isDIRCACHE(vol->dosType)) {
        adfChgEnvProp(PR_USEDIRC,&true);
        puts("Using dir cache blocks.");
    }

    if (lflag) {
        if (!rflag) {
            cell = list = adfGetDirEnt(vol,vol->curDirPtr);
            while(cell) {
                printEnt(vol,cell->content,"", sflag);
                cell = cell->next;
            }
            adfFreeDirList(list);
        } else {
            cell = list = adfGetRDirEnt(vol,vol->curDirPtr,TRUE);
            printTree(vol,cell,"", sflag);
            adfFreeDirList(list);
        }
    }else if (xflag) {

    }
    else
        help();

    adfUnMount(vol);
    adfUnMountDev(dev);

    adfEnvCleanUp();

    return(0);
}
