// Translated from solution.cpp.

var MOD_TYPE = 1;

var MOD = (if ((MOD_TYPE == 1)) (ll)((1e9 + 7)) else 998244353);

var INF = cpp_cast(1e9);

var LINF = cpp_cast(4e18);

var PI = acos(-1.0);

func REP(i: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for (ll i = m; i < (ll)(n); ++i)");
}

func rep(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <bi");
}

var MP = cpp_expression("#include");

var MT = cpp_expression("#include <");

func YES(n: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using names");
}

func Yes(n: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using names");
}

func Possible(n: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; //#");
}

func possible(n: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std; //#");
}

func Yay(n: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namesp");
}

func all(v: dynamic)
{
  return cpp_expression("#include <bits/std");
}

func NP(v: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func dbg(x: dynamic)
{
  cpp_macro("cerr << #x << \":\" << x << endl;");
}

var Dx = [0, 0, -1, 1, -1, 1, -1, 1, 0];

var Dy = [1, -1, 0, 0, -1, -1, 1, 1, 0];

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  write(setprecision(50), setiosflags(ios.fixed));
  var n: dynamic;
  read(n);
  rep(i, n);
  read(a[i]);
  var m: dynamic;
  read(m);
  rep(i, m);
  read(b[i]);
  var result: dynamic;
  set_difference(all(a), all(b), back_inserter(result));
  for (var r in result)
  {
    write(r, "\n");
  }
  return 0;
}
