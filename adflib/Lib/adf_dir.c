/*
 *  ADF Library. (C) 1997-1998 Laurent Clevy
 *
 *  adf_dir.c
 *
 *  directory code
 */

#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include"adf_dir.h"
#include"adf_str.h"
#include"adf_util.h"
#include"defendian.h"
#include"adf_blk.h"
#include"adf_raw.h"
#include"adf_disk.h"
#include"adf_bitm.h"
#include"adf_file.h"
#include"adf_err.h"
#include"adf_cache.h"

extern struct Env adfEnv;


/*
 * adfRenameEntry
 *
 */ 
RETCODE adfRenameEntry(struct Volume *vol, SECTNUM pSect, char *oldName,
	SECTNUM nPSect, char *newName)
{
    struct bEntryBlock parent, previous, entry, nParent;
    SECTNUM nSect2, nSect, prevSect, tmpSect;
    int hashValueO, hashValueN, len;
    char name2[MAXNAMELEN+1], name3[MAXNAMELEN+1];
	BOOL intl;
    RETCODE rc;

    if (strcmp(oldName,newName)==0)
        return RC_OK;
    
    intl = isINTL(vol->dosType) || isDIRCACHE(vol->dosType);
    len = strlen(newName);
    myToUpper(name2, newName, len, intl);
    myToUpper(name3, oldName, strlen(oldName), intl);
    /* newName == oldName ? */

    if (adfReadEntryBlock( vol, pSect, &parent )!=RC_OK)
		return RC_ERROR;

    hashValueO = adfGetHashValue(oldName, intl);

    nSect = adfNameToEntryBlk(vol, parent.hashTable, oldName, &entry, &prevSect);
    if (nSect==-1) {
        (*adfEnv.wFct)("adfRenameEntry : existing entry not found");
        return RC_ERROR;
    }

    /* change name and parent dir */
    entry.nameLen = min(31, strlen(newName));
    memcpy(entry.name, newName, entry.nameLen);
    entry.parent = nPSect;
    tmpSect = entry.nextSameHash;

    entry.nextSameHash = 0;
    if (adfWriteEntryBlock(vol, nSect, &entry)!=RC_OK)
		return RC_ERROR;

    /* del from the oldname list */

    /* in hashTable */
    if (prevSect==0) {
        parent.hashTable[hashValueO] = tmpSect;
        if (parent.secType==ST_ROOT)
            rc = adfWriteRootBlock(vol, pSect, (struct bRootBlock*)&parent);
        else
            rc = adfWriteDirBlock(vol, pSect, (struct bDirBlock*)&parent);
        if (rc!=RC_OK)
            return rc;
    }
    else {
        /* in linked list */
	    if (adfReadEntryBlock(vol, prevSect, &previous)!=RC_OK)
            return RC_ERROR;
        /* entry.nextSameHash (tmpSect) could be == 0 */
        previous.nextSameHash = tmpSect;
        if (adfWriteEntryBlock(vol, prevSect, &previous)!=RC_OK)
            return RC_ERROR;
    }


    if (adfReadEntryBlock( vol, nPSect, &nParent )!=RC_OK)
		return RC_ERROR;

    hashValueN = adfGetHashValue(newName, intl);
    nSect2 = nParent.hashTable[ hashValueN ];
    /* no list */
    if (nSect2==0) {
        nParent.hashTable[ hashValueN ] = nSect;
        if (nParent.secType==ST_ROOT)
            rc = adfWriteRootBlock(vol, nPSect, (struct bRootBlock*)&nParent);
        else
            rc = adfWriteDirBlock(vol, nPSect, (struct bDirBlock*)&nParent);
    }
    else {
        /* a list exists : addition at the end */
        /* len = strlen(newName);
                   * name2 == newName
                   */
        do {
            if (adfReadEntryBlock(vol, nSect2, &previous)!=RC_OK)
                return -1;
            if (previous.nameLen==len) {
                myToUpper(name3,previous.name,previous.nameLen,intl);
                if (strncmp(name3,name2,len)==0) {
                    (*adfEnv.wFct)("adfRenameEntry : entry already exists");
                    return -1;
                }
            }
            nSect2 = previous.nextSameHash;
//printf("sect=%ld\n",nSect2);
        }while(nSect2!=0);
        
        previous.nextSameHash = nSect;
        if (previous.secType==ST_DIR)
            rc=adfWriteDirBlock(vol, previous.headerKey, 
			       (struct bDirBlock*)&previous);
        else if (previous.secType==ST_FILE)
            rc=adfWriteFileHdrBlock(vol, previous.headerKey, 
                   (struct bFileHeaderBlock*)&previous);
        else {
            (*adfEnv.wFct)("adfRenameEntry : unknown entry type");
            rc = RC_ERROR;
        }
	
	}
    if (rc!=RC_OK)
        return rc;

    if (isDIRCACHE(vol->dosType))
		if (pSect==nPSect) {
            adfUpdateCache(vol, &parent, (struct bEntryBlock*)&entry,TRUE);
        }
        else {
            adfDelFromCache(vol,&parent,entry.headerKey);
            adfAddInCache(vol,&nParent,&entry);
        }
/*
    if (isDIRCACHE(vol->dosType) && pSect!=nPSect) {
        adfUpdateCache(vol, &nParent, (struct bEntryBlock*)&entry,TRUE);
    }
*/
    return RC_OK;
}

