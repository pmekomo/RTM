#! /usr/bin/python

import os
import json

var = os.popen("avahi-browse -atpr|grep -E '=;.*_inventory._tcp'").read().split('\n')

eqpmt = {}
for vv in var:
	vv = vv.split(';')
	if len(vv) != 1:
		key = "{0}_{1}_{2}".format(vv[-4], vv[3], vv[-2])
		eqpmt[key] = [vv[-4], vv[3], vv[-2]] #hostname, service, port

host = os.popen("hostname").read().split('\n')[0]
fichier = open("tmp.txt", 'w')
for key in eqpmt.keys():
	if eqpmt[key][0] != host+".local":
		fichier.write("{0};{1};{2}\n".format(eqpmt[key][0], eqpmt[key][1], eqpmt[key][2]))

fichier.close()

#for key in eqpmt.keys():
	#if eqpmt[key][0].split('.')[0] == host:
		#os.popen("./server {} &".format(eqpmt[key][2]))
		#msg = os.popen("./server 16 &").read()
