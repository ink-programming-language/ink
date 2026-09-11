// Translated from solution.cpp.

var MOD_TYPE: dynamic = 1;

var MOD: dynamic = ( ((MOD_TYPE == 1)) ? (ll)((1e9 + 7)) : 998244353);

var INF: dynamic = cpp_cast(1e9);

var LINF: dynamic = cpp_cast(4e18);

var PI: dynamic = acos(-1.0);

func REP(i: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (ll i = m; i < (ll)(n); ++i)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

var MP: dynamic = cpp_expression("#include");

var MT: dynamic = cpp_expression("#include <");

func YES(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using names");
}

func Yes(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using names");
}

func Possible(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; //#");
}

func possible(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; //#");
}

func Yay(n: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namesp");
}

func all(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

func NP(v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func dbg(x: dynamic) -> dynamic
{
  cpp_macro("cerr << #x << \":\" << x << endl;");
}

var Dx: dynamic = [0, 0, -1, 1, -1, 1, -1, 1, 0];

var Dy: dynamic = [1, -1, 0, 0, -1, -1, 1, 1, 0];

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  write(setprecision(50), setiosflags(ios.fixed));
  var n: dynamic = cpp_uninitialized();
  read(n);
  rep(i, n);
  read(a[i]);
  var m: dynamic = cpp_uninitialized();
  read(m);
  rep(i, m);
  read(b[i]);
  var result: dynamic = cpp_uninitialized();
  set_difference(all(a), all(b), back_inserter(result));
  for (var r: dynamic in result)
  {
    write(r, "\n");
  }
  return 0;
}