/*
 * adfRemoveEntry
 *
 */
RETCODE adfRemoveEntry(struct Volume *vol, SECTNUM pSect, char *name)
{
    struct bEntryBlock parent, previous, entry;
    SECTNUM nSect2, nSect;
    int hashVal;
    BOOL intl;

    if (adfReadEntryBlock( vol, pSect, &parent )!=RC_OK)
		return RC_ERROR;
    nSect = adfNameToEntryBlk(vol, parent.hashTable, name, &entry, &nSect2);
    if (nSect==-1) {
        (*adfEnv.wFct)("adfRemoveEntry : entry not found");
        return RC_ERROR;
    }
    /* if it is a directory, is it empty ? */
    if ( entry.secType==ST_DIR && !isDirEmpty((struct bDirBlock*)&entry) ) {
        (*adfEnv.wFct)("adfRemoveEntry : directory not empty");
        return RC_ERROR;
    }
/*    printf("name=%s  nSect2=%ld\n",name, nSect2);*/

    /* in parent hashTable */
    if (nSect2==0) {
        intl = isINTL(vol->dosType) || isDIRCACHE(vol->dosType);
        hashVal = adfGetHashValue( name, intl );
/*printf("hashTable=%d nexthash=%d\n",parent.hashTable[hashVal],
 entry.nextSameHash);*/
        parent.hashTable[hashVal] = entry.nextSameHash;
        if (adfWriteEntryBlock(vol, pSect, &parent)!=RC_OK)
			return RC_ERROR;
    }
    /* in linked list */
    else {
        if (adfReadEntryBlock(vol, nSect2, &previous)!=RC_OK)
			return RC_ERROR;
        previous.nextSameHash = entry.nextSameHash;
        if (adfWriteEntryBlock(vol, nSect2, &previous)!=RC_OK)
			return RC_ERROR;
    }

    if (entry.secType==ST_FILE) {
        adfFreeFileBlocks(vol, (struct bFileHeaderBlock*)&entry);
        (*adfEnv.notifyFct)(pSect,ST_FILE);
    }
    else {
        adfSetBlockFree(vol, nSect);
        (*adfEnv.notifyFct)(pSect,ST_DIR);
    }

    if (isDIRCACHE(vol->dosType))
        adfDelFromCache(vol, &parent, entry.headerKey);

    adfUpdateBitmap(vol);

    return RC_OK;
}


/*
 * adfSetEntryComment
 *
 */
RETCODE adfSetEntryComment(struct Volume* vol, SECTNUM parSect, char* name,
    char* newCmt)
{
    struct bEntryBlock parent, entry;
    SECTNUM nSect;

    if (adfReadEntryBlock( vol, parSect, &parent )!=RC_OK)
		return RC_ERROR;
    nSect = adfNameToEntryBlk(vol, parent.hashTable, name, &entry, NULL);
    if (nSect==-1) {
        (*adfEnv.wFct)("adfSetEntryComment : entry not found");
        return RC_ERROR;
    }

    entry.commLen = min(MAXCMMTLEN, strlen(newCmt));
    memcpy(entry.comment, newCmt, entry.commLen);

    if (entry.secType==ST_DIR)
        adfWriteDirBlock(vol, nSect, (struct bDirBlock*)&entry);
    else if (entry.secType==ST_FILE)
        adfWriteFileHdrBlock(vol, nSect, (struct bFileHeaderBlock*)&entry);
    else
        (*adfEnv.wFct)("adfSetEntryComment : entry secType incorrect");

    if (isDIRCACHE(vol->dosType))
        adfUpdateCache(vol, &parent, (struct bEntryBlock*)&entry, TRUE);

    return RC_OK;
}


