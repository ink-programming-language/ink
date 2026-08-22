// Translated from solution.cpp.

func REP(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(x);i++)");
}

func FOR(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin())i=(c).begin();i!=(c).end();i++)");
}

func RREP(i: dynamic, x: dynamic)
{
  cpp_macro("for(int i=((int)(x)-1);i>=0;i--)");
}

func RFOR(i: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).rbegin())i=(c).rbegin();i!=(c).rend();i++)");
}

func ALL(container: dynamic)
{
  return cpp_expression("#include <cstdio> #include <cmath>");
}

func RALL(container: dynamic)
{
  return cpp_expression("#include <cstdio> #include <cmath> #");
}

func SZ(container: dynamic)
{
  return cpp_expression("#include <cstdio> #incl");
}

func mp(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <cstdi");
}

func UNIQUE(v: dynamic)
{
  cpp_macro("v.erase( unique(v.begin(), v.end()), v.end() );");
}

func chmax(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func chmin(a: dynamic, b: dynamic)
{
  if ((a > b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "[");
  (os << "]");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  (os << "{");
  (os << "}");
  return os;
}

func operator_shift_left(os: dynamic, t: dynamic)
{
  return (((((os << "(") << t.first) << ",") << t.second) << ")");
}

var INF = (1 << 28);

var EPS = 1e-8;

var MOD = 1000000007;

var n: dynamic;

var s: dynamic;

var ParseFailed = false;

func num(p: dynamic)
{
  var res = 0;
  if ((p >= s.size()))
  {
    ParseFailed = true;
    return 0;
  }
  if ((s[p] == cpp_char("(")))
  {
    res = parse(cpp_update(p, "++"), 4);
    if (((p >= s.size()) || (s[cpp_update(p, "++")] != cpp_char(")"))))
    {
      ParseFailed = true;
      return 0;
    }
  } else
  {
    if (((!isdigit(s[p])) || (s[p] == cpp_char("0"))))
    {
      ParseFailed = true;
      return 0;
    }
    {
      while (isdigit(s[p]))
      {
        res = (((res * 10) + s[p]) - cpp_char("0"));
        p += 1;
      }
    }
  }
  return res;
}

func parse(p: dynamic, d: dynamic = 4)
{
  var res = if (d) parse(p, (d - 1)) else num(p);
  while ((p < s.size()))
  {
    if (((d == 0) && (s[p] == cpp_char("*"))))
    {
      res *= num(cpp_update(p, "++"));
    } else if (((d == 1) && (s[p] == cpp_char("+"))))
    {
      res += parse(cpp_update(p, "++"), 0);
    } else if (((d == 1) && (s[p] == cpp_char("-"))))
    {
      res -= parse(cpp_update(p, "++"), 0);
    } else if (((d == 2) && (s[p] == cpp_char("&"))))
    {
      res &= parse(cpp_update(p, "++"), 1);
    } else if (((d == 3) && (s[p] == cpp_char("^"))))
    {
      res ^= parse(cpp_update(p, "++"), 2);
    } else if (((d == 4) && (s[p] == cpp_char("|"))))
    {
      res |= parse(cpp_update(p, "++"), 3);
    } else
    {
      break;
    }
  }
  return res;
}

func isExpressionValid()
{
  ParseFailed = false;
  var Position = 0;
  parse(Position);
  return ((!ParseFailed) && (Position == s.size()));
}

var tbl = "()*+-&^|0123456789";

func Max(t: dynamic, rest: dynamic)
{
  s = t;
  var p = 0;
  var res = (-INF);
  if ((rest == 0))
  {
    return parse(p);
  }
  REP(i, (t.size() + 1));
  REP(j, tbl.size());
  {
    s = t;
    s.insert((s.begin() + i), tbl[j]);
    var f = isExpressionValid();
    if (f)
    {
      res = max(res, Min(s, (rest - 1)));
    }
  }
  REP(i, t.size());
  {
    s = t;
    s.erase((s.begin() + i));
    var f = isExpressionValid();
    if (f)
    {
      res = max(res, Min(s, (rest - 1)));
    }
  }
  return res;
}

func Min(t: dynamic, rest: dynamic)
{
  s = t;
  var p = 0;
  var res = INF;
  if ((rest == 0))
  {
    return parse(p);
  }
  REP(i, (t.size() + 1));
  REP(j, tbl.size());
  {
    s = t;
    s.insert((s.begin() + i), tbl[j]);
    var f = isExpressionValid();
    if (f)
    {
      res = min(res, Max(s, (rest - 1)));
    }
  }
  REP(i, t.size());
  {
    s = t;
    s.erase((s.begin() + i));
    var f = isExpressionValid();
    if (f)
    {
      res = min(res, Max(s, (rest - 1)));
    }
  }
  return res;
}

func main()
{
  var s: dynamic;
  while (cpp_comma(((cin >> n) >> s), n))
  {
    var ans: dynamic;
    var p = 0;
    var res = -1;
    while ((n > 2))
    {
      n -= 2;
    }
    write(Max(s, n), "\n");
  }
  return 0;
}

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }

func FOR(argument_0: dynamic, argument_1: dynamic)
{
    if ((it != t.begin()))
    {
      (os << ",");
    }
    (os << (*it));
  }
