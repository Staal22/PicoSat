# PicoSat

PicoSat is a learning project to familiarize myself with SAT solving and algorithms like DPLL, also to brush up on my C++. It is a relatively lightweight solver that only uses atomic cut and unit propagation.

On my shitty Ryzen 5 2600 it can solve clause sets with a little over 100,000 clauses before timing out (taking longer than 30s).

Name inspired my [MiniSat](http://minisat.se/)

## TODO/Improvements

- More efficient implementation of clauses/clause sets. One could easily imagine using something a data structure like bitsets to more efficiently store and operate on the clause sets instead of the standard library vector, this would greatly reduce memory footprint and improve computation time.

- Generalize:
  Right now the program just runs some tests to confirm that it's working, and then benchmarks itself on larger and larger unsatisfiable clauses (that contain every possible permutation of *n* propositional variables, so $2^n$ clauses). It does this until a solve takes more than 30 secods, at which point it times out. If this was to actually be a useful program it should probably launch some kind of interface and allow for processing specific clause sets. Alas...

## Code style

Allman/BSD style braces.

~~K&R was invented to save paper and people only use it to make their code unreadable because they are  menaces to society (or because it's what they're used to, but it's still objectively less readable).~~
