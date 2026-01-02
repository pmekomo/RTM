/***************************************************************************************************************************************************
Programme permettant de gérer la fusion de fichiers XML dans un répertoire
On gère l'ensemble des fichiers du répertoire dans un vector dont un fichier principal constituant un agregat des différents autres
Une fonction permet de vérifier s'il y a des modifications dans le répertoire(nouveau fichiers, modification de fichiers)
En cas de modification dans le répertoire une mise à jour du fichier XML principal est effectuée

Note-1:Ce programme est considéré comme un daemon il doit être lancé en parallèle des autres programmes sur chaque équipement
Note-2:Ce programme n'est pas figé et peut être modifié pour être adapté à tous fichiers XML

****************************************************************************************************************************************************/
#include "xmlmanager.h"

using namespace std;
 
typedef struct xmlFile
{
	char name[1024];
	char creationtime[1024];
	int indicator; //indicateur permettant de savoir si le fichier est récent
}xmlFile;

//we manage a table of the current repository files
vector<xmlFile> filesTab;

//Fonction qui compte le nombre de fichiers dans le répertoire
int countFiles(void)
{
	DIR * dir = NULL;
	dir = opendir("xmlFiles");
	int nbf = 0;
	struct dirent* file = NULL;

	while ((file = readdir(dir)) != NULL)
	{
		if (strcmp(file->d_name, ".")!= 0 && strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, FINAL_FILE) != 0)
			nbf++;
	}
	if (closedir(dir) == -1)
		exit(-1);

	return nbf;
}

//Fonction qui initialise le conteneur de fichiers avec les fichiers du répertoire
int init_filesTab (void)
{
	DIR * dir = NULL;
	dir = opendir("xmlFiles");
	struct stat sb;
	struct dirent *file = NULL;
	
	filesTab.clear();
	if(dir != NULL)
	{
		while((file = readdir(dir)) != NULL)
		{
			if (strcmp(file->d_name, ".")!= 0 && strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, FINAL_FILE) != 0)
			{
				xmlFile xmltmp;
				strcpy(xmltmp.name, "xmlFiles/");
				strcat(xmltmp.name, file->d_name);
				if (stat(xmltmp.name, &sb) != -1)
				{
					strcpy(xmltmp.creationtime, ctime(&sb.st_mtime));
					xmltmp.indicator = 0;
					filesTab.push_back(xmltmp);
				}
				else
				{
					cout<<"Stat a échoué"<<endl;
					return -1;
				}
			}
		}
	}

	if (closedir(dir) == -1)
		exit(-1);

	return 0;
}

//Fonction permettant de vérifier l'ensemble des fichiers du répertoire
int check_xmlfiles (void)
{
	DIR * dir = NULL;
	dir = opendir("xmlFiles");
	struct dirent * file = NULL;
	struct stat sb;
	int nb_files = filesTab.size(),cmp;
	while ((file = readdir(dir)) != NULL)
	{
		if (strcmp(file->d_name, ".")!= 0 && strcmp(file->d_name, "..")!=0 && strcmp(file->d_name, FINAL_FILE) != 0)
		{
			char tmp[1024];
			strcpy(tmp, "xmlFiles/");
			strcat(tmp, file->d_name);
			int i = 0, trouve = 0;

			//On recherche le fichier contenu dans tmp dans le conteneur de fichiers filesTab
			while(i< nb_files && trouve == 0)
			{
				//Si on le retrouve on vérifie s'il a été modifié et on lui affecte un indicateur le précisant("1" pour modifié)
				if (strcmp(filesTab[i].name, tmp) == 0)
				{
					trouve = 1;
					if (stat (tmp, &sb) != -1)
					{
						cmp = strcmp(filesTab[i].creationtime, ctime(&sb.st_mtime));
						//cout<<"name:"<<filesTab[i].name<<" date:"<<filesTab[i].creationtime<<" now:"<<ctime(&sb.st_mtime)<<endl; 
						if (cmp != 0)
						{
							strcpy(filesTab[i].creationtime, ctime(&sb.st_mtime));
							filesTab[i].indicator = 1;//fichier modifié
						}
					}
					else
						cout<<"stat failed"<<endl;
				}
				i++;
			}
			//si on a pas retrouvé le fichier dans le conteneur de fichier on l'y insère et on ajoute un indication précisant qu'il est nouveau
			//("2" pour nouveau fichier)
			if (trouve == 0)
			{
				xmlFile xmltmp;
				strcpy(xmltmp.name, tmp);
				if (stat(xmltmp.name, &sb) != -1)
				{
					strcpy(xmltmp.creationtime, ctime(&sb.st_mtime));
					xmltmp.indicator = 2;//nouveau fichier
					filesTab.push_back(xmltmp);
				}
				else
					cout<<"stat failed for the new file"<<endl;

			}
		}
	}
	if (closedir(dir) == -1)
		exit(-1);

	return 0;
}

