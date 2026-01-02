#!/usr/bin/python

import xml.etree.ElementTree
import sys

class ParserXFI:
    
    def __init__(self, file):
        self.file = file
        self.root = xml.etree.ElementTree.parse(self.file).getroot()
        self.status = ''
        self.equipment = {}

    def parser(self):
        event = self.root.find('event')
        state = event.find('state')
        res = {'equipment-id':state.get('equipment-id'),
               'status': state.get('status')}
        if state.find('equipment') is not None:
            res['equipment'] = []
            for equipment in state.findall('equipment'):
                eq = {'equipment-id':equipment.get('equipment-id'),
                      'status': equipment.get('status')}
                res['equipment'].append(eq)
        print res

if __name__== '__main__':
    if (len(sys.argv) < 2) or (len(sys.argv)>=3):
        print '[USE]\n$ parser_xfi file_name'
    else:
        stat = ParserXFI(sys.argv[1])
        stat.parser()
