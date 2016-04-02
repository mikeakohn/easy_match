# Easy Match

[http://www.mikekohn.net/software/easy_match.php](http://www.mikekohn.net/software/easy_match.php)

Library for doing for simple text searches similar to REGEX but with simpler syntax.  To maximize performance,
a JIT compiler turns matching commands into assembly for x86_64, x86, and ARM. Easy Match will use syntax
such as the following:

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

Using a library is as simple as:

match_t match;

match = compiler_generate("starts_with('asdf') or ends_with('1111')");

if (match("asdf blah") == 1) { printf("Matches\n"); }

Current speed comparison examples when running against the entire
src/generate_x86_64.c file on the x86_64 platform where the JIT is also
turned on for PCRE:

[ starts_with('int ') ^int  ]
* Easy Match       count=11 msec=0.026282
* Easy Match (len) count=11 msec=0.026197
* PCRE             count=11 msec=0.122402
* strncmp()        count=11 msec=0.029939

[ ends_with(');') \);$ ]
* Easy Match       count=124 msec=0.064317
* Easy Match (len) count=124 msec=0.014167
* PCRE             count=124 msec=0.202526
* strncmp()        count=124 msec=0.075861

[ starts_with('int ') or ends_with(');') ^int|\);$ ]
* Easy Match       count=135 msec=0.053906
* Easy Match (len) count=135 msec=0.015211
* PCRE             count=135 msec=0.305322

[ contains('int') int ]
* Easy Match       count=57 msec=0.123988
* Easy Match (len) count=57 msec=0.059791
* PCRE             count=57 msec=0.146438
* strstr()         count=57 msec=0.087332

[ equals('  return 0;') ^  return 0;$ ]
* Easy Match       count=16 msec=0.025463
* Easy Match (len) count=16 msec=0.024647
* PCRE             count=16 msec=0.098309
* strcmp()         count=16 msec=0.021507

[ equals('  return 0') ^  return 0$ ]
* Easy Match       count=0 msec=0.025164
* Easy Match (len) count=0 msec=0.024221
* PCRE             count=0 msec=0.096943
* strcmp()         count=0 msec=0.029123