//Fonction permettant de mettre à jour un fichier XML(Ajout de Noeud, ajout d'attributs, modification des valeurs des attributs...)
void update(char *docName)
{
	xmlDocPtr doc, doc2;
	xmlNodePtr cur, cur2, node, node2, subnode, subnode2, newnode, indnode;
	doc = xmlParseFile(docName);
	doc2 = xmlParseFile("xmlFiles/final.xml");//Document à mettre à jour
	xmlChar * status, * equip, * indicatorValue, * indicatorName;
	int modif = 0;
	
	if (doc == NULL) {
		fprintf(stderr, "Le documents %s n'a pas pu être ouvert \n", docName);
		return;
	}
	else
		if (doc2 == NULL){
			fprintf(stderr, "Le documents final.xml n'a pas pu être ouvert \n");
			return;
		}
	cur = xmlDocGetRootElement(doc);
	cur2 = xmlDocGetRootElement(doc2);
	if (cur == NULL) {
		fprintf(stderr, "Le document %s est vide\n", docName);
		return;
	}
	else
		if (cur2 == NULL){
			fprintf(stderr, "Le document final.xml est vide\n");
			return;
		}
	//On vérifie le nom du noeud principal
	if (xmlStrcmp(cur->name, (const xmlChar*) "event")) {
		fprintf(stderr, "le document a un type invalide, root node != %s \n",
				cur->name);
		xmlFreeDoc(doc);
		return;
	}
	else
		if(xmlStrcmp(cur2->name, (const xmlChar*) "event")){
			fprintf(stderr, "le document final.xml a un type invalide, root node != %s \n",
				cur2->name);
			xmlFreeDoc(doc2);
			return;
		}
		
	cur = cur->xmlChildrenNode; //-->state du fichier intermédiaire
	cur2 = cur2->xmlChildrenNode;//-->state du fichier résultat
	/*if (baliseFille != NULL) {*/
	//On parcourt tous les noeuds du fichiers intermédiaire pour faire les comparaisons avec tout ceux du fichier résultat
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "state"))) 
		{
			node = cur->xmlChildrenNode;//-->equipment du fichier intermédiaire
			while (cur2 != NULL)
			{
				if(!xmlStrcmp(cur2->name, (const xmlChar *) "state"))
				{
					if(myStrcmp(xmlGetProp(cur2, (const xmlChar *)"equipment-id"), 
						xmlGetProp(cur, (const xmlChar *)"equipment-id")) != 0)
					{
						//fonction de recherche
						node2 = rechercheEq(cur2, xmlGetProp(cur, (const xmlChar *)"equipment-id"));
						if (node2 != NULL)
						{
							if (myStrcmp(xmlGetProp(node2, (const xmlChar *)"status"),
								xmlGetProp(cur, (const xmlChar *)"status")) != 0)
							{
								xmlSetProp(node2, (const xmlChar *)"status", xmlGetProp(cur, (const xmlChar *)"status"));
								modif = 1;
							}
						}
						else
						{
							newnode = xmlNewTextChild(cur2, NULL, (const xmlChar *)"equipment", NULL);
							xmlNewProp (newnode, (const xmlChar *)"status", xmlGetProp(node2, (const xmlChar *)"status"));
							xmlNewProp (newnode, (const xmlChar *)"equipment-id", xmlGetProp(node2, (const xmlChar *)"equipment-id"));
							modif = 1;
						}
					}
					while(node != NULL)
					{
						if((!xmlStrcmp(node->name, (const xmlChar *) "equipment")))
						{
							status = xmlGetProp(node, (const xmlChar *)"status"); //status de l'équipement à rechercher
							equip = xmlGetProp(node, (const xmlChar *)"equipment-id"); //equipement à rechercher
							
							node2 = rechercheEq(cur2, equip); //on effectue une recherche dans le fichier résultat
							if (node2 != NULL)
							{
								if (myStrcmp(xmlGetProp(node2, (const xmlChar *)"status"),
									status) != 0)
								{
									xmlSetProp(node2, (const xmlChar *)"status", status);
									modif = 1;
								}
								subnode = node->xmlChildrenNode; //-->indicator dans le fichier intermédiaire
								while(subnode != NULL)
								{
									if((!xmlStrcmp(subnode->name, (const xmlChar *) "indicator")))
									{
										indicatorName = xmlGetProp(subnode, (const xmlChar *)"name");
										indicatorValue = xmlGetProp(subnode, (const xmlChar *)"value");
										subnode2 = node2->xmlChildrenNode;
										
										subnode2 = rechercheInd(node2, indicatorName);
										if(subnode2 != NULL)
										{
											if(myStrcmp(indicatorValue, xmlGetProp(subnode2, (const xmlChar *)"value")) != 0)
											{
												xmlSetProp(subnode2, (const xmlChar *)"value", indicatorValue);
												modif = 1;
											}
										}
										else
										{
											indnode = xmlNewTextChild(node2, NULL, (const xmlChar *)"indicator", NULL);
											xmlNewProp (indnode, (const xmlChar *)"name", xmlGetProp(subnode, (const xmlChar *)"name"));
											xmlNewProp (indnode, (const xmlChar *)"value", xmlGetProp(subnode, (const xmlChar *)"value"));
											modif = 1;
										}
										
									}
									subnode = subnode->next;
								}
							}
							else
							{
								newnode = xmlNewTextChild(cur2, NULL, (const xmlChar *)"equipment", NULL);
								xmlNewProp (newnode, (const xmlChar *)"status", xmlGetProp(node, (const xmlChar *)"status"));
								xmlNewProp (newnode, (const xmlChar *)"equipment-id", xmlGetProp(node, (const xmlChar *)"equipment-id"));
								
								subnode = node->xmlChildrenNode;
								while(subnode != NULL)
								{
									if((!xmlStrcmp(subnode->name, (const xmlChar *) "indicator")))
									{
										indnode = xmlNewTextChild(newnode, NULL, (const xmlChar *)"indicator", NULL);
										xmlNewProp (indnode, (const xmlChar *)"name", xmlGetProp(subnode, (const xmlChar *)"name"));
										xmlNewProp (indnode, (const xmlChar *)"value", xmlGetProp(subnode, (const xmlChar *)"value"));
									}
									subnode = subnode->next;
								}
								modif = 1;
							}	
						}
						node = node->next;
					}
				}
				cur2 = cur2->next;
			}
			
		}
		cur = cur->next;
	}
	if (doc2 != NULL && modif == 1)
	{
		xmlSaveFormatFile("xmlFiles/final.xml", doc2, 1);
	}
	
	xmlFreeDoc(doc);
	xmlFreeDoc(doc2);
}