/*
 * adfSetEntryAccess
 *
 */
RETCODE adfSetEntryAccess(struct Volume* vol, SECTNUM parSect, char* name,
    long newAcc)
{
    struct bEntryBlock parent, entry;
    SECTNUM nSect;

    if (adfReadEntryBlock( vol, parSect, &parent )!=RC_OK)
		return RC_ERROR;
    nSect = adfNameToEntryBlk(vol, parent.hashTable, name, &entry, NULL);
    if (nSect==-1) {
        (*adfEnv.wFct)("adfSetEntryAccess : entry not found");
        return RC_ERROR;
    }

    entry.access = newAcc;
    if (entry.secType==ST_DIR)
        adfWriteDirBlock(vol, nSect, (struct bDirBlock*)&entry);
    else if (entry.secType==ST_FILE)
        adfWriteFileHdrBlock(vol, nSect, (struct bFileHeaderBlock*)&entry);
    else
        (*adfEnv.wFct)("adfSetEntryAccess : entry secType incorrect");

    if (isDIRCACHE(vol->dosType))
        adfUpdateCache(vol, &parent, (struct bEntryBlock*)&entry, FALSE);

    return RC_OK;
}


/*
 * isDirEmpty
 *
 */
BOOL isDirEmpty(struct bDirBlock *dir)
{
    int i;
	
    for(i=0; i<HT_SIZE; i++)
        if (dir->hashTable[i]!=0)
           return FALSE;

    return TRUE;
}


/*
 * adfFreeDirList
 *
 */
void adfFreeDirList(struct List* list)
{
    struct List *root, *cell;

    root = cell = list;
    while(cell!=NULL) {
        adfFreeEntry(cell->content);
        if (cell->subdir!=NULL)
            adfFreeDirList(cell->subdir);
        cell = cell->next;
    }
    freeList(root);
}


/*
 * adfGetRDirEnt
 *
 */
struct List* adfGetRDirEnt(struct Volume* vol, SECTNUM nSect, BOOL recurs )
{
    struct bEntryBlock entryBlk;
	struct List *cell, *head;
    int i;
    struct Entry *entry;
    SECTNUM nextSector;
    long *hashTable;
    struct bEntryBlock parent;


    if (adfEnv.useDirCache && isDIRCACHE(vol->dosType))
        return (adfGetDirEntCache(vol, nSect, recurs ));


    if (adfReadEntryBlock(vol,nSect,&parent)!=RC_OK)
		return NULL;

    hashTable = parent.hashTable;
    cell = head = NULL;
    for(i=0; i<HT_SIZE; i++) {
        if (hashTable[i]!=0) {
             entry = (struct Entry *)malloc(sizeof(struct Entry));
             if (!entry) {
                 adfFreeDirList(head);
				 (*adfEnv.eFct)("adfGetDirEnt : malloc");
                 return NULL;
             }
             if (adfReadEntryBlock(vol, hashTable[i], &entryBlk)!=RC_OK) {
				 adfFreeDirList(head);
                 return NULL;
             }
             if (adfEntBlock2Entry(&entryBlk, entry)!=RC_OK) {
				 adfFreeDirList(head); return NULL;
             }
             entry->sector = hashTable[i];
	
             if (head==NULL)
                 head = cell = newCell(0, (void*)entry);
             else
                 cell = newCell(cell, (void*)entry);
             if (cell==NULL) {
                 adfFreeDirList(head); return NULL;
             }

             if (recurs && entry->type==ST_DIR)
                 cell->subdir = adfGetRDirEnt(vol,entry->sector,recurs);

             /* same hashcode linked list */
             nextSector = entryBlk.nextSameHash;
             while( nextSector!=0 ) {
                 entry = (struct Entry *)malloc(sizeof(struct Entry));
                 if (!entry) {
                     adfFreeDirList(head);
					 (*adfEnv.eFct)("adfGetDirEnt : malloc");
                     return NULL;
                 }
                 if (adfReadEntryBlock(vol, nextSector, &entryBlk)!=RC_OK) {
					 adfFreeDirList(head); return NULL;
                 }

                 if (adfEntBlock2Entry(&entryBlk, entry)!=RC_OK) {
					 adfFreeDirList(head);
                     return NULL;
                 }
                 entry->sector = nextSector;
	
                 cell = newCell(cell, (void*)entry);
                 if (cell==NULL) {
                     adfFreeDirList(head); return NULL;
                 }
				 
                 if (recurs && entry->type==ST_DIR)
                     cell->subdir = adfGetRDirEnt(vol,entry->sector,recurs);
				 
                 nextSector = entryBlk.nextSameHash;
             }
        }
    }

/*    if (parent.extension && isDIRCACHE(vol->dosType) )
        adfReadDirCache(vol,parent.extension);
*/
    return head;
}


