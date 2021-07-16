/**
 * JEMCC functions for internal hashtable management (VM internal and classes).
 * Copyright (C) 1999-2004 J.M. Heisz 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU Lesser General Public 
 * License, as well as further clarification on your rights to use this 
 * software.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "jeminc.h"

/* Read the VM structure/method definitions */
#include "jem.h"

/* Internal object to represent a hashtable entry */
struct JEM_HashEntry {
    juint hashCode;
    void *key, *object;
};

static void *DummyEntry = (void *) "x";

#define HASHSTART(table, index) ((index) & table->tableMask)
#define HASHJUMP(table, index) ((((index) % (table->tableMask - 2)) + 2) | 1)
#define HASHNEXT(table, index, jump) (((index) + (jump)) & table->tableMask)

/**
 * Initialize a hash table instance to the given number of base hash
 * points.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - pointer to an existing instance of the hashtable to
 *             be initialized.  Already existing entries in the table
 *             will not be cleaned up
 *     startSize - the number of hash blocks to initially allocate in the
 *                 table.  If <= 0, an appropriate start size will be selected.
 *
 * Returns:
 *     JNI_OK - hashtable was initialized
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_HashInitTable(JNIEnv *env, JEMCC_HashTable *table, jsize startSize) {
    int i = 1;

    /* Start small, grow big */
    if (startSize < 0x1F) startSize = 0x1F;
    while (i <= startSize) i = i << 1;
    startSize = i - 1;

    /* Initialize the hash specific details */
    table->entries = JEMCC_Malloc(env, (startSize + 1) * 
                                                 sizeof(struct JEM_HashEntry));
    if (table->entries == NULL) return JNI_ENOMEM;
    table->tableMask = startSize;
    table->entryCount = 0;
    table->occupied = 0;

    return JNI_OK;
}

/**
 * Destroy the internals of a hashtable instance.  Does NOT destroy the
 * objects contained in the hashtable or the hashtable structure itself.
 *
 * Parameters:
 *     table - the hashtable instance to be internally destroyed
 */
void JEMCC_HashDestroyTable(JEMCC_HashTable *table) {
    JEMCC_Free(table->entries);

    table->entries = NULL;
    table->tableMask = 0;
    table->entryCount = 0;
    table->occupied = 0;
}

/**
 * Internal routine to check table fill and expand if necessary.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to be checked/expanded
 *
 * Returns:
 *     JNI_OK - hashtable was unchanged or expansion succeeded
 *     JNI_ENOMEM - a memory allocation failed in expanding the tables
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
static jint checkTableOccupancy(JNIEnv *env, JEMCC_HashTable *table) {
    unsigned int i, index, jump, origMask;
    struct JEM_HashEntry *newEntries, *entry = NULL;

#ifdef ENABLE_ERRORSWEEP
    /* Testsuite always reallocates the hashtable storage */
    origMask = table->tableMask;
    if ((table->occupied + (table->occupied >> 1)) > table->tableMask) {
        table->tableMask = (table->tableMask << 1) | 1;
    }
#else
    if ((table->occupied + (table->occupied >> 1)) <= table->tableMask) {
        /* No need for expansion this time */
        return JNI_OK;
    }
    origMask = table->tableMask;
    table->tableMask = (table->tableMask << 1) | 1;
#endif

    newEntries = JEMCC_Malloc(env, (table->tableMask + 1) * 
                                                sizeof(struct JEM_HashEntry));
    if (newEntries == NULL) {
        table->tableMask = origMask;
        return JNI_ENOMEM;
    }
    table->occupied = table->entryCount;
    for (i = 0; i <= origMask; i++) {
         entry = &(table->entries[i]);
         if ((entry->object != NULL) && (entry->object != DummyEntry)) {
            index = HASHSTART(table, entry->hashCode);
            if (newEntries[index].object != NULL) {
                jump = HASHJUMP(table, entry->hashCode);
                do {
                    index = HASHNEXT(table, index, jump);
                } while (newEntries[index].object != NULL);
            }
            newEntries[index] = *entry;
        }
    }
    JEMCC_Free(table->entries);
    table->entries = newEntries;

    return JNI_OK;
}

/**
 * Core method to handle the two models of hashtable entry insertion 
 * (replace or collide).  See methods below for more information and
 * parameter/return code descriptions.
 */
