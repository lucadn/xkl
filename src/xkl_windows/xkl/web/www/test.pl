#!/usr/local/bin/perl

# $Header: /cys/people/brenner/http/docs/cgi-lib/ex/RCS/combined-form.cgi,v 1.5 1996/07/31 16:35:52 brenner Exp $
# Copyright (C) 1994 Steven E. Brenner
# This is a small demonstration script to demonstrate the use of 
# the cgi-lib.pl library, where the script both creates the form and
# processes its input

require "cgi-lib.pl";


 MAIN: 
{
# Read in all the variables set by the form
  if (&ReadParse(*input)) {
    &ProcessForm;
  } else {
    &PrintForm;
  }
}

sub ProcessForm {

  # Print the header
  print &PrintHeader;
  print &HtmlTop ("cgi-lib.pl demo form output");

  # Do some processing, and print some output
  ($text = $input{'text'}) =~ s/\n/\n<BR>/g; 
                                   # add <BR>'s after carriage returns
                                   # to multline input, since HTML does not
                                   # preserve line breaks
  print <<ENDOFTEXT;

You, $input{'name'}, whose favorite color is $input{'color'} are on a
quest which is $input{'quest'}, and are looking for the weight of an
$input{'swallow'} swallow.  And this is what you have to say for
yourself:<P> $text<P>

ENDOFTEXT


  # If you want, just print out a list of all of the variables.
  print "<HR>And here is a list of the variables you entered...<P>";
  print &PrintVariables(*input);

  # Close the document cleanly.
  print &HtmlBot;
}


sub PrintForm {

  print &PrintHeader;
  print &HtmlTop ("A simple combined form example");


  # Print out the body of the form from a single quoted here-document
  # Note that if ENDOFTEXT weren't surrounded by single quotes,
  # it would be necessary to be more careful with the other text
  # For example, the @ sign (in the address) would need to be escaped as \@
  print <<'ENDOFTEXT';

This is a sample form which demonstrates the use of
 the <B><A HREF="cgi-lib.pl">cgi-lib.pl</A></B> library of
routines for both generating a form &amp; processing its input.
</P>
<HR>
<form method="post" action="test.pl">
<H2> Pop Quiz: </H2>
What is thy name: <input name="name"><P>
What is thy quest: <input name="quest"><P>

What is thy favorite color:
 <select name="color">
 <option selected>chartreuse
 <option>azure
 <option>puce
 <option>cornflower
 <option>olive draub
 <option>gunmetal
 <option>indigo2
 <option>blanched almond
 <option>flesh
 <option>ochre
 <option>opal
 <option>amber
 <option>mustard
 </select>
<P>

What is the weight of a swallow: <input type="radio" name="swallow"
value="african" checked> African Swallow or
<input type="radio" name="swallow" value="continental"> Continental
Swallow
<P>

What do you have to say for yourself 
<textarea name="text" rows=5 cols=60></textarea>
<P>

Press <input type="submit" value="here"> to submit your query.
</form>
<hr>
<address>Steven E. Brenner / cgi-lib@pobox.com</address>
$Date: 1996/07/31 16:35:52 $
ENDOFTEXT

  print &HtmlBot;

}








