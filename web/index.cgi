#!/usr/bin/python2
import lxml.etree as ET

print "Content-Type: text/html"     # HTML is following
print                               # blank line, end of headers
print "<TITLE>CGI script output</TITLE>"

print "<p><img src='data:image/png;base64,%s' /><p>"%(''.join(open('ITXPT_green_RGB-2153744500.png').read().encode('base64').split('\n')))

print "<br/>"

print "<h1>EQUIPMENTS STATES</h1>"
dom = ET.parse("simxfi/xmlFiles/final.xml")
xslt = ET.parse("path.xsl")
transform = ET.XSLT(xslt)
newdom = transform(dom)
print(ET.tostring(newdom, pretty_print=True))
