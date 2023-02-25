#!/usr/local/bin/perl

# Copyright (c) 1996 Steven E. Brenner
# $Id: fup.cgi,v 1.2 1996/03/30 01:35:32 brenner Exp $


require "./cgi-lib.pl";

# When writing files, several options can be set... here we just set one
# Limit upload size to avoid using too much memory
$cgi_lib'maxdata = 50000; 


# Start off by reading and parsing the data.  Save the return value.
# We could also save the file's name and content type, but we don't
# do that in this example.
$ret = &ReadParse(*input);


# A bit of error checking never hurt anyone
&CgiDie("Error in reading and parsing of CGI input") if !defined $ret;
&CgiDie("No data uploaded",
	"Please enter it in <a href='fup.html'>fup.html</a>.") if !$ret;


# Munge the uploaded text so that it doesn't contain HTML elements
# This munging isn't complete -- lots of illegal characters are left as-is.
# However, it takes care of the most common culprits.  
$input{'upfile'} =~ s/</&lt;/g;
$input{'upfile'} =~ s/>/&gt;/g;


# Now produce the result: an HTML page...
print &PrintHeader;
print &HtmlTop("File Upload Results");
print <<EOT;

<p>You've uploaded a file.  Your notes on the file were:<br>
<blockquote>$input{'note'}</blockquote><br>
<p>The file's contents are:
<pre>
$input{'upfile'}
</pre>
EOT

print &HtmlBot;



