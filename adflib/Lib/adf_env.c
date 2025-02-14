/*
 * ADF Library
 *
 * adf_env.c
 *
 */

#include<stdio.h>
#include<stdlib.h>

#include"adf_defs.h"
#include"adf_str.h"
#include"adf_nativ.h"
#include"adf_env.h"

#include"defendian.h"

union u{
    long l;
    char c[4];
    };

ENV_DECLARATION;


void Warning(char* msg) {
    fprintf(stderr,"Warning <%s>\n",msg);
}

void Error(char* msg) {
    fprintf(stderr,"Error <%s>\n",msg);
//    exit(1);
}

void Verbose(char* msg) {
    fprintf(stderr,"Verbose <%s>\n",msg);
}

void Changed(SECTNUM nSect, int changedType)
{
/*    switch(changedType) {
    case ST_FILE:
        fprintf(stderr,"Notification : sector %ld (FILE)\n",nSect);
        break;
    case ST_DIR:
        fprintf(stderr,"Notification : sector %ld (DIR)\n",nSect);
        break;
    case ST_ROOT:
        fprintf(stderr,"Notification : sector %ld (ROOT)\n",nSect);
        break;
    default:
        fprintf(stderr,"Notification : sector %ld (???)\n",nSect);
    }
*/}

/*
 * adfInitEnv
 *
 */
void adfEnvInitDefault()
{
//    char str[80];
    union u val;

    /* internal checking */

    if (sizeof(short)!=2) 
        { fprintf(stderr,"Compilation error : sizeof(short)!=2\n"); exit(1); }
    if (sizeof(long)!=4) 
        { fprintf(stderr,"Compilation error : sizeof(short)!=2\n"); exit(1); }
    if (sizeof(struct bEntryBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bEntryBlock)!=512\n"); exit(1); }
    if (sizeof(struct bRootBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bRootBlock)!=512\n"); exit(1); }
    if (sizeof(struct bDirBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bDirBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBootBlock)!=1024)
        { fprintf(stderr,"Internal error : sizeof(struct bBootBlock)!=1024\n"); exit(1); }
    if (sizeof(struct bFileHeaderBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bFileHeaderBlock)!=512\n"); exit(1); }
    if (sizeof(struct bFileExtBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bFileExtBlock)!=512\n"); exit(1); }
    if (sizeof(struct bOFSDataBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bOFSDataBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBitmapBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bBitmapBlock)!=512\n"); exit(1); }
    if (sizeof(struct bBitmapExtBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bBitmapExtBlock)!=512\n"); exit(1); }
    if (sizeof(struct bLinkBlock)!=512)
        { fprintf(stderr,"Internal error : sizeof(struct bLinkBlock)!=512\n"); exit(1); }

    val.l=1L;
/* if LITT_ENDIAN not defined : must be BIG endian */
#ifndef LITT_ENDIAN
    if (val.c[3]!=1) /* little endian : LITT_ENDIAN must be defined ! */
        { fprintf(stderr,"Compilation error : #define LITT_ENDIAN must exist\n"); exit(1); }
#else
    if (val.c[3]==1) /* big endian : LITT_ENDIAN must not be defined ! */
        { fprintf(stderr,"Compilation error : #define LITT_ENDIAN must not exist\n"); exit(1); }
#endif

    adfEnv.wFct = Warning;
    adfEnv.eFct = Error;
    adfEnv.vFct = Verbose;
    adfEnv.notifyFct = Changed;

    adfEnv.useDirCache = FALSE;

/*    sprintf(str,"ADFlib %s (%s)",adfGetVersionNumber(),adfGetVersionDate());
    (*adfEnv.vFct)(str);
*/
    adfEnv.nativeFct=(struct nativeFunctions*)malloc(sizeof(struct nativeFunctions));
    if (!adfEnv.nativeFct) (*adfEnv.wFct)("adfInitDefaultEnv : malloc");

    adfInitNativeFct();
}


/*
 * adfEnvCleanUp
 *
 */
void adfEnvCleanUp()
{
    free(adfEnv.nativeFct);
}


/*
 * adfChgEnvProp
 *
 */
void adfChgEnvProp(int prop, void *new)
{
	BOOL *newBool;

    switch(prop) {
    case PR_VFCT:
        adfEnv.vFct = (void(*)(char*))new;
        break;
    case PR_WFCT:
        adfEnv.wFct = (void(*)(char*))new;
        break;
    case PR_EFCT:
        adfEnv.eFct = (void(*)(char*))new;
        break;
    case PR_NOTFCT:
        adfEnv.notifyFct = (void(*)(SECTNUM,int))new;
        break;
    case PR_USEDIRC:
        newBool = (BOOL*)new;
		adfEnv.useDirCache = *newBool;
        break;
    }
}

/*
 *  adfSetEnv
 *
 */
void adfSetEnvFct( void(*eFct)(char*), void(*wFct)(char*), void(*vFct)(char*),
    void(*notFct)(SECTNUM,int)  )
{
    if (*eFct!=0)
		adfEnv.eFct = *eFct;
    if (*wFct!=0)
		adfEnv.wFct = *wFct;
    if (*vFct!=0)
		adfEnv.vFct = *vFct;
    if (*notFct!=0)
        adfEnv.notifyFct = *notFct;
}


/*
 * adfGetVersionNumber
 *
 */
char* adfGetVersionNumber()
{
	return(ADFLIB_VERSION);
}


/*
 * adfGetVersionDate
 *
 */
char* adfGetVersionDate()
{
	return(ADFLIB_DATE);
}




/*##################################################################################*/