/*
 * adfGetDirEnt
 *
 */
struct List* adfGetDirEnt(struct Volume* vol, SECTNUM nSect )
{
    return adfGetRDirEnt(vol, nSect, FALSE);
}


/*
 * adfFreeEntry
 *
 */
void adfFreeEntry(struct Entry *entry)
{
	if (entry==NULL)
       return;
    if (entry->name)
        free(entry->name);
    if (entry->comment)
        free(entry->comment);
    free(entry);    
}


/*
 * adfChangeDir
 *
 */
RETCODE adfChangeDir(struct Volume* vol, char *name)
{
    struct bEntryBlock entry;
    SECTNUM nSect;

    if (adfReadEntryBlock( vol, vol->curDirPtr, &entry )!=RC_OK)
		return RC_ERROR;
    nSect = adfNameToEntryBlk(vol, entry.hashTable, name, &entry, NULL);
/*printf("adfChangeDir=%d\n",nSect);*/
    if (nSect!=-1) {
        vol->curDirPtr = nSect;
//        (*adfEnv.notifyFct)(0,ST_ROOT);
        return RC_OK;
    }
    else
		return RC_ERROR;
}


/*
 * adfParentDir
 *
 */
SECTNUM adfParentDir(struct Volume* vol)
{
    struct bEntryBlock entry;

    if (vol->curDirPtr!=vol->rootBlock) {
        if (adfReadEntryBlock( vol, vol->curDirPtr, &entry )!=RC_OK)
			return RC_ERROR;
        vol->curDirPtr = entry.parent;
    }
    return RC_OK;
}


/*
 * adfEntBlock2Entry
 *
 */
RETCODE adfEntBlock2Entry(struct bEntryBlock *entryBlk, struct Entry *entry)
{
    char buf[MAXCMMTLEN+1];
    int len;

	entry->type = entryBlk->secType;
    entry->parent = entryBlk->parent;

    len = min(entryBlk->nameLen, MAXNAMELEN);
    strncpy(buf, entryBlk->name, len);
    buf[len] = '\0';
    entry->name = strdup(buf);
    if (entry->name==NULL)
        return RC_MALLOC;
//printf("len=%d name=%s parent=%ld\n",entryBlk->nameLen, entry->name,entry->parent );
    adfDays2Date( entryBlk->days, &(entry->year), &(entry->month), &(entry->days));
	entry->hour = entryBlk->mins/60;
    entry->mins = entryBlk->mins%60;
    entry->secs = entryBlk->ticks/50;

    entry->access = -1;
    entry->size = 0L;
    entry->comment = NULL;
    entry->real = 0L;
    switch(entryBlk->secType) {
    case ST_ROOT:
        break;
    case ST_DIR:
        entry->access = entryBlk->access;
        len = min(entryBlk->commLen, MAXCMMTLEN);
        strncpy(buf, entryBlk->comment, len);
        buf[len] = '\0';
        entry->comment = strdup(buf);
        if (entry->comment==NULL) {
            free(entry->name);
            return RC_MALLOC;
        }
        break;
    case ST_FILE:
        entry->access = entryBlk->access;
        entry->size = entryBlk->byteSize;
        len = min(entryBlk->commLen, MAXCMMTLEN);
        strncpy(buf, entryBlk->comment, len);
        buf[len] = '\0';
        entry->comment = strdup(buf);
        if (entry->comment==NULL) {
            free(entry->name);
            return RC_MALLOC;
        }
        break;
    case ST_LFILE:
    case ST_LDIR:
        entry->real = entryBlk->realEntry;
    case ST_LSOFT:
        break;
    default:
        (*adfEnv.wFct)("unknown entry type");
    }
	
    return RC_OK;
}


/*
 * adfNameToEntryBlk
 *
 */
