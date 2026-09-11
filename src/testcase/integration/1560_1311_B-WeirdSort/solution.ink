// Translated from solution.cpp.

var INF: dynamic = 1000000009;

var M: dynamic = 1000000007;

var INFLL: dynamic = (cpp_cast(INF) * cpp_cast(INF));

var EPS: dynamic = 10e-9;

func ckmin(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

func ckmax(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

var rang: dynamic = cpp_construct(chrono.high_resolution_clock.now().time_since_epoch().count());

func re(x: dynamic) -> dynamic
{
  read(x);
}

func re(x: dynamic) -> dynamic
{
  var t: dynamic = cpp_uninitialized();
  re(t);
  x = stod(t);
}

func re(first: dynamic, rest: dynamic...) -> dynamic
{
  re(first);
  re(cpp_expand(rest));
}

func re(p: dynamic) -> dynamic
{
  re(p.f, p.s);
}

func re(a: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < int_cpp((a).size())))
    {
      re(a[i]);
      i += 1;
    }
  }
}

func re(a: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < SZ))
    {
      re(a[i]);
      i += 1;
    }
  }
}

class is_outputtable
{
  func test(argument_0: dynamic) -> dynamic
  {
      return true;
    }
  func test() -> dynamic
  {
      return false;
    }
  var value: dynamic = cpp_uninitialized();
}

func pr(x: dynamic) -> dynamic
{
  write(x);
}

func pr(first: dynamic, rest: dynamic...) -> dynamic
{
  pr(first);
  pr(cpp_expand(rest));
}

func prContain(x: dynamic) -> dynamic
{
  if (pretty)
  {
    pr("{");
  }
  var fst: dynamic = 1;
  for (var a: dynamic in x)
  {
    pr( ((!fst)) ?  (pretty) ? ", " : " " : "", a);
    fst = 0;
  }
  if (pretty)
  {
    pr("}");
  }
}

func pc(x: dynamic) -> dynamic
{
  prContain(x);
  pr("\n");
}

func pr(x: dynamic) -> dynamic
{
  pr("{", x.f, ", ", x.s, "}");
}

func pr(x: dynamic) -> dynamic
{
  prContain(x);
}

func ps() -> dynamic
{
  pr("\n");
}

func ps(first: dynamic) -> dynamic
{
  pr(first);
  ps();
}

func ps(first: dynamic, rest: dynamic...) -> dynamic
{
  pr(first, " ");
  ps(cpp_expand(rest));
}

func setIO() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  write(setprecision(15));
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(x);
}

func print(x: dynamic) -> dynamic
{
  write(cpp_char("'"), x, cpp_char("'"));
}

func print(x: dynamic) -> dynamic
{
  write(cpp_char("\\\""), x, cpp_char("\\\""));
}

func print(x: dynamic) -> dynamic
{
  write(cpp_char("\\\""), x, cpp_char("\\\""));
}

func print(x: dynamic) -> dynamic
{
  write(( (x) ? "true" : "false"));
}

func print(x: dynamic) -> dynamic
{
  write(cpp_char("{"));
  print(x.first);
  write(cpp_char(","));
  print(x.second);
  write(cpp_char("}"));
}

func print(x: dynamic) -> dynamic
{
  var f: dynamic = 0;
  write(cpp_char("{"));
  for (var i: dynamic in x)
  {
    write(( (cpp_update(f, "++")) ? "," : ""));
    print(i);
  }
  write("}");
}

func print() -> dynamic
{
  write("]\n");
}

func print(t: dynamic, v: dynamic...) -> dynamic
{
  print(t);
  if (cpp_sizeof(v))
  {
    write(", ");
  }
  print(cpp_expand(v));
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  re(n, m);
  re(a, p);
  sort(p.begin(), p.end());
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
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

func main() -> dynamic
{
  setIO();
  srand(chrono.high_resolution_clock.now().time_since_epoch().count());
  var t: dynamic = 1;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
