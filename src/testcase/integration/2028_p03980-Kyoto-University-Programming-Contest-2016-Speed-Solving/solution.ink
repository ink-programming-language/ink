// Translated from solution.cpp.

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 0; i < (n); ++i)");
}

func rrep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = 1; i <= (n); ++i)");
}

func drep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i = (n)-1; i >= 0; --i)");
}

func gep(i: dynamic, g: dynamic, j: dynamic)
{
  cpp_macro("for(int i = g.head[j]; i != -1; i = g.e[i].next)");
}

func each(it: dynamic, c: dynamic)
{
  cpp_macro("for(__typeof((c).begin()) it=(c).begin();it!=(c).end();it++)");
}

func rng(a: dynamic)
{
  return cpp_expression("#include <cstdio>");
}

func maxs(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <cs");
}

func mins(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <cs");
}

var pb = cpp_expression("#include");

func sz(x: dynamic)
{
  return cpp_expression("#include <cstdi");
}

var pcnt = cpp_expression("#include <cstdio>");

func uni(x: dynamic)
{
  return cpp_expression("#include <cstdio> #include <alg");
}

var snuke = cpp_expression("#include <cstdio> #include <algorithm> #includ");

func df(x: dynamic)
{
  return cpp_expression("#include <cs");
}

var dame = cpp_expression("#include <cstdio> #incl");

func show(x: dynamic)
{
  cpp_macro("cout<<#x<<\" = \"<<x<<endl;");
}

func PQ(T: dynamic)
{
  return cpp_expression("#include <cstdio> #include <algorithm>");
}

func in_cpp()
{
  var x: dynamic;
  scanf("%d", (&x));
  return x;
}

func priv(a: dynamic)
{
  rep(i, sz(a));
  printf("%d%c", a[i], if ((i == (sz(a) - 1))) cpp_char("\n") else cpp_char(" "));
}

func operator_shift_right(i: dynamic, v: dynamic)
{
  rep(j, sz(v));
  (i >> v[j]);
  return i;
}

func join(v: dynamic)
{
  var s: dynamic;
  ((rep(i, sz(v)) << cpp_char(" ")) << v[i]);
  return s.str().substr(1);
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  if (sz(v))
  {
    (o << join(v));
  }
  return o;
}

func operator_shift_right(i: dynamic, v: dynamic)
{
  return ((i >> v.fi) >> v.se);
}

func operator_shift_left(o: dynamic, v: dynamic)
{
  return (((o << v.fi) << " ") << v.se);
}

var MX = 100005;

var INF = 1001001001;

var LINF = 1e18;

var eps = 1e-10;

class Parse
{
  var n: dynamic;
  var s: dynamic;
  var i: dynamic;
  func parse()
  {
      {
        n = 1;
        while ((n <= sz(s)))
        {
          i = 0;
          var p = expr();
          if ((p.fi == p.se))
          {
            return P(p.fi, n);
          }
          n += 1;
        }
      }
      return P(-1, -1);
    }
  func expr()
  {
      if ((i >= n))
      {
        return P(0, 99);
      }
      if (isdigit(s[i]))
      {
        return num();
      }
      if ((s[i] == cpp_char("_")))
      {
        i += 1;
        if ((i >= n))
        {
          return P(0, 99);
        }
        i += 1;
        var x = expr();
        if ((i >= n))
        {
          return P(0, x.se);
        }
        i += 1;
        var y = expr();
        i += 1;
        return P(min(x.fi, y.fi), min(x.se, y.se));
      } else
      {
        i += 1;
        if ((i >= n))
        {
          return P(0, 99);
        }
        i += 1;
        var x = expr();
        if ((i >= n))
        {
          return P(x.fi, 99);
        }
        i += 1;
        var y = expr();
        i += 1;
        return P(max(x.fi, y.fi), max(x.se, y.se));
      }
    }
  func num()
  {
      if ((i >= n))
      {
        return P(0, 99);
      }
      if ((s[i] == cpp_char("0")))
      {
        i += 1;
        return P(0, 0);
      } else
      {
        var x = (s[i] - cpp_char("0"));
        i += 1;
        if ((i >= n))
        {
          return P(x, ((x * 10) + 9));
        }
        if (isdigit(s[i]))
        {
          x = (((x * 10) + s[i]) - cpp_char("0"));
          i += 1;
        }
        return P(x, x);
      }
    }
}

func main()
{
  var ts: dynamic;
  read(ts);
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    var p: dynamic;
    read(p.s);
    var ans = p.parse();
    write(ans, "\n");
  }
