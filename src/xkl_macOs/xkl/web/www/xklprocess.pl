#!/usr/local/bin/perl

require "cgi-lib.pl";

MAIN:
{
# Read in all the variables set by the form
    &ReadParse(*input);
    &ProcessForm;
}



sub ProcessForm {

  # Print the header
  print &PrintHeader;
  print &HtmlTop ("XKL Database Form Output");

  print <<ENDOFTEXT;

here are the waveforms you selected: <P> $input{'waveradio'} <P>

ENDOFTEXT


  # If you want, just print out a list of all of the variables.
  print "<HR>And here is a list of the variables you entered...<P>";
  print &PrintVariables(*input);

  # Close the document cleanly.
  print &HtmlBot;
}

