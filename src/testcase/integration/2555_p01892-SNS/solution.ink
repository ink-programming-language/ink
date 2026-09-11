// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func REP(i: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var PB: dynamic = cpp_expression("#include");

func read() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  scanf("%d", (&i));
  return i;
}

func chmin(t: dynamic, u: dynamic) -> dynamic
{
  if ((ll(t) > ll(u)))
  {
    t = u;
  }
}

func chmax(t: dynamic, u: dynamic) -> dynamic
{
  if ((ll(t) < ll(u)))
  {
    t = u;
  }
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var n: dynamic = cpp_uninitialized();
  read(a, b, n);
  var ans: dynamic = LLONG_MAX;
  {
    var q: dynamic = 1;
    while ((q <= n))
    {
      if (((b % q) != 0))
      {
        q += 1;
        continue;
      }
      var k: dynamic = (b / q);
      var x: dynamic = ((a / k) * k);
      var y: dynamic = (x + k);
      var z: dynamic = (k * n);
      if (((0 < x) && (x <= z)))
      {
        chmin(ans, abs((a - x)));
      }
      if (((0 < y) && (y <= z)))
      {
        chmin(ans, abs((a - y)));
      }
      if ((0 < z))
      {
        chmin(ans, abs((a - z)));
      }
      q += 1;
    }
  }
  write(ans, "\n");
}
