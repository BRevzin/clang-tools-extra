// RUN: %check_clang_tidy %s bugprone-chained-comparison %t

struct X {
  X();
  X(bool );
};
bool operator<(X const&, X const& );
bool operator>(X, X);

struct DSL {
  DSL operator<(DSL const& ) const;
};

void Bad() {
  int a, b, c;

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  a < b < c;

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  a > b > c;

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  a == b < c;

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  a < 2 * b < c;

  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  a < (2 & b) < c;

  X x;
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  x < x < x;
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding bool) [bugprone-chained-comparison]
  x < x > x;

   DSL dsl;
  // CHECK-MESSAGES: :[[@LINE+1]]:3: warning: chaining comparisons likely wrong (yielding DSL) [bugprone-chained-comparison]
  dsl < dsl < dsl;
}

void Good() {
  int a, b, c, d;

  a < b && b < c;
  (a < b) < c; // parentheses indicate intent
  (a < b) == (c < d); // ditto

  a < b + c;

  DSL dsl;
  (dsl < dsl) < dsl;

  a == int(b == c); // this isn't a chained comparison
  a == static_cast<int>(b == c); // nor is this
}
