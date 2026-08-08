// Translated from solution.cpp.

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;i++)");
}

func REP(i: dynamic, b: dynamic)
{
  return cpp_expression("#include <");
}

var PB = cpp_expression("#include");

func read()
{
  var i: dynamic;
  scanf("%d", (&i));
  return i;
}

func chmin(t: dynamic, u: dynamic)
{
  if ((ll(t) > ll(u)))
  {
    t = u;
  }
}

func chmax(t: dynamic, u: dynamic)
{
  if ((ll(t) < ll(u)))
  {
    t = u;
  }
}

func main()
{
  var a: dynamic;
  var b: dynamic;
  var n: dynamic;
  read(a, b, n);
  var ans = LLONG_MAX;
  {
    var q = 1;
    while ((q <= n))
    {
      if (((b % q) != 0))
      {
        q += 1;
        continue;
      }
      var k = (b / q);
      var x = ((a / k) * k);
      var y = (x + k);
      var z = (k * n);
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