static jint JEM_HashEntry(JNIEnv *env, JEMCC_HashTable *table,
                          void *key, void *object,
                          void **lastKey, void **lastObject,
                          JEMCC_KeyHashFn keyHashFn, 
                          JEMCC_KeyEqualsFn keyEqualsFn,
                          jboolean replaceFlag) {
    int firstDummyIndex;
    unsigned int index, jump;
    unsigned long hashCode = 0;
    struct JEM_HashEntry *entry = NULL;

    /* First, find a slot to be used/replaced */
    firstDummyIndex = -1;
    hashCode = (*keyHashFn)(env, key);
    index = HASHSTART(table, hashCode);
    if ((entry = &(table->entries[index]))->object != NULL) {
        jump = HASHJUMP(table, hashCode);
        do {
            if (entry->object == DummyEntry) {
                if (firstDummyIndex < 0) firstDummyIndex = index;
            } else {
                if ((entry->hashCode == hashCode) &&
                    ((*keyEqualsFn)(env, entry->key, key))) break;
            }
            index = HASHNEXT(table, index, jump);
        } while ((entry = &(table->entries[index]))->object != NULL);
    }

    if (entry->object == NULL) {
        /* Either insert here or replace a prior dummy record placeholder */
        if (firstDummyIndex < 0) {
            table->occupied++;
            entry->hashCode = hashCode;
            entry->key = key;
            entry->object = object;
            if (checkTableOccupancy(env, table) != JNI_OK) {
                /* The tables are not be modified if exception is thrown */
                table->occupied--;
                entry->hashCode = 0;
                entry->key = entry->object = NULL;
                return JNI_ENOMEM;
            }
        } else {
            entry = &(table->entries[firstDummyIndex]);
            entry->hashCode = hashCode;
            entry->key = key;
            entry->object = object;
        }
        table->entryCount++;

        if (lastKey != NULL) *lastKey = NULL;
        if (lastObject != NULL) *lastObject = NULL;
    } else {
        /* Potentially replace an already existing hash entry */
        if (lastKey != NULL) *lastKey = entry->key;
        if (lastObject != NULL) *lastObject = entry->object;
        if (replaceFlag != JNI_FALSE) {
            entry->key = key;
            entry->object = object;
        } else {
            return JNI_ERR;
        }
    }

    return JNI_OK;
}

/**
 * Store an object into a hashtable.  Hashtable will expand as necessary,
 * and object will replace an already existing object with an equal key.
 * If an existing object is replaced, it is not destroyed but the key/object
 * pair is returned to allow the caller to clean up.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the previous key is returned if
 *               the put entry replaces one in the hashtable or NULL is
 *               returned if the entry is new
 *     lastObject - if this pointer is non-NULL, the previous object is 
 *                  returned if the put entry replaces one in the hashtable or 
 *                  NULL is returned if the entry is new
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successful
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_HashPutEntry(JNIEnv *env, JEMCC_HashTable *table,
                        void *key, void *object,
                        void **lastKey, void **lastObject,
                        JEMCC_KeyHashFn keyHashFn, 
                        JEMCC_KeyEqualsFn keyEqualsFn) {
    return JEM_HashEntry(env, table, key, object, lastKey, lastObject,
                         keyHashFn, keyEqualsFn, JNI_TRUE);
}

/**
 * Almost identical to the HashPutEntry method, this method stores an
 * key->object entry into a hashtable unless there already exists an entry
 * in the hashtable with an "equal" key (i.e. this method will not replace
 * already existing hashtable entries where HashPutEntry does).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to put the key->value pair into
 *     key - the key associated with the entry
 *     object - the object to store in the hashtable according to the given key
 *     lastKey - if this pointer is non-NULL, the existing key is returned if
 *               the insert did not happen (no replace) or NULL if the entry
 *               is new and was inserted
 *     lastObject - if this pointer is non-NULL, the previous object is
 *                  returned if the put entry replaces one in the hashtable or
 *                  NULL is returned if the entry is new
 *     lastObject - if this pointer is non-NULL, the existing key is returned 
 *                  if the insert did not happen (no replace) or NULL if the 
 *                  entry is new and was inserted
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - hashtable insertion was successful
 *     JNI_ERR - an entry already exists in the hashtable for the given key
 *               and no action was taken (no exception has been thrown either)
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_HashInsertEntry(JNIEnv *env, JEMCC_HashTable *table,
                           void *key, void *object,
                           void **lastKey, void **lastObject,
                           JEMCC_KeyHashFn keyHashFn, 
                           JEMCC_KeyEqualsFn keyEqualsFn) {
    return JEM_HashEntry(env, table, key, object, lastKey, lastObject,
                         keyHashFn, keyEqualsFn, JNI_FALSE);
}

/**
 * Remove an entry from the hashtable.  This does not destroy the removed
 * object/key, only the reference to them.  The original key/object pair
 * can be returned to the caller for cleanup purposes.  NOTE: this method
 * is not "safe" for use during hashtable scanning - use HashScanRemoveEntry
 * instead.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     origKey - if this pointer is non-NULL and an entry is removed, the
 *               original key of the entry is returned here
 *     origObject - if this pointer is non-NULL and an entry is removed, the
 *                  object associated with the entry is returned here
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *     
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
jint JEMCC_HashRemoveEntry(JNIEnv *env, JEMCC_HashTable *table, void *key,
                           void **origKey, void **origObject,
                           JEMCC_KeyHashFn keyHashFn, 
                           JEMCC_KeyEqualsFn keyEqualsFn) {
    unsigned int index, jump;
    unsigned long hashCode;
    struct JEM_HashEntry *entry = NULL;

    /* See if we can find the record in question */
    hashCode = (*keyHashFn)(env, key);
    index = HASHSTART(table, hashCode);
    if ((entry = &(table->entries[index]))->object != NULL) {
        jump = HASHJUMP(table, hashCode);
        do {
            if ((entry->object != DummyEntry) &&
                (entry->hashCode == hashCode) &&
                ((*keyEqualsFn)(env, entry->key, key))) break;
            index = HASHNEXT(table, index, jump);
        } while ((entry = &(table->entries[index]))->object != NULL);
    }

    if (entry->object != NULL) {
        if (origKey != NULL) *origKey = entry->key;
        if (origObject != NULL) *origObject = entry->object;

        entry->key = NULL;
        entry->object = DummyEntry;
        table->entryCount--;
    } else {
        if (origKey != NULL) *origKey = NULL;
        if (origObject != NULL) *origObject = NULL;
        return JNI_ERR;
    }

    return JNI_OK;
}

