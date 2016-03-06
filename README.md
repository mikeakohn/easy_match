# easy_match
Experimental replacement for simple regex-like searches.  Easy Match
will use syntax such as the following:

(startswith('asdf') or startswith('1111')) and endswith('7890')

Search Keywords are:

startswith()
endswith()
equals()
contains()

Other keywords:

and
or
not

Easy Match searches are compiled with a JIT into a assembly language
function that can be called many times over:

match_t match;

match = compiler_generate("startswith('asdf') or endswith('1111')");

if (match("asdf blah") == 1) { printf("Matches\n"); }


