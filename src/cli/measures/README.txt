
##

Making changes to ProductionCode (e.g., Taylor)

Use old binary with data and save results

1.  Academic, Beta, or Eval licenses:
Edit main.cpp
Change 
#define MONTH X
where X is 0 (Jan) through 11 (Dec)
#define YEAR X
e.g., 2004
creation date 
string CreationDate = "X";  where X is the day's date.

Fix any bugs

make -f Makefile

strip main
mv main Clustering (or "Measures", etc. which ever binary you are compiling)

Test new binary with same data, and compare results of old binary.

gzip Clustering (or other program name, etc.)

A few programs are not called main.  E.g., ChemTattooModalStats, RussianDoll, MACCSKeys*

The making of the MACCSKeys functions is a bit more complicated and there is a README file 
that explains in the "functions" directory.

2.  Binaries for clients:

The only changes is to make the YEAR 3004.  The program will always run, with thexception 
of the MACCSKeys that will die when OE license dies.

Makefiles are all largely the same format, again with the exception of the MACCSKeys 
and MACCSKeys function Makefiles.  They have a bunch of OE dependencies in them.

Makefile164 and Makefile320 are the program Makefiles, the function Makefiles have the 
word function in them.