SECTNUM adfNameToEntryBlk(struct Volume *vol, long ht[], char* name, 
    struct bEntryBlock *entry, SECTNUM *nUpdSect)
{
    int hashVal;
    char upperName[MAXNAMELEN+1];
    char upperName2[MAXNAMELEN+1];
    SECTNUM nSect;
    int nameLen;
    BOOL found;
    SECTNUM updSect;
    BOOL intl;

    intl = isINTL(vol->dosType) || isDIRCACHE(vol->dosType);
    hashVal = adfGetHashValue( name, intl );
    nameLen = strlen(name);
    myToUpper( upperName, name, nameLen, intl );

    nSect = ht[hashVal];
    if (nSect==0)
        return -1;

    updSect = 0;
    found = FALSE;
    do {
        if (adfReadEntryBlock(vol, nSect, entry)!=RC_OK)
			return -1;
        if (nameLen==entry->nameLen) {
            myToUpper( upperName2, entry->name, nameLen, isINTL(vol->dosType) );
            found = strncmp(upperName, upperName2, nameLen)==0;
        }
        if (!found) {
            updSect = nSect;
            nSect = entry->nextSameHash; 
        }
    }while( !found && nSect!=0 );

    if ( nSect==0 && !found )
        return -1;
    else {
        if (nUpdSect!=NULL)
            *nUpdSect = updSect;
        return nSect;
    }
}


/*
 * Access2String
 *
 */
    char* 
adfAccess2String(long acc)
{
    static char ret[8+1];

    strcpy(ret,"----rwed");
    if (hasD(acc)) ret[7]='-';
    if (hasE(acc)) ret[6]='-';
    if (hasW(acc)) ret[5]='-';
    if (hasR(acc)) ret[4]='-';
    if (hasA(acc)) ret[3]='a';
    if (hasP(acc)) ret[2]='p';
    if (hasS(acc)) ret[1]='s';
    if (hasH(acc)) ret[0]='h';

    return(ret);
}


/*
 * adfCreateEntry
 *
 */
SECTNUM adfCreateEntry(struct Volume *vol, struct bEntryBlock *dir, char *name )
{
    BOOL intl;
    struct bEntryBlock updEntry;
    int len, hashValue;
    RETCODE rc;
    char name2[MAXNAMELEN+1], name3[MAXNAMELEN+1];
    SECTNUM nSect, newSect, newSect2;
    struct bRootBlock* root;

//puts("adfCreateEntry in");

    intl = isINTL(vol->dosType) || isDIRCACHE(vol->dosType);
    len = strlen(name);
    myToUpper(name2, name, len, intl);
    hashValue = adfGetHashValue(name, intl);
    nSect = dir->hashTable[ hashValue ];

    if ( nSect==0 ) {
        newSect = adfGet1FreeBlock(vol);
        if (newSect==-1) {
           (*adfEnv.wFct)("adfCreateEntry : nSect==-1");
           return -1;
        }

        dir->hashTable[ hashValue ] = newSect;
        if (dir->secType==ST_ROOT) {
            root = (struct bRootBlock*)dir;
            adfTime2AmigaTime(adfGiveCurrentTime(),
                &(root->cDays),&(root->cMins),&(root->cTicks));
            rc=adfWriteRootBlock(vol, vol->rootBlock, root);
        }
        else {
            adfTime2AmigaTime(adfGiveCurrentTime(),&(dir->days),&(dir->mins),&(dir->ticks));
            rc=adfWriteDirBlock(vol, dir->headerKey, (struct bDirBlock*)dir);
        }
//puts("adfCreateEntry out, dir");
        if (rc!=RC_OK) {
            adfSetBlockFree(vol, newSect);    
            return -1;
        }
        else
            return( newSect );
    }

    do {
        if (adfReadEntryBlock(vol, nSect, &updEntry)!=RC_OK)
			return -1;
        if (updEntry.nameLen==len) {
            myToUpper(name3,updEntry.name,updEntry.nameLen,intl);
            if (strncmp(name3,name2,len)==0) {
                (*adfEnv.wFct)("adfCreateEntry : entry already exists");
                return -1;
            }
        }
        nSect = updEntry.nextSameHash;
    }while(nSect!=0);

    newSect2 = adfGet1FreeBlock(vol);
    if (newSect2==-1) {
        (*adfEnv.wFct)("adfCreateEntry : nSect==-1");
        return -1;
    }
	 
    rc = RC_OK;
    updEntry.nextSameHash = newSect2;
    if (updEntry.secType==ST_DIR)
        rc=adfWriteDirBlock(vol, updEntry.headerKey, (struct bDirBlock*)&updEntry);
    else if (updEntry.secType==ST_FILE)
        rc=adfWriteFileHdrBlock(vol, updEntry.headerKey, 
		    (struct bFileHeaderBlock*)&updEntry);
    else
        (*adfEnv.wFct)("adfCreateEntry : unknown entry type");

//puts("adfCreateEntry out, hash");
    if (rc!=RC_OK) {
        adfSetBlockFree(vol, newSect2);    
        return -1;
    }
    else
        return(newSect2);
}


