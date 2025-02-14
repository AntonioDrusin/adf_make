/*
 *  ADF Library. (C) 1997-1998 Laurent Clevy
 *
 *  adf_disk.c
 *
 * logical disk/volume code
 */

#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "adf_str.h"
#include "adf_disk.h"
#include "adf_raw.h"
#include "adf_hd.h"
#include "adf_bitm.h"
#include "adf_util.h"
#include "adf_nativ.h"
#include "adf_dump.h"
#include "adf_err.h"
#include "adf_cache.h"

extern struct Env adfEnv;

unsigned long bitMask[32] = { 
    0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
    0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000 };


RETCODE adfInstallBootBlock(struct Volume *vol, unsigned char* code)
{
    int i;
    struct bBootBlock boot;

    if (vol->dev->devType!=DEVTYPE_FLOPDD && vol->dev->devType!=DEVTYPE_FLOPHD)
        return RC_ERROR;

    if (adfReadBootBlock(vol, &boot)!=RC_OK)
        return RC_ERROR;

    boot.rootBlock = 880;
    for(i=0; i<1024-12; i++)         /* bootcode */
        boot.data[i] = code[i+12];

    if (adfWriteBootBlock(vol, &boot)!=RC_OK)
		return RC_ERROR;

	vol->bootCode = TRUE;

    return RC_OK;
}


/*
 * isSectNumValid
 *
 */
BOOL isSectNumValid(struct Volume *vol, SECTNUM nSect)
{
    return (vol->firstBlock<=nSect && nSect<=vol->lastBlock);
}	
	


/*
 * adfVolumeInfo
 *
 */