/**
 * Retrieve an object from the hashtable according to the specified key.
 *
 * Parameters: 
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     NULL if no object entry has a matching key, otherwise the matching
 *     object reference.
 */
void *JEMCC_HashGetEntry(JNIEnv *env, JEMCC_HashTable *table, void *key,
                         JEMCC_KeyHashFn keyHashFn, 
                         JEMCC_KeyEqualsFn keyEqualsFn) {
    unsigned int index, jump;
    unsigned long hashCode;
    struct JEM_HashEntry *entry = NULL;

    /* See if we can find the record in question */
    hashCode = (*keyHashFn)(env, key);
    index = HASHSTART(table, hashCode);
    if ((entry = &(table->entries[index]))->object != NULL) {
        jump = HASHJUMP(table, hashCode);
        do {
            if ((entry->object != DummyEntry) &&
                (entry->hashCode == hashCode) &&
                ((*keyEqualsFn)(env, entry->key, key))) break;
            index = HASHNEXT(table, index, jump);
        } while ((entry = &(table->entries[index]))->object != NULL);
    }

    return entry->object;
}

/**
 * Similar to the HashGetEntry method, this retrieves entry information
 * for the provided key, but obtains both the object and the key associated
 * with the hashtable entry.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to retrieve the entry from
 *     key - the key of the object to be obtained
 *     retKey - if non-NULL, the entry key is returned if a matching entry
 *              was found, otherwise NULL is returned
 *     retObject - if non-NULL, the entry object is returned if a matching 
 *                 entry was found, otherwise NULL is returned
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *     
 * Returns:
 *     JNI_OK - an entry matching the key was found and the data was returned
 *     JNI_ERR - no entry matching the key was found 
 */
jint JEMCC_HashGetFullEntry(JNIEnv *env, JEMCC_HashTable *table,
                            void *key, void **retKey, void **retObject,
                            JEMCC_KeyHashFn keyHashFn, 
                            JEMCC_KeyEqualsFn keyEqualsFn) {
    unsigned int index, jump;
    unsigned long hashCode;
    struct JEM_HashEntry *entry = NULL;

    /* See if we can find the record in question */
    hashCode = (*keyHashFn)(env, key);
    index = HASHSTART(table, hashCode);
    if ((entry = &(table->entries[index]))->object != NULL) {
        jump = HASHJUMP(table, hashCode);
        do {
            if ((entry->object != DummyEntry) &&
                (entry->hashCode == hashCode) &&
                ((*keyEqualsFn)(env, entry->key, key))) break;
            index = HASHNEXT(table, index, jump);
        } while ((entry = &(table->entries[index]))->object != NULL);
    }

    if (retKey != NULL) *retKey = entry->key;
    if (retObject != NULL) *retObject = entry->object;
    return ((entry->object != NULL) ? JNI_OK : JNI_ERR);
}

