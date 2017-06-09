#! /usr/bin/python

#Ce script est utilisé afin de générer un fichier tmp.txt 
#tmp.txt contenant les informations de l'ensemble des services disponibles sur le réseau
#On a comme information (le nom de l'hôte du service, le nom du service et le port à utiliser pour y accéder

import os
import json

#On lance la commande sytème avahi-browser pour obtenir l'ensemble des services disponibles sur le réseau
var = os.popen("avahi-browse -atpr|grep -E '=;.*_inventory._tcp'").read().split('\n')

#On récupére et on stocke les informations de chaque service
eqpmt = {}
for vv in var:
	vv = vv.split(';')
	if len(vv) != 1:
		key = "{0}_{1}_{2}".format(vv[-4], vv[3], vv[-2])
		eqpmt[key] = [vv[-4], vv[3], vv[-2]] #hostname, service, port

#On enregistre les informations de chaque service en éliminant ceux de l'hôte qui lance le script
#le but étant de n'avoir que les informations sur les services des autres équipements
host = os.popen("hostname").read().split('\n')[0]
fichier = open("tmp.txt", 'w')
for key in eqpmt.keys():
	if eqpmt[key][0] != host+".local":
		fichier.write("{0};{1};{2}\n".format(eqpmt[key][0], eqpmt[key][1], eqpmt[key][2]))

fichier.close()
