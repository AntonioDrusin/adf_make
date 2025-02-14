/*
 * ADF Library
 *
 * adf_dump.c
 *
 * Amiga Dump File specific routines
 */

#include<stdio.h>
#include<stdlib.h>

#include"adf_defs.h"
#include"adf_str.h"
#include"adf_disk.h"
#include"adf_nativ.h"
#include"adf_err.h"

extern struct Env adfEnv;

/*
 * adfInitDumpDevice
 *
 */
RETCODE adfInitDumpDevice(struct Device* dev, char* name)
{
    struct nativeDevice* nDev;
    long size;

    nDev = (struct nativeDevice*)dev->nativeDev;

    nDev = (struct nativeDevice*)malloc(sizeof(struct nativeDevice));
    if (!nDev) {
        (*adfEnv.eFct)("adfInitDumpDevice : malloc");
        return RC_MALLOC;
    }
    dev->nativeDev = nDev;

    nDev->fd = fopen(name,"rb+");
    if (!nDev->fd) {
        free(nDev);
        (*adfEnv.eFct)("myInitDevice : fopen");
        return RC_ERROR;
    }

    /* determines size */
    fseek(nDev->fd, 0, SEEK_END);
	size = ftell(nDev->fd);
    fseek(nDev->fd, 0, SEEK_SET);

    dev->size = size;
	
    return RC_OK;
}


/*
 * adfReadDumpSector
 *
 */
RETCODE adfReadDumpSector(struct Device *dev, long n, int size, unsigned char* buf)
{
    struct nativeDevice* nDev;
    int r;
//puts("adfReadDumpSector");
    nDev = (struct nativeDevice*)dev->nativeDev;
    r = fseek(nDev->fd, 512*n, SEEK_SET);
//printf("nnn=%ld size=%d\n",n,size);
    if (r==-1)
        return RC_ERROR;
//puts("123");
    if ((r=fread(buf, 1, size, nDev->fd))!=size) {
//printf("rr=%d\n",r);
        return RC_ERROR;
}
//puts("1234");

    return RC_OK;
}


/*
 * adfWriteDumpSector
 *
 */
RETCODE adfWriteDumpSector(struct Device *dev, long n, int size, unsigned char* buf)
{
    struct nativeDevice* nDev;
    int r;

    nDev = (struct nativeDevice*)dev->nativeDev;

    r=fseek(nDev->fd, 512*n, SEEK_SET);
    if (r==-1)
        return RC_ERROR;

    if ( fwrite(buf, 1, size, nDev->fd)!=(unsigned int)(size) )
        return RC_ERROR;
//puts("adfWriteDumpSector");
    return RC_OK;
}


/*
 * adfReleaseDumpDevice
 *
 */
RETCODE adfReleaseDumpDevice(struct Device *dev)
{
    struct nativeDevice* nDev;

    if (!dev->nativeDev)
		return RC_ERROR;

    nDev = (struct nativeDevice*)dev->nativeDev;
    fclose(nDev->fd);

    free(nDev);

    return RC_OK;
}


/*
 * adfCreateHdFile
 *
 */
RETCODE adfCreateHdFile(struct Device* dev, char* volName, int volType)
{
	
    if (dev==NULL) {
        (*adfEnv.eFct)("adfCreateHdFile : dev==NULL");
        return RC_ERROR;
    }
    dev->volList =(struct Volume**) malloc(sizeof(struct Volume*));
    if (!dev->volList) { 
                (*adfEnv.eFct)("adfCreateHdFile : unknown device type");
        return RC_ERROR;
    }

    dev->volList[0] = adfCreateVol( dev, 0L, (long)dev->cylinders, volName, volType );
    if (dev->volList[0]==NULL) {
        free(dev->volList);
        return RC_ERROR;
    }

    dev->nVol = 1;
    dev->devType = DEVTYPE_HARDFILE;

    return RC_OK;
}


/*
 * adfCreateDumpDevice
 *
 * returns NULL if failed
 */ 
    struct Device*
adfCreateDumpDevice(char* filename, long cylinders, long heads, long sectors)
{
    struct Device* dev;
    unsigned char buf[LOGICAL_BLOCK_SIZE];
    struct nativeDevice* nDev;
//    long i;
    int r;
	
    dev=(struct Device*)malloc(sizeof(struct Device));
    if (!dev) { 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc dev");
        return NULL;
    }
    nDev = (struct nativeDevice*)malloc(sizeof(struct nativeDevice));
    if (!nDev) {
        free(dev); 
        (*adfEnv.eFct)("adfCreateDumpDevice : malloc nDev");
        return NULL;
    }
    dev->nativeDev = nDev;

    nDev->fd = (FILE*)fopen(filename,"wb");
    if (!nDev->fd) {
        free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fopen");
        return NULL;
    }

/*    for(i=0; i<cylinders*heads*sectors; i++)
        fwrite(buf, sizeof(unsigned char), 512 , nDev->fd);
*/
    r=fseek(nDev->fd, ((cylinders*heads*sectors)-1)*LOGICAL_BLOCK_SIZE, SEEK_SET);
    if (r==-1) {
        fclose(nDev->fd); free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fseek");
        return NULL;
    }

    fwrite(buf, LOGICAL_BLOCK_SIZE, 1, nDev->fd);

    fclose(nDev->fd);

    nDev->fd=(FILE*)fopen(filename,"rb+");
    if (!nDev->fd) {
        free(nDev); free(dev);
        (*adfEnv.eFct)("adfCreateDumpDevice : fopen");
        return NULL;
    }
    dev->cylinders = cylinders;
    dev->heads = heads;
    dev->sectors = sectors;
    dev->size = cylinders*heads*sectors* LOGICAL_BLOCK_SIZE;	

    if (dev->size==80*11*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPDD;
    else if (dev->size==80*22*2*LOGICAL_BLOCK_SIZE)
        dev->devType = DEVTYPE_FLOPHD;
	else 	
        dev->devType = DEVTYPE_HARDDISK;
		
    dev->nVol = 0;
    dev->isNativeDev = FALSE;
    dev->readOnly = FALSE;

    return(dev);
}

/*##################################################################################*/

