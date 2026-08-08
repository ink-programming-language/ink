// Translated from solution.cpp.

var INF = 1000000009;

var M = 1000000007;

var INFLL = (cpp_cast(INF) * cpp_cast(INF));

var EPS = 10e-9;

func ckmin(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

func ckmax(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

var rang = cpp_construct(chrono.high_resolution_clock.now().time_since_epoch().count());

func re(x: dynamic)
{
  read(x);
}

func re(x: dynamic)
{
  var t: dynamic;
  re(t);
  x = stod(t);
}

func re(first: dynamic, rest: dynamic...)
{
  re(first);
  re(cpp_expand(rest));
}

func re(p: dynamic)
{
  re(p.f, p.s);
}

func re(a: dynamic)
{
  {
    var i = 0;
    while ((i < int_cpp((a).size())))
    {
      re(a[i]);
      i += 1;
    }
  }
}

func re(a: dynamic)
{
  {
    var i = 0;
    while ((i < SZ))
    {
      re(a[i]);
      i += 1;
    }
  }
}

class is_outputtable
{
  func test(argument_0: dynamic)
  {
      return true;
    }
  func test()
  {
      return false;
    }
  var value: dynamic;
}

func pr(x: dynamic)
{
  write(x);
}

func pr(first: dynamic, rest: dynamic...)
{
  pr(first);
  pr(cpp_expand(rest));
}

func prContain(x: dynamic)
{
  if (pretty)
  {
    pr("{");
  }
  var fst = 1;
  for (var a in x)
  {
    pr(if ((!fst)) if (pretty) ", " else " " else "", a);
    fst = 0;
  }
  if (pretty)
  {
    pr("}");
  }
}

func pc(x: dynamic)
{
  prContain(x);
  pr("\n");
}

func pr(x: dynamic)
{
  pr("{", x.f, ", ", x.s, "}");
}

func pr(x: dynamic)
{
  prContain(x);
}

func ps()
{
  pr("\n");
}

func ps(first: dynamic)
{
  pr(first);
  ps();
}

func ps(first: dynamic, rest: dynamic...)
{
  pr(first, " ");
  ps(cpp_expand(rest));
}

func setIO()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(setprecision(15));
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(x);
}

func print(x: dynamic)
{
  write(cpp_char("'"), x, cpp_char("'"));
}

func print(x: dynamic)
{
  write(cpp_char("\\\""), x, cpp_char("\\\""));
}

func print(x: dynamic)
{
  write(cpp_char("\\\""), x, cpp_char("\\\""));
}

func print(x: dynamic)
{
  write((if (x) "true" else "false"));
}

func print(x: dynamic)
{
  write(cpp_char("{"));
  print(x.first);
  write(cpp_char(","));
  print(x.second);
  write(cpp_char("}"));
}

func print(x: dynamic)
{
  var f = 0;
  write(cpp_char("{"));
  for (var i in x)
  {
    write((if (cpp_update(f, "++")) "," else ""));
    print(i);
  }
  write("}");
}

func print()
{
  write("]\n");
}

func print(t: dynamic, v: dynamic...)
{
  print(t);
  if (cpp_sizeof(v))
  {
    write(", ");
  }
  print(cpp_expand(v));
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  re(n, m);
  re(a, p);
  sort(p.begin(), p.end());
  var i: dynamic;
  var j: dynamic;
  {
    i = 0;
    while ((i < (n - 1)))
    {
      {
        j = 0;
        while ((j < ((n - i) - 1)))
        {
          if ((a[j] > a[(j + 1)]))
          {
            if (binary_search(p.begin(), p.end(), (j + 1)))
            {
              swap(a[j], a[(j + 1)]);
            } else
            {
              ps("NO");
              return;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  ps("YES");
}

func main()
{
  setIO();
  srand(chrono.high_resolution_clock.now().time_since_epoch().count());
  var t = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
