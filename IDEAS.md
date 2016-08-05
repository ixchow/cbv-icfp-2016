Bruteforce
==========

Approximate Solutions:
----------------------
- "translate the square to cover the silhouette"

- Approximate by folding on all edges of the convex hull of the silhouette (repeatedly).

Exact Solutions that are probably slow:
---------------------------------------
- Exact search: unwrap faces from skeleton.

- Will skeleton be provided as hint with all solutions?

Hard Problems:
--------------
- lots of small faces during search
- use really long (greater than 64 bits) integers

Research
========
- http://erikdemaine.org/papers/CGTA2000/paper.pdf
  - polynomial-time positive results if scale doesn't matter
  - three different algorithms
  - very simple cases that we should implement:
    * convex: hiding algorithm shown as Fig 2 or Theorem 2

Computational Geomotry
======================
- Rational numbers (built-in/libraries available in most languages)
- Mirror, line intersection
- Clockwise or not? See https://en.wikipedia.org/wiki/Shoelace_formula

Other Properties
================
- A valid solution is probably physically feasible. See http://erikdemaine.org/papers/PaperReachability_CCCG2004/paper.ps