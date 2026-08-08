// Translated from solution.cpp.

var FI = cpp_expression("/*");

var SE = cpp_expression("/*");

func ALL(a: dynamic)
{
  return cpp_expression("/* Author: Ngu");
}

func SZ(a: dynamic)
{
  return cpp_expression("/* Author:");
}

func MS(s: dynamic, n: dynamic)
{
  return cpp_expression("/* Author: Nguyen T");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i = (a); i <= (b); i++)");
}

func FORE(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for (int i = (a); i >= (b); i--)");
}

func FORALL(it: dynamic, a: dynamic)
{
  cpp_macro("for (__typeof((a).begin()) it = (a).begin(); it != (a).end(); it++)");
}

func TRAV(x: dynamic, a: dynamic)
{
  return cpp_expression("/* Author: Ng");
}

func ckmin(a: dynamic, val: dynamic)
{
  return if ((val < a)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func ckmax(a: dynamic, val: dynamic)
{
  return if ((a < val)) cpp_comma(cpp_assign(a, "=", val), 1) else 0;
}

func remDup(v: dynamic)
{
  sort(ALL(v));
  v.erase(unique(ALL(v)), end(v));
}

func pct(x: dynamic)
{
  return builtin_popcount(x);
}

func bits(x: dynamic)
{
  return if ((x == 0)) 0 else (31 - builtin_clz(x));
}

func p2(x: dynamic)
{
  return (1 << x);
}

func msk2(x: dynamic)
{
  return (p2(x) - 1);
}

func ceilDiv(a: dynamic, b: dynamic)
{
  return ((a / b) + (((((a ^ b)) > 0) && (a % b))));
}

func floorDiv(a: dynamic, b: dynamic)
{
  return ((a / b) - (((((a ^ b)) < 0) && (a % b))));
}

func setPrec(x: dynamic)
{
  write(fixed, setprecision(x));
}

var ts = cpp_expression("/* Au");

func ts(c: dynamic)
{
  return string_cpp(1, c);
}

func ts(s: dynamic)
{
  return cpp_cast(s);
}

func ts(s: dynamic)
{
  return s;
}

func ts(b: dynamic)
{
  return ts(cpp_cast(b));
}

func ts(c: dynamic)
{
  var ss: dynamic;
  (ss << c);
  return ss.str();
}

func ts(v: dynamic)
{
  var res = "{";
  FOR(i, 0, (SZ(v) - 1)) += char((cpp_char("0") + v[i]));
  res += "}";
  return res;
}

func ts(b: dynamic)
{
  var res = "";
  FOR(i, 0, (SZ(b) - 1)) += char((cpp_char("0") + b[i]));
  return res;
}

func ts(v: dynamic)
{
  var fst = 1;
  var res = "";
  for (var x in v)
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

func ts(p: dynamic)
{
  return (((("(" + ts(p.FI)) + ", ") + ts(p.SE)) + ")");
}

func pr(x: dynamic)
{
  write(ts(x));
}

func pr(t: dynamic, u: dynamic...)
{
  pr(t);
  pr(cpp_expand(u));
}

func ps()
{
  pr("\n");
}

func ps(t: dynamic, u: dynamic...)
{
  pr(t);
  if (cpp_sizeof(u))
  {
    pr(" ");
  }
  ps(cpp_expand(u));
}

func DBG()
{
  write("]", "\n");
}

func DBG(t: dynamic, u: dynamic...)
{
  write(ts(t));
  if (cpp_sizeof(u))
  {
    write(", ");
  }
  DBG(cpp_expand(u));
}

func dbg()
{
  return cpp_expression("/* Author: Nguyen Tan Bao Status: Idea: */ #include <bits/stdc++.h> #d");
}

func chk()
{
  cpp_macro("if (!(__VA_ARGS__)) cerr << \"Line(\" << __LINE__ << \") -> function(\" \\\n        << __FUNCTION__  << \") -> CHK FAILED: (\" << #__VA_ARGS__ << \")\" << \"\\n\", exit(0);");
}

var PI = acos(-1.0);

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

var EPS = 1e-9;

var MODBASE = 1000000007;

var INF = 0x3f3f3f3f;

var MAXN = 200010;

var MAXM = 1000;

var MAXK = 16;

var MAXQ = 200010;

var n: dynamic;

var c: dynamic;

var a = cpp_array(MAXN);

var b = cpp_array(MAXN);

func main()
{
  ios.sync_with_stdio(0);
  cin.tie(null);
  var te: dynamic;
  read(te);
  while (cpp_update(te, "--"))
  {
    read(n, c);
    FOR(i, 1, n);
    read(a[i]);
    FOR(i, 1, (n - 1));
    read(b[i]);
    var money = 0;
    var days = 0;
    var res = 1e18;
    FOR(i, 1, n);
    {
      var resStay = days;
      if ((money < c))
      {
        resStay += ((((c - money)) / a[i]) + (((((c - money)) % a[i]) > 0)));
      }
      res = min(res, resStay);
      if ((i < n))
      {
        var resMove = days;
        if ((money < b[i]))
        {
          var incr = ((((b[i] - money)) / a[i]) + (((((b[i] - money)) % a[i]) > 0)));
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
