#include "cgi.h"

using namespace std;

void head(string title)
{
	cout<<"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\
	\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n<html\
	 xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"fr\" >\n\t<head>";
	cout<<"\t\t<title>"<<title<<"</title>";
	cout<<"\t\t<meta http-equiv=\"Content-Type\" content=\"text/html;\
	 charset=utf-8\"/>\n\t</head>\n\t<body>";
}

void foot()
{
	cout<<"\t<body>\n<html>";
}