/**
 * Duplicate the given hashtable.  This will create copies of the internal
 * management structures of the hashtable and may possibly create duplicates
 * of the entry keys, if a duplication function is provided.  It does not
 * duplicate the object instances, only the references to the objects.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     dest - the hashtable to copy information into.  Any entries in this
 *            table will be lost without any sort of cleanup
 *     source - the hashtable containing the information to be copied
 *     keyDupFn - if non-NULL, this function will be called to duplicate
 *                the key instances between the tables
 *
 * Returns:
 *     JNI_OK - hashtable was duplicated
 *     JNI_ENOMEM - a memory allocation failed in creating the tables and
 *                  an OutOfMemory exception has been thrown in the current
 *                  environment.  The duplicate hashtable may be partially
 *                  filled, if the memory failure occurred during key
 *                  duplication.
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
jint JEMCC_HashDuplicate(JNIEnv *env, JEMCC_HashTable *dest, 
                         JEMCC_HashTable *source, JEMCC_KeyDupFn keyDupFn) {
    int i;
    struct JEM_HashEntry *srcEntry = NULL, *dstEntry = NULL;

    /* Duplicate the hash count information */
    dest->tableMask = source->tableMask;
    dest->entryCount = source->entryCount;
    dest->occupied = source->occupied;

    /* Duplicate the hash record information */
    dest->entries = JEMCC_Malloc(env, (dest->tableMask + 1) * 
                                                 sizeof(struct JEM_HashEntry));
    if (dest->entries == NULL) return JNI_ENOMEM;
    srcEntry = source->entries;
    dstEntry = dest->entries;
    for (i = 0; i <= dest->tableMask; i++) {
         if (srcEntry->object == DummyEntry) {
             dstEntry->object = DummyEntry;
         } else if (srcEntry->object != NULL) {
             if (keyDupFn != NULL) {
                 dstEntry->key = (*keyDupFn)(env, srcEntry->key);
             } else {
                 dstEntry->key = srcEntry->key;
             }
             if (dstEntry->key == NULL) return JNI_ENOMEM;
             dstEntry->object = srcEntry->object;
             dstEntry->hashCode = srcEntry->hashCode;
         }
         srcEntry++;
         dstEntry++;
    }

    return JNI_OK;
}

/**
 * Scan through all entries in a hashtable, calling the specified
 * callback function for each valid hashtable entry.  NOTE: only the
 * "safe" methods (such as HashScanRemoveEntry below) should be used while
 * a hashtable scan is in progress.
 *
 * Parameters: 
 *     env - the VM environment which is currently in context
 *     table - the hashtable containing the entries to be scanned
 *     entryCB - a function reference which is called for each valid
 *               entry in the hashtable
 *     userData - a caller provided data set which is included in the
 *                scan arguments
 */
void JEMCC_HashScan(JNIEnv *env, JEMCC_HashTable *table,
                    JEMCC_EntryScanCB entryCB, void *userData) {
    struct JEM_HashEntry *entry = table->entries;
    unsigned int tblSize = table->tableMask + 1;
    int rc;

    if (entry == NULL) return;
    while (tblSize > 0) {
        if ((entry->object != NULL) && (entry->object != DummyEntry)) {
            rc = (*entryCB)(env, table, entry->key, entry->object, userData);
            if (rc != JNI_OK) break;
        }
        entry++;
        tblSize--;
    }
}

/**
 * Identical to the HashRemoveEntry method, this removes an entry from
 * the hashtable but is "safe" to use while a scan of the hashtable is
 * in progress.  This does not destroy the removed object/key - only the 
 * reference to them.  
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     table - the hashtable to remove the entry from
 *     key - the key of the pair entry to be removed
 *     keyHashFn - a function reference used to generate hashcode values from
 *                 the table keys
 *     keyEqualsFn - a function reference used to compare keys in the hashtable
 *                   entries
 *
 * Returns:
 *     JNI_OK - a matching entry was found and has been removed
 *     JNI_ERR - no entry matching the specified key was found (no exception
 *               is thrown as it may not really be an error)
 */
