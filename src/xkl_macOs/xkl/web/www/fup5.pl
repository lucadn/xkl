#!/usr/local/bin/perl

require 5.001;
use strict;
require "./cgi-lib.pl";

MAIN: 
{
  my (%cgi_data,  # The form data
      %cgi_cfn,   # The uploaded file(s) client-provided name(s)
      %cgi_ct,    # The uploaded file(s) content-type(s).  These are
                  #   set by the user's browser and may be unreliable
      %cgi_sfn,   # The uploaded file(s) name(s) on the server (this machine)
      $ret,       # Return value of the ReadParse call.       
      $buf        # Buffer for data read from disk.
     );

  # When writing files, several options can be set..
  # Spool the files to the /tmp directory
  $cgi_lib::writefiles = "http://figelwump.webjump.com/";   
 
  # Limit upload size to avoid using too much memory
  $cgi_lib::maxdata = 50000; 

  # Start off by reading and parsing the data.  Save the return value.
  # Pass references to retreive the data, the filenames, and the content-type
  $ret = &ReadParse(\%cgi_data,\%cgi_cfn,\%cgi_ct,\%cgi_sfn);

  # A bit of error checking never hurt anyone
  if (!defined $ret) {
    &CgiDie("Error in reading and parsing of CGI input");
  } elsif (!$ret) {
    &CgiDie("Missing parameters\n",
            "Please complete the form <a href='fup.html'>fup.html</a>.\n");
  } elsif (!defined $cgi_data{'upfile'} or !defined $cgi_data{'note'}) {
    &CgiDie("Data missing\n",
            "Please complete the form <a href='fup.html'>fup.html</a>.\n");
  }



  # Now print the page for the user to see...
  print &PrintHeader;
  print &HtmlTop("File Upload Results");

  print <<EOT;
<p>You've uploaded a file.  Your notes on the file were:<br>
<blockquote>$cgi_data{'note'}</blockquote><br>
<p>The file has been spooled to disk as: <i>$cgi_data{'upfile'}</i><br>
The file's reported name on the client machine is:
 <i>$cgi_cfn{'upfile'}</i><br>
The file's reported Content-type (possibly none) was:
 <i>$cgi_ct{'upfile'}</i><br>
<hr>
The contents of $cgi_data{'upfile'} are as follows:<br>
<pre>
EOT

  # Print the contents of the uploaded file
  open (UPFILE, $cgi_sfn{'upfile'}) or 
    &CgiError("Error: Unable to open file $cgi_sfn{'upfile'}: $!\n");
  $buf = "";    # avoid annoying warning message
  while (read (UPFILE, $buf, 8192)) {
    # Munge the uploaded text so that it doesn't contain HTML elements
    # This munging isn't complete -- lots of illegal characters are left
    # However, it takes care of the most common culprits.  
    $buf =~ s/</&lt;/g;
    $buf =~ s/>/&gt;/g;
    print $buf;
  }
  close (UPFILE);

  print "</pre>\n";

  unlink ($cgi_sfn{'upfile'}) or
   &CgiError("Error: Unable to delete file",
             "Error: Unable to delete file $cgi_sfn{'upfile'}: $!\n");
  # cleanup - delete the uploaded file
  # Note that when using spooling of files to disk, the uploaded file's
  # name on the server machine is in both %cgi_data and %cgi_sfn
  # (that is, the first and fourth parameters to ReadParse).  However,
  # for technical reasons, the data in %cgi_data are tainted.  The data in
  # %cgi_sfn are not tainted, but the keys can contain only a limited
  # set of characters ([-\w] in cgi-lib 2.8)

  print "<hr>File $cgi_data{'upfile'} has now been removed\n";
  print &HtmlBot;


  # The following lines are solely to suppress 'only used once' warnings
  $cgi_lib::writefiles = $cgi_lib::writefiles;
  $cgi_lib::maxdata    = $cgi_lib::maxdata;

}




