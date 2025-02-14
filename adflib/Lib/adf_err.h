#ifndef ADF_ERR_H
#define ADF_ERR_H

/*
 * adf_err.h
 *
 *
 */


#define hasRC(rc,c) ((rc)&(c))

#define RC_OK				0
#define RC_ERROR			-1

#define RC_MALLOC           1
#define RC_VOLFULL			2


#define RC_FOPEN            1<<10
#define RC_NULLPTR          1<<12

/* adfRead*Block() */

#define RC_BLOCKTYPE        1
#define RC_BLOCKSTYPE       1<<1
#define RC_BLOCKSUM         1<<2
#define RC_HEADERKEY		1<<3
#define RC_BLOCKREAD        1<<4

/* adfWrite*Block */
#define RC_BLOCKWRITE       1<<4


/* adfReadBlock() */
#define RC_BLOCKOUTOFRANGE  1
#define RC_BLOCKNATREAD     1<<1

/* adfWriteBlock() */
/* RC_BLOCKOUTOFRANGE */
#define RC_BLOCKNATWRITE    1<<1
#define RC_BLOCKREADONLY    1<<2

/* adfInitDumpDevice() */
/* RC_FOPEN */
/* RC_MALLOC */

/* adfNativeReadBlock(), adfReadDumpSector() */

#define RC_BLOCKSHORTREAD   1
#define RC_BLOCKFSEEK       1<<1

/* adfNativeWriteBlock(), adfWriteDumpSector() */

#define RC_BLOCKSHORTWRITE  1
/* RC_BLOCKFSEEK */


/*-- adfReadRDSKblock --*/
#define RC_BLOCKID          1<<5

/*-- adfWriteRDSKblock() --*/
/*RC_BLOCKREADONLY*/

#endif /* ADF_ERR_H */

/*############################################################################*/