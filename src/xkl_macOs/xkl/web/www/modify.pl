#!/usr/local/bin/perl

require "cgi-lib.pl";

MAIN:
{
    $category = $ARGV[0];

    &PrintForm();
}




sub PrintForm {

  print &PrintHeader;
  print &HtmlTop ("Modify the XKL Database Categories");


  # Print out the body of the form from a single quoted here-document
  # Note that if ENDOFTEXT weren't surrounded by single quotes,
  # it would be necessary to be more careful with the other text
  # For example, the @ sign (in the address) would need to be escaped as \@

  print <<'HEADER';

This is a test form which demonstrates the xkl web-based concept.

<HR>
print (<form method="post" action="add.pl">


HEADER

    $absdirpath = "http://figelwump.webjump.com/cgi-bin/";
    $dirpath = "./";

  # open files for the sub-categories, and the waveforms in the cur. category:
    $CATFILE = $dirpath . $category . "/categories.txt"; 

  $PROCESSOR = "modify.pl";

  open (SUBCATS, "<$CATFILE");
  @categories = <SUBCATS>;
  close(SUBCATS);

  print ("<h3> Categories in Current Section: </h3>");

foreach $cat (@categories)
{
   $fullcat = $category . "/" . $cat; 
   print ("<a href = \"" . $absdirpath);
   print ("$PROCESSOR?$fullcat\"> $cat </a> <br>\n");
}

  print <<'NEWCAT';

  <hr> <h3> Create a new Category in this Section: </h3>
<br>
 New Category: <input type="text" name="newcat">
NEWCAT

print ("<input type=\"hidden\" name=\"catpath\" value=\"");
print ($category . "\">");

  print <<'ENDOFTEXT';
<br>

Press <input type="submit" value="here"> to analyze the selected waveform.
</form>
<hr>
<address>Vishal Kapur / vkapur@palate.mit.edu</address>
ENDOFTEXT

  print &HtmlBot;

}



