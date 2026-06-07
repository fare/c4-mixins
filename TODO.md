## TODO

* Package and distribute it as a library that C++ programmers might actually use.

* Figure out a way to be O(dn) or at least O(dn log n) with some kind of hash-tables
  or sets for the ancestor counting during template processing.
  This should not be a big deal until users have hundred-deep mixin hierarchies, though.
