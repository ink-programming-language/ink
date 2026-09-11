// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

func fundamental_unit(d: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  var x: dynamic = 0;
  var y: dynamic = 1;
  var z: dynamic = 1;
  var sqrtd: dynamic = sqrt(d);
  var seen: dynamic = cpp_uninitialized();
  while (1)
  {
    if (seen.count(pplll(pll(x, y), z)))
    {
      break;
    }
    seen.insert(pplll(pll(x, y), z));
    var q: dynamic = floor((((x + (sqrtd * y))) / z));
    if (0)
    {
      write("x,y,z=(", x, "+", y, "*sqrt(d))/", z, "\n");
      write("q=", q, "\n");
    }
    ans.push_back(q);
    x -= (q * z);
    var norm: dynamic = ((x * x) - ((d * y) * y));
    y = (-y);
    z = (norm / z);
    if ((z < 0))
    {
      x = (-x);
      y = (-y);
      z = (-z);
    }
  }
  var num: dynamic = 0;
  var den: dynamic = 1;
  {
    var i: dynamic = (cpp_cast(ans.size()) - 2);
    while ((i >= 0))
    {
      var z: dynamic = (num + (ans[i] * den));
      num = den;
      den = z;
      i -= 1;
    }
  }
  if ((((den * den) - ((d * num) * num)) == -1))
  {
    var x: dynamic = ((den * den) + ((d * num) * num));
    var y: dynamic = ((2 * den) * num);
    den = x;
    num = y;
  }
  assert((((den * den) - ((d * num) * num)) == 1));
  return pll(den, num);
}

func solve(n: dynamic) -> dynamic
{
  cpp_statement("rep(i,200)");
  if (((i * i) == (2 * n)))
  {
    return pll(i, 1);
  }
  return fundamental_unit((2 * n));
}

func main() -> dynamic
{
  {
    var t: dynamic = 1;
    while (true)
    {
      var n: dynamic = cpp_uninitialized();
      read(n);
      if ((n == 0))
      {
        break;
      }
      var ans: dynamic = solve(n);
      write("Case ", t, ": ", ans.first, " ", ans.second, "\n");
      t += 1;
    }
  }
}
