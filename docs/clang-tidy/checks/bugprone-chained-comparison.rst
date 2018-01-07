.. title:: clang-tidy - bugprone-chained-comparison

bugprone-chained-comparison
===========================

Checks for suspicious use of chained comparison, where it was likely intended
to do pairwise comparison instead. The check will only consider comparisons
chained if the result of that comparison is ``bool`` (to avoid false positives
on DSLs) and if the comparison is unparenthesized (to allow expression of
intent in certain scenarios):

Examples:

.. code-block:: c++

   bool isInRange(int lo, int x, int hi) {
     return lo <= x < hi; // likely intent here is to do the pairwise
                          // checks (lo <= x) && (x < hi)
   }