xmlNodePtr rechercheEq(xmlNodePtr node, xmlChar * equipmentId)
{
	xmlNodePtr tmp = node->xmlChildrenNode;
	while(tmp != NULL)
	{
		if((!xmlStrcmp(tmp->name, (const xmlChar *) "equipment")))
		{
			if(myStrcmp(equipmentId , xmlGetProp(tmp, (const xmlChar *)"equipment-id")) == 0)
			{
				return tmp;
			}
		}
		tmp = tmp->next;
	}
	return NULL;
}

xmlNodePtr rechercheInd(xmlNodePtr node, xmlChar * equipmentId)
{
	xmlNodePtr tmp = node->xmlChildrenNode;
	while(tmp != NULL)
	{
		if((!xmlStrcmp(tmp->name, (const xmlChar *) "indicator")))
		{
			if(myStrcmp(equipmentId , xmlGetProp(tmp, (const xmlChar *)"name")) == 0)
			{
				return tmp;
			}
		}
		tmp = tmp->next;
	}
	return NULL;
}

int myStrcmp(xmlChar * ch1, xmlChar * ch2)
{
	int i;
	for (i=0; i<xmlStrlen(ch1); i++)
		ch1[i]=tolower(ch1[i]);
		
	for (i=0; i<xmlStrlen(ch2); i++)
		ch2[i]=tolower(ch2[i]);
		
	return xmlStrcmp(ch1, ch2);
	
}


void updateFiles()
{
	for(unsigned int i = 0; i< filesTab.size(); i++)
	{
		//S'il s'agit d'un fichier modifié
		if(filesTab[i].indicator == 1)
		{
			cout<<"modified: "<<filesTab[i].name<<endl;
			update(filesTab[i].name);
			filesTab[i].indicator = 0;
		}
		else
			//S'il s'agit d'un nouveau fichier
			if(filesTab[i].indicator == 2)
			{
				cout<<"new: "<<filesTab[i].name<<endl;
				update(filesTab[i].name);
				filesTab[i].indicator = 0;
			}
	}
	
}

int main (void)
{
	int init_nbf = countFiles();
	
	init_filesTab();
	while (1)
	{	
		check_xmlfiles();		
		int curr_nbf = countFiles();	
		cout<<"nb de fichiers: "<<curr_nbf<<endl;
		if(init_nbf > curr_nbf)
		{
			init_nbf = curr_nbf;
			cout<<"y a des fichiers en moins :'("<<endl;
			init_filesTab();
		}
		else
		{
			updateFiles();
		}
		sleep(10);

	}

	return 0;
}
