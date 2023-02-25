#!/usr/local/bin/perl

require "cgi-lib.pl";

MAIN: 
{
  &PrintForm;
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
<H2> Waveform Database: </H2>


HEADER

print ("Current Waveforms:\n");
print ("<br>");

  $INPUT_FILE = "categories.txt";
  $PROCESSOR = "dispcat.pl";

  open (INPUT, "<$INPUT_FILE");
  @array = <INPUT>;
  close(INPUT);

foreach $cat (@array)
{
   print ("<a href = \"http://figelwump.webjump.com/cgi-bin/");
   print ("$PROCESSOR?$cat\"> $cat </a> <br>\n");

}

print ("<P>");

  print <<'ENDOFTEXT';



<hr>
<address>Vishal Kapur / vkapur@palate.mit.edu</address>
ENDOFTEXT

  print &HtmlBot;

}







