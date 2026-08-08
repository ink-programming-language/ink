// Translated from solution.cpp.

var INF = cpp_cast(1e9);

var LINF = cpp_cast(1e18);

var MOD = (ll)((1e9 + 7));

var PI = acos(-1.0);

var limit = 200010;

func REP(i: dynamic, m: dynamic, n: dynamic)
{
  cpp_macro("for(ll i = m; i < (ll)(n); ++i)");
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
  cpp_macro("cerr << #x_ << \":\" << x_ << endl;");
}

func dbg2(x: dynamic)
{
  cpp_macro("for(auto a_ : x_) cerr << a_ << \" \"; cerr << endl;");
}

func dbg3(x: dynamic, sx: dynamic)
{
  cpp_macro("rep(i, sx_) cerr << x_[i] << \" \"; cerr << endl;");
}

var Dx = [0, 0, -1, 1, -1, 1, -1, 1, 0];

var Dy = [1, -1, 0, 0, -1, -1, 1, 1, 0];

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var h: dynamic;
  var w: dynamic;
  var a: dynamic;
  var b: dynamic;
  read(h, w, a, b);
  write(((h * w) - (((h - (h % a))) * ((w - (w % b))))), "\n");
  return 0;
}
