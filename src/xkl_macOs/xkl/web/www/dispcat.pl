#!/usr/local/bin/perl

require "cgi-lib.pl";

MAIN:
{
    $category = $ARGV[0];

    &PrintForm();
}




sub PrintForm {

  print &PrintHeader;
  print &HtmlTop ("Testing the XKL Form");


  # Print out the body of the form from a single quoted here-document
  # Note that if ENDOFTEXT weren't surrounded by single quotes,
  # it would be necessary to be more careful with the other text
  # For example, the @ sign (in the address) would need to be escaped as \@

  print <<'HEADER';

This is a test form which demonstrates the xkl web-based concept.

<HR>
<form method="post" action="xklprocess.pl">


HEADER

#    $dirpath = "http://figelwump.webjump.com/cgi-bin/";
    $dirpath = "./";

  # open files for the sub-categories, and the waveforms in the cur. category:
    $CATFILE = $dirpath . $category . "/categories.txt"; 
  $NAMEFILE = $dirpath . $category . "/wavenames.txt";
  $PROCESSOR = "dispcat.pl";

  open (SUBCATS, "<$CATFILE");
  @categories = <SUBCATS>;
  close(SUBCATS);

  open (WAVENAMES, "<$NAMEFILE");
  @waveforms = <WAVENAMES>;
  close(WAVENAMES);

  print ("<h3> Sub Categories: </h3>");

foreach $cat (@categories)
{
   $fullcat = $category . "/" . $cat; 
   print ("<a href = \"http://figelwump.webjump.com/cgi-bin/");
   print ("$PROCESSOR?$fullcat\"> $cat </a> <br>\n");
}


  print ("<hr> <h3> Waveforms in the Current Category: </h3>");

foreach(@waveforms)
{
   print ("<input type=\"radio\" name=\"waveradio\" value=\"");
   print();
   print ("\">");
   print();
   print("<br>");
}


  print <<'ENDOFTEXT';

Press <input type="submit" value="here"> to analyze the selected waveform.
</form>
<hr>
<address>Vishal Kapur / vkapur@palate.mit.edu</address>
ENDOFTEXT

  print &HtmlBot;

}



