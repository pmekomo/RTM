#!/usr/bin/python2
import lxml.etree as ET

print "Content-Type: text/html"     # HTML is following
print                               # blank line, end of headers
print "<TITLE>CGI script output</TITLE>"
#print "<h1>RTM</h1>"

print "<p><img src='data:image/png;base64,%s' /><p>"%(''.join(open('logortm.png').read().encode('base64').split('\n')))

print "<br/>"

print "<h1>ETATS DES EQUIPEMENTS DU BUS</h1>"
dom = ET.parse("simxfi/xmlFiles/final.xml")
xslt = ET.parse("path.xsl")
transform = ET.XSLT(xslt)
newdom = transform(dom)
print(ET.tostring(newdom, pretty_print=True))
