/*
 * HeapManager.h
 *
 *  Created on: Feb 4, 2015
 *      Author: robert
 */

#ifndef HEAPMANAGER_H_
#define HEAPMANAGER_H_

#include <cstdlib>

class HeapManager {
   char* space;
   char* here;
   unsigned int size;
public:
	HeapManager();
	virtual ~HeapManager();
	char* allocate(unsigned int);
<<<<<<< HEAD
	char* getStart();
	unsigned short int getOffset();
=======
	void deallocate(char*);
>>>>>>> Float, String literals now work

};

#endif /* HEAPMANAGER_H_ */