void adfVolumeInfo(struct Volume *vol)
{
	struct bRootBlock root;
	char diskName[35];
	int days,month,year;
	
	if (adfReadRootBlock(vol, vol->rootBlock, &root)!=RC_OK)
		return;
	
	memset(diskName, 0, 35);
	memcpy(diskName, root.diskName, root.nameLen);
	
	printf ("Name : %-30s\n",diskName);
	printf ("Type : ");
	switch(vol->dev->devType) {
		case DEVTYPE_FLOPDD:
			printf ("Floppy Double Density : 880 KBytes\n");
			break;
		case DEVTYPE_FLOPHD:
			printf ("Floppy High Density : 1760 KBytes\n");
			break;
		case DEVTYPE_HARDDISK:
			printf ("Hard Disk partition : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			break;
		case DEVTYPE_HARDFILE:
			printf ("HardFile : %3.1f KBytes\n", 
				(vol->lastBlock - vol->firstBlock +1) * 512.0/1024.0);
			break;
		default:
			printf ("Unknown devType!\n");
	}
	printf ("Filesystem : ");
	printf("%s ",isFFS(vol->dosType) ? "FFS" : "OFS");
	if (isINTL(vol->dosType))
		printf ("INTL ");
	if (isDIRCACHE(vol->dosType))
		printf ("DIRCACHE ");
	putchar('\n');

    printf("free blocks = %ld\n", adfCountFreeBlocks(vol));
    if (vol->readOnly)
        printf("Read only\n");
    else
        printf("Read/Write\n");
 	
    /* created */
	adfDays2Date(root.coDays, &year, &month, &days);
    printf ("created %d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.coMins/60,root.coMins%60,root.coTicks/50);	
	adfDays2Date(root.days, &year, &month, &days);
    printf ("last access %d/%02d/%02d %ld:%02ld:%02ld,   ",days,month,year,
	    root.mins/60,root.mins%60,root.ticks/50);	
	adfDays2Date(root.cDays, &year, &month, &days);
    printf ("%d/%02d/%02d %ld:%02ld:%02ld\n",days,month,year,
	    root.cMins/60,root.cMins%60,root.cTicks/50);	
}



/*
 * adfMount
 *
 * 
 */
struct Volume* adfMount( struct Device *dev, int nPart, BOOL readOnly )
{
    long nBlock;
    struct bRootBlock root;
	struct bBootBlock boot;
	struct Volume* vol;

    if (dev==NULL || nPart<nPart || nPart >= dev->nVol) {
        (*adfEnv.eFct)("adfMount : invalid parameter(s)");
        return NULL;
    }

    vol = dev->volList[nPart];
	vol->dev = dev;
    vol->mounted = TRUE;

/*printf("first=%ld last=%ld root=%ld\n",vol->firstBlock,
 vol->lastBlock, vol->rootBlock);
*/
    if (adfReadBootBlock(vol, &boot)!=RC_OK) {
        (*adfEnv.wFct)("adfMount : BootBlock invalid");
        return NULL;
    }       
    
	vol->dosType = boot.dosType[3];
	if (isFFS(vol->dosType))
		vol->datablockSize=512;
	else
		vol->datablockSize=488;

    if (dev->readOnly /*|| isDIRCACHE(vol->dosType)*/)
       vol->readOnly = TRUE;
    else
       vol->readOnly = readOnly;
	   	
	if (adfReadRootBlock(vol, vol->rootBlock, &root)!=RC_OK) {
        (*adfEnv.wFct)("adfMount : RootBlock invalid");       
        return NULL;
    }

    nBlock = vol->lastBlock - vol->firstBlock +1 - 2;

	adfReadBitmap( vol, nBlock, &root );
    vol->curDirPtr = vol->rootBlock;

    return( vol );
}


/*
*
* adfUnMount
*
* free bitmap structures
* free current dir
*/
void adfUnMount(struct Volume *vol)
{
	if (!vol) {
		(*adfEnv.eFct)("adfUnMount : vol is null");
        return;
    }

    adfFreeBitmap(vol);

    vol->mounted = FALSE;
	
}



/*
 * adfCreateVol
 *
 * 
 */
struct Volume* adfCreateVol( struct Device* dev, long start, long len, 
    char* volName, int volType )
{
    struct bBootBlock boot;
    struct bRootBlock root;
//    struct bDirCacheBlock dirc;
    SECTNUM blkList[2];
    struct Volume* vol;
    int nlen;

    vol=(struct Volume*)malloc(sizeof(struct Volume));
    if (!vol) { 
		(*adfEnv.eFct)("adfCreateVol : malloc vol");
        return NULL;
    }
	
    vol->dev = dev;
    vol->firstBlock = (dev->heads * dev->sectors)*start;
    vol->lastBlock = (vol->firstBlock + (dev->heads * dev->sectors)*len)-1;
    vol->rootBlock = (vol->lastBlock - vol->firstBlock+1)/2;
/*printf("first=%ld last=%ld root=%ld\n",vol->firstBlock,
 vol->lastBlock, vol->rootBlock);
*/
    vol->curDirPtr = vol->rootBlock;

    vol->readOnly = dev->readOnly;

    vol->mounted = TRUE;

    nlen = min( MAXNAMELEN, strlen(volName) );
    vol->volName = (char*)malloc(nlen+1);
    if (!vol->volName) { 
		(*adfEnv.eFct)("adfCreateVol : malloc");
		free(vol); return NULL;
    }
    memcpy(vol->volName, volName, nlen);
    vol->volName[nlen]='\0';

    memset(&boot, 0, 1024);
    boot.dosType[3] = volType;
/*printf("first=%d last=%d\n", vol->firstBlock, vol->lastBlock);
printf("name=%s root=%d\n", vol->volName, vol->rootBlock);
*/
    if (adfWriteBootBlock(vol, &boot)!=RC_OK) {
        free(vol->volName); free(vol);
        return NULL;
    }

    if (adfCreateBitmap( vol )!=RC_OK) {
        free(vol->volName); free(vol);
        return NULL;
    }
//puts("22");
/*for(i=0; i<127; i++)
printf("%3d %x, ",i,vol->bitmapTable[0]->map[i]);
*/
    if ( isDIRCACHE(volType) )
        adfGetFreeBlocks( vol, 2, blkList );
    else
        adfGetFreeBlocks( vol, 1, blkList );


/*printf("[0]=%d [1]=%d\n",blkList[0],blkList[1]);*/

    memset(&root, 0, LOGICAL_BLOCK_SIZE);

    if (strlen(volName)>MAXNAMELEN)
        volName[MAXNAMELEN]='\0';
    root.nameLen = strlen(volName);
    memcpy(root.diskName,volName,root.nameLen);
    adfTime2AmigaTime(adfGiveCurrentTime(),&(root.coDays),&(root.coMins),&(root.coTicks));

    /* dircache block */
    if ( isDIRCACHE(volType) ) {
//        vol->readOnly = TRUE;
        root.extension = 0L;
        root.secType = ST_ROOT; /* needed by adfCreateEmptyCache() */
        adfCreateEmptyCache(vol, (struct bEntryBlock*)&root, blkList[1]);
    }

    if (adfWriteRootBlock(vol, blkList[0], &root)!=RC_OK)
		return NULL;

   /* fills root->bmPages[] and writes filled bitmapExtBlocks */
    if (adfWriteNewBitmap(vol)!=RC_OK)
		return NULL;

    if (adfUpdateBitmap(vol)!=RC_OK)
		return NULL;

    /* will be managed by adfMount() later */
    adfFreeBitmap(vol);

    vol->mounted = FALSE;

    return(vol);
}


/*-----*/

/*
 * adfReadBlock
 *
 * read logical block
 */
RETCODE
adfReadBlock(struct Volume* vol, long nSect, unsigned char* buf)
{
  /*    char strBuf[80];*/
    long pSect;
    struct nativeFunctions *nFct;
    RETCODE rc;

    if (!vol->mounted) {
        (*adfEnv.eFct)("the volume isn't mounted, adfReadBlock not possible");
        return RC_ERROR;
    }

    /* translate logical sect to physical sect */
    pSect = nSect+vol->firstBlock;
//printf("psect=%ld nsect=%ld\n",pSect,nSect);
/*    sprintf(strBuf,"ReadBlock : accessing logical block #%ld", nSect);	
    (*adfEnv.vFct)(strBuf);
*/
    if (pSect<vol->firstBlock || pSect>vol->lastBlock) {
        (*adfEnv.wFct)("adfReadBlock : nSect out of range");
        
    }
//printf("pSect R =%ld\n",pSect);
    nFct = adfEnv.nativeFct;
    if (vol->dev->isNativeDev)
        rc = (*nFct->adfNativeReadSector)(vol->dev, pSect, 512, buf);
    else
        rc = adfReadDumpSector(vol->dev, pSect, 512, buf);
//printf("rc=%ld\n",rc);
    if (rc!=RC_OK)
        return RC_ERROR;
    else
        return RC_OK;
}


/*
 * adfWriteBlock
 *
 */
RETCODE adfWriteBlock(struct Volume* vol, long nSect, unsigned char *buf)
{
    long pSect;
    struct nativeFunctions *nFct;
    RETCODE rc;

    if (!vol->mounted) {
        (*adfEnv.eFct)("the volume isn't mounted, adfWriteBlock not possible");
        return RC_ERROR;
    }

    if (vol->readOnly) {
        (*adfEnv.wFct)("adfWriteBlock : can't write block, read only volume");
        return RC_ERROR;
    }

    pSect = nSect+vol->firstBlock;
//printf("write nsect=%ld psect=%ld\n",nSect,pSect);

    if (pSect<vol->firstBlock || pSect>vol->lastBlock) {
        (*adfEnv.wFct)("adfWriteBlock : nSect out of range");
    }

    nFct = adfEnv.nativeFct;
//printf("nativ=%d\n",vol->dev->isNativeDev);
    if (vol->dev->isNativeDev)
        rc = (*nFct->adfNativeWriteSector)(vol->dev, pSect, 512, buf);
    else
        rc = adfWriteDumpSector(vol->dev, pSect, 512, buf);

    if (rc!=RC_OK)
        return RC_ERROR;
    else
        return RC_OK;
}



/*#######################################################################################*/