/*
 * myToUpper
 *
 */
    void
myToUpper( char *nstr, char *ostr, int nlen, BOOL intl )
{
    int i;

    if (intl)
        for(i=0; i<nlen; i++)
            nstr[i]=adfIntlToUpper(ostr[i]);
    else
        for(i=0; i<nlen; i++)
            nstr[i]=toupper(ostr[i]);
    nstr[nlen]='\0';
}


/*
 * adfIntlToUpper
 *
 */
    int 
adfIntlToUpper(int c)
{
return (c>='a' && c<='z') || (c>=224 && c<=254 && c!=247) ? c - ('a'-'A') : c ;
}


/*
 * adfGetHashValue
 * 
 */
    int 
adfGetHashValue(char *name, BOOL intl)
{
    int hash;
    int i,len;

    len = hash = strlen(name);
    for(i=0; i<len; i++) {
        hash = hash*13;
        if (intl)
                hash = hash + adfIntlToUpper(name[i]);
        else
                hash = hash + toupper(name[i]);
        hash = hash & 0x7ff;
    }
    hash = hash % HT_SIZE;

    return(hash);
}


/*
 * printEntry
 *
 */
void printEntry(struct Entry* entry)
{
    printf("%-30s %2d %6ld ", entry->name, entry->type, entry->sector);
    printf("%2d/%02d/%04d %2d:%02d:%02d",entry->days, entry->month, entry->year,
        entry->hour, entry->mins, entry->secs);
    if (entry->type==ST_FILE)
        printf("%8ld ",entry->size);
    else
        printf("         ");
    if (entry->type==ST_FILE || entry->type==ST_DIR)
        printf("%-s ",adfAccess2String(entry->access));
    if (entry->comment!=NULL)
        printf("%s ",entry->comment);
    putchar('\n');
}


/*
 * adfCreateDir
 *
 */
RETCODE adfCreateDir(struct Volume* vol, SECTNUM nParent, char* name)
{
    SECTNUM nSect;
    struct bDirBlock dir;
    struct bEntryBlock parent;
//    SECTNUM nCache;

    if (adfReadEntryBlock(vol, nParent, &parent)!=RC_OK)
		return RC_ERROR;

    nSect = adfCreateEntry(vol, &parent, name);
    if (nSect==-1) {
        (*adfEnv.wFct)("adfCreateDir : no sector available");
        return RC_ERROR;
    }
    memset(&dir, 0, sizeof(struct bDirBlock));
    dir.nameLen = min(MAXNAMELEN, strlen(name));
    memcpy(dir.dirName,name,dir.nameLen);
    dir.headerKey = nSect;

    if (parent.secType==ST_ROOT)
        dir.parent = vol->rootBlock;
    else
        dir.parent = parent.headerKey;
    adfTime2AmigaTime(adfGiveCurrentTime(),&(dir.days),&(dir.mins),&(dir.ticks));

    if (isDIRCACHE(vol->dosType)) {
        adfAddInCache(vol, &parent, (struct bEntryBlock *)&dir);
        /* for adfCreateEmptyCache, will be added by adfWriteDirBlock */
        dir.secType = ST_DIR;
        adfCreateEmptyCache(vol, (struct bEntryBlock *)&dir, -1);
    }

    /* writes the dirblock, with the possible dircache assiocated */
    if (adfWriteDirBlock(vol, nSect, &dir)!=RC_OK)
		return RC_ERROR;

    adfUpdateBitmap(vol);

    (*adfEnv.notifyFct)(nParent,ST_DIR);

    return RC_OK;
}


/*
 * adfCreateFile
 *
 */
