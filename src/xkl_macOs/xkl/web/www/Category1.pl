#!/usr/local/bin/perl

require "cgi-lib.pl";

MAIN:
{
 if (&ReadParse(*input)) {
    &ProcessForm;
  } else {
    &PrintForm;
  }
}



sub ProcessForm {

  # Print the header
  print &PrintHeader;
  print &HtmlTop ("XKL Database Form Output");

  print <<ENDOFTEXT;

here are the waveforms you selected: <P> $waveradio <P>

ENDOFTEXT


  # If you want, just print out a list of all of the variables.
  print "<HR>And here is a list of the variables you entered...<P>";
  print &PrintVariables(*input);

  # Close the document cleanly.
  print &HtmlBot;
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
<form method="post" action="Category1.pl">
<H2> Waveform Database: </H2>


HEADER

print ("Current Waveforms:");
print ("<br>");

$INPUT_FILE = "wavenames.txt";

open (INPUT_FILE);
@array = <INPUT_FILE>;
close(INPUT_FILE);

foreach(@array)
{
   print ("<input type=\"checkbox\" name=\"waveradio\" value=\"");
   print();
   print ("\">");
   print();
   print("<br>");
}

print ("<P>");

  print <<'ENDOFTEXT';


Press <input type="submit" value="here"> to submit your query.
</form>
<hr>
<address>Vishal Kapur / vkapur@palate.mit.edu</address>
ENDOFTEXT

  print &HtmlBot;

}

