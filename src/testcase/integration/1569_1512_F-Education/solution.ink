// Translated from solution.cpp.

var FI: dynamic = cpp_expression("/*");

var SE: dynamic = cpp_expression("/*");

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("/* Author: Ngu");
}

func SZ(a: dynamic) -> dynamic
{
  return cpp_expression("/* Author:");
}

func MS(s: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("/* Author: Nguyen T");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for (int i = (a); i <= (b); i++)");
}

func FORE(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for (int i = (a); i >= (b); i--)");
}

func FORALL(it: dynamic, a: dynamic) -> dynamic
{
  cpp_macro("for (__typeof((a).begin()) it = (a).begin(); it != (a).end(); it++)");
}

func TRAV(x: dynamic, a: dynamic) -> dynamic
{
  return cpp_expression("/* Author: Ng");
}

func ckmin(a: dynamic, val: dynamic) -> dynamic
{
  return  ((val < a)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func ckmax(a: dynamic, val: dynamic) -> dynamic
{
  return  ((a < val)) ? cpp_comma(cpp_assign(a, "=", val), 1) : 0;
}

func remDup(v: dynamic) -> dynamic
{
  sort(ALL(v));
  v.erase(unique(ALL(v)), end(v));
}

func pct(x: dynamic) -> dynamic
{
  return builtin_popcount(x);
}

func bits(x: dynamic) -> dynamic
{
  return  ((x == 0)) ? 0 : (31 - builtin_clz(x));
}

func p2(x: dynamic) -> dynamic
{
  return (1 << x);
}

func msk2(x: dynamic) -> dynamic
{
  return (p2(x) - 1);
}

func ceilDiv(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / b) + (((((a ^ b)) > 0) && (a % b))));
}

func floorDiv(a: dynamic, b: dynamic) -> dynamic
{
  return ((a / b) - (((((a ^ b)) < 0) && (a % b))));
}

func setPrec(x: dynamic) -> dynamic
{
  write(fixed, setprecision(x));
}

var ts: dynamic = cpp_expression("/* Au");

func ts(c: dynamic) -> dynamic
{
  return string_cpp(1, c);
}

func ts(s: dynamic) -> dynamic
{
  return cpp_cast(s);
}

func ts(s: dynamic) -> dynamic
{
  return s;
}

func ts(b: dynamic) -> dynamic
{
  return ts(cpp_cast(b));
}

func ts(c: dynamic) -> dynamic
{
  var ss: dynamic = cpp_uninitialized();
  (ss << c);
  return ss.str();
}

func ts(v: dynamic) -> dynamic
{
  var res: dynamic = "{";
  FOR(i, 0, (SZ(v) - 1)) += char((cpp_char("0") + v[i]));
  res += "}";
  return res;
}

func ts(b: dynamic) -> dynamic
{
  var res: dynamic = "";
  FOR(i, 0, (SZ(b) - 1)) += char((cpp_char("0") + b[i]));
  return res;
}

func ts(v: dynamic) -> dynamic
{
  var fst: dynamic = 1;
  var res: dynamic = "";
  for (var x: dynamic in v)
  {
    if ((!fst))
    {
      res += " ";
    }
    fst = 0;
    res += ts(x);
  }
  return res;
}

func ts(p: dynamic) -> dynamic
{
  return (((("(" + ts(p.FI)) + ", ") + ts(p.SE)) + ")");
}

func pr(x: dynamic) -> dynamic
{
  write(ts(x));
}

func pr(t: dynamic, u: dynamic...) -> dynamic
{
  pr(t);
  pr(cpp_expand(u));
}

func ps() -> dynamic
{
  pr("\n");
}

func ps(t: dynamic, u: dynamic...) -> dynamic
{
  pr(t);
  if (cpp_sizeof(u))
  {
    pr(" ");
  }
  ps(cpp_expand(u));
}

func DBG() -> dynamic
{
  write("]", "\n");
}

func DBG(t: dynamic, u: dynamic...) -> dynamic
{
  write(ts(t));
  if (cpp_sizeof(u))
  {
    write(", ");
  }
  DBG(cpp_expand(u));
}

func dbg() -> dynamic
{
  return cpp_expression("/* Author: Nguyen Tan Bao Status: Idea: */ #include <bits/stdc++.h> #d");
}

func chk() -> dynamic
{
  cpp_macro("if (!(__VA_ARGS__)) cerr << \"Line(\" << __LINE__ << \") -> function(\" \\\n        << __FUNCTION__  << \") -> CHK FAILED: (\" << #__VA_ARGS__ << \")\" << \"\\n\", exit(0);");
}

var PI: dynamic = acos(-1.0);

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

var EPS: dynamic = 1e-9;

var MODBASE: dynamic = 1000000007;

var INF: dynamic = 0x3f3f3f3f;

var MAXN: dynamic = 200010;

var MAXM: dynamic = 1000;

var MAXK: dynamic = 16;

var MAXQ: dynamic = 200010;

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(MAXN);

var b: dynamic = cpp_array(MAXN);

func main() -> dynamic
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  var te: dynamic = cpp_uninitialized();
  read(te);
  while (cpp_update(te, "--"))
  {
    read(n, c);
    FOR(i, 1, n);
    read(a[i]);
    FOR(i, 1, (n - 1));
    read(b[i]);
    var money: dynamic = 0;
    var days: dynamic = 0;
    var res: dynamic = 1e18;
    FOR(i, 1, n);
    {
      var resStay: dynamic = days;
      if ((money < c))
      {
        resStay += ((((c - money)) / a[i]) + (((((c - money)) % a[i]) > 0)));
      }
      res = min(res, resStay);
      if ((i < n))
      {
        var resMove: dynamic = days;
        if ((money < b[i]))
        {
          var incr: dynamic = ((((b[i] - money)) / a[i]) + (((((b[i] - money)) % a[i]) > 0)));
          money += (incr * a[i]);
          resMove += incr;
        }
        money -= b[i];
        days = (resMove + 1);
      }
    }
    write(res, "\n");
  }
  return 0;
}