RETCODE adfCreateFile(struct Volume* vol, SECTNUM nParent, char *name,
    struct bFileHeaderBlock *fhdr)
{
    SECTNUM nSect;
    struct bEntryBlock parent;
//puts("adfCreateFile in");
    if (adfReadEntryBlock(vol, nParent, &parent)!=RC_OK)
		return RC_ERROR;

    nSect = adfCreateEntry(vol, &parent, name);
    if (nSect==-1) return RC_ERROR;
/*printf("new fhdr=%d\n",nSect);*/
    memset(fhdr,0,512);
    fhdr->nameLen = min(MAXNAMELEN, strlen(name));
    memcpy(fhdr->fileName,name,fhdr->nameLen);
    fhdr->headerKey = nSect;
    if (parent.secType==ST_ROOT)
        fhdr->parent = vol->rootBlock;
    else if (parent.secType==ST_DIR)
        fhdr->parent = parent.headerKey;
    else
        (*adfEnv.wFct)("adfCreateFile : unknown parent secType");
    adfTime2AmigaTime(adfGiveCurrentTime(),
        &(fhdr->days),&(fhdr->mins),&(fhdr->ticks));

    if (adfWriteFileHdrBlock(vol,nSect,fhdr)!=RC_OK)
		return RC_ERROR;

    if (isDIRCACHE(vol->dosType))
        adfAddInCache(vol, &parent, (struct bEntryBlock *)fhdr);

    adfUpdateBitmap(vol);
//puts("adfCreateFile out");

    (*adfEnv.notifyFct)(nParent,ST_FILE);

    return RC_OK;
}


/*
 * adfReadEntryBlock
 *
 */
RETCODE adfReadEntryBlock(struct Volume* vol, SECTNUM nSect, struct bEntryBlock *ent)
{
    unsigned char buf[512];

    if (adfReadBlock(vol, nSect, buf)!=RC_OK)
        return RC_ERROR;

    memcpy(ent, buf, 512);
#ifdef LITT_ENDIAN
    swapEndian((unsigned char*)ent, SWBL_ENTRY);
#endif
/*printf("readentry=%d\n",nSect);*/
    if (ent->checkSum!=adfNormalSum((unsigned char*)buf,20,512)) {
        (*adfEnv.wFct)("adfReadEntryBlock : invalid checksum");
        return RC_ERROR;
    }
    if (ent->type!=T_HEADER) {
        (*adfEnv.wFct)("adfReadEntryBlock : T_HEADER id not found");
        return RC_ERROR;
    }
    if (ent->nameLen<0 || ent->nameLen>MAXNAMELEN || ent->commLen>MAXCMMTLEN) {
        (*adfEnv.wFct)("adfReadEntryBlock : nameLen or commLen incorrect"); 
        printf("nameLen=%d, commLen=%d, name=%s sector%ld\n",
            ent->nameLen,ent->commLen,ent->name, ent->headerKey);
    }

    return RC_OK;
}


/*
 * adfWriteEntryBlock
 *
 */
RETCODE adfWriteEntryBlock(struct Volume* vol, SECTNUM nSect, struct bEntryBlock *ent)
{
    unsigned char buf[512];
    unsigned long newSum;
   

    memcpy(buf, ent, sizeof(struct bEntryBlock));

#ifdef LITT_ENDIAN
    swapEndian(buf, SWBL_ENTRY);
#endif
    newSum = adfNormalSum(buf,20,sizeof(struct bEntryBlock));
    swLong(buf+20, newSum);

    if (adfWriteBlock(vol, nSect, buf)!=RC_OK)
		return RC_ERROR;

    return RC_OK;
}


/*
 * adfWriteDirBlock
 *
 */
RETCODE adfWriteDirBlock(struct Volume* vol, SECTNUM nSect, struct bDirBlock *dir)
{
    unsigned char buf[512];
    unsigned long newSum;
    

/*printf("wdirblk=%d\n",nSect);*/
    dir->type = T_HEADER;
    dir->highSeq = 0;
    dir->hashTableSize = 0;
    dir->secType = ST_DIR;

    memcpy(buf, dir, sizeof(struct bDirBlock));
#ifdef LITT_ENDIAN
    swapEndian(buf, SWBL_DIR);
#endif
    newSum = adfNormalSum(buf,20,sizeof(struct bDirBlock));
    swLong(buf+20, newSum);

    if (adfWriteBlock(vol, nSect, buf)!=RC_OK)
		return RC_ERROR;

    return RC_OK;
}



/*###########################################################################*/
