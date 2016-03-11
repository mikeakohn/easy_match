# easy_match

[http://www.mikekohn.net/software/easy_match.php](http://www.mikekohn.net/software/easy_match.php)

Experimental replacement for simple regex-like searches.  Easy Match
will use syntax such as the following:

(starts_with('asdf') or starts_with('1111')) and ends_with('7890')

Search Keywords are:

* starts_with()
* ends_with()
* match_at()
* equals()
* contains()

Other keywords:

* and
* or
* not

Easy Match searches are compiled with a JIT into a assembly language
function that can be called many times over:

match_t match;

match = compiler_generate("starts_with('asdf') or ends_with('1111')");

if (match("asdf blah") == 1) { printf("Matches\n"); }


