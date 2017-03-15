# circuit-isomatch

A library for graph subisomorphism and pattern-match-and-replace in electronic
circuits.

The end goal of this library is, when given an electronic circuit (as described
with our AST), a pattern (ie. a sub-electronic circuit) and a replacement, to
perform a pattern search in all the graph (aka. graph subisomorphism) and
replace the matching parts with the replacement, without breaking connections.
