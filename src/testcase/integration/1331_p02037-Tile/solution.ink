// Translated from solution.cpp.

var INF: dynamic = cpp_cast(1e9);

var LINF: dynamic = cpp_cast(1e18);

var MOD: dynamic = (ll)((1e9 + 7));

var PI: dynamic = acos(-1.0);

var limit: dynamic = 200010;

func REP(i: dynamic, m: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(ll i = m; i < (ll)(n); ++i)");
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
  cpp_macro("cerr << #x_ << \":\" << x_ << endl;");
}

func dbg2(x: dynamic) -> dynamic
{
  cpp_macro("for(auto a_ : x_) cerr << a_ << \" \"; cerr << endl;");
}

func dbg3(x: dynamic, sx: dynamic) -> dynamic
{
  cpp_macro("rep(i, sx_) cerr << x_[i] << \" \"; cerr << endl;");
}

var Dx: dynamic = [0, 0, -1, 1, -1, 1, -1, 1, 0];

var Dy: dynamic = [1, -1, 0, 0, -1, -1, 1, 1, 0];

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(h, w, a, b);
  write(((h * w) - (((h - (h % a))) * ((w - (w % b))))), "\n");
  return 0;
}
