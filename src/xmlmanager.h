#ifndef XMLMANAGER_H
#define XMLMANAGER_H

#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>
#ifndef WIN32
	#include <sys/types.h>
#endif

#include <unistd.h>
#include <string.h>
#include <sys/stat.h> 
#include <iostream>
#include <vector>
#include <libxml/parser.h>

#define FINAL_FILE "final.xml"

void update(char *docName);
xmlNodePtr rechercheEq(xmlNodePtr node, xmlChar * equipmentId);
xmlNodePtr rechercheInd(xmlNodePtr node, xmlChar * equipmentId);
int myStrcmp(xmlChar * ch1, xmlChar * ch2);

#endif