void JEMCC_HashScanRemoveEntry(JNIEnv *env, JEMCC_HashTable *table, void *key,
                               JEMCC_KeyHashFn keyHashFn, 
                               JEMCC_KeyEqualsFn keyEqualsFn) {
    unsigned int index, jump;
    unsigned long hashCode;
    struct JEM_HashEntry *entry = NULL;

    /* See if we can find the record in question */
    hashCode = (*keyHashFn)(env, key);
    index = HASHSTART(table, hashCode);
    if ((entry = &(table->entries[index]))->object != NULL) {
        jump = HASHJUMP(table, hashCode);
        do {
            if ((entry->object != DummyEntry) &&
                (entry->hashCode == hashCode) &&
                ((*keyEqualsFn)(env, entry->key, key))) break;
            index = HASHNEXT(table, index, jump);
        } while ((entry = &(table->entries[index]))->object != NULL);
    }

    if (entry->object != NULL) {
        entry->key = NULL;
        entry->object = DummyEntry;
        table->entryCount--;
    }
}

/* * * * * * * * * * * Convenience Hash Methods * * * * * * * * * * */

/**
 * Generate the hashcode value for a char sequence of characters (may
 * be ASCII or UTF-8 encoded Unicode).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode associated with the given characters.
 */
juint JEMCC_StrHashFn(JNIEnv *env, void *key) {
    unsigned long hashCode = 0;
    char *ptr = (char *) key;

    while (*ptr != '\0') {
        hashCode += *(ptr++);
    }
    return hashCode;
}

/**
 * Perform a comparison of the character information contained in the
 * two given keys.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya, keyb - the two char sequences to be compared
 *                  (must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two character sequences are identical (including
 *     null character), JNI_FALSE otherwise.
 */
jboolean JEMCC_StrEqualsFn(JNIEnv *env, void *keya, void *keyb) {
    return (strcmp((char *) keya, (char *) keyb) == 0) ? JNI_TRUE : JNI_FALSE;
}

/**
 * Generate a duplicate of the character information provided in the given
 * key.
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the character sequence to be duplicated
 *
 * Returns:
 *     An allocated copy of the provided character sequence or NULL if
 *     the memory allocation failed (an OutOfMemoryError will have been
 *     thrown in the current environment).
 *
 * Exceptions:
 *     OutOfMemoryError - a memory allocation failed
 */
void *JEMCC_StrDupFn(JNIEnv *env, void *key) {
    char *newKey = JEMCC_Malloc(env, strlen(key) + 1);

    if (newKey == NULL) return NULL;
    (void) strcpy(newKey, (char *) key);
    return (void *) newKey;
}

/**
 * Generate the hashcode value for a fully qualified Java class name.  In
 * this case, the hash result is the same whether the '.' or '/' package
 * separator is used (i.e. 'java/lang/Object' and 'java.lang.Object' will
 * generate the same hashcode).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     key - the fully qualified classname to be hashed (must be \0 terminated)
 *
 * Returns:
 *     The numerical hashcode for the class name, "ignoring" package
 *     separators.
 */
juint JEMCC_ClassNameHashFn(JNIEnv *env, void *key) {
    char *ptr = (char *) key;
    juint hashCode = 0;

    while (*ptr != '\0') {
        if (*ptr == '/') {
            hashCode += '.';
            ptr++;
        } else {
            hashCode += *(ptr++);
        }
    }
    return hashCode;
}

/**
 * Perform a string comparison of two fully qualified Java class names,
 * ignoring any differences in the '.' or '/' package separators
 * (i.e. 'java/lang/Object' and 'java.lang.Object' are identical according
 * to this method).
 *
 * Parameters:
 *     env - the VM environment which is currently in context
 *     keya - the fully qualified classname to be compared
 *     keyb - the fully qualified classname to compare against
 *            (both classnames must be \0 terminated)
 *
 * Returns:
 *     JNI_TRUE if the two classnames are equal ignoring separator differences,
 *     JNI_FALSE if they are different.
 */
jboolean JEMCC_ClassNameEqualsFn(JNIEnv *env, void *keya, void *keyb) {
    char *ptra = (char *) keya, *ptrb = (char *) keyb;

    while (*ptra != '\0') {
        if (*ptra != *ptrb) {
            if (((*ptra != '/') && (*ptra != '.')) ||
                ((*ptrb != '/') && (*ptrb != '.'))) return JNI_FALSE;
        }
        ptra++; ptrb++;
    }
    if (*ptrb != '\0') return JNI_FALSE;
    return JNI_TRUE;
}
