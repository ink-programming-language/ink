// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);++i)");
}

func fundamental_unit(d: dynamic)
{
  var ans: dynamic;
  var x = 0;
  var y = 1;
  var z = 1;
  var sqrtd = sqrt(d);
  var seen: dynamic;
  while (1)
  {
    if (seen.count(pplll(pll(x, y), z)))
    {
      break;
    }
    seen.insert(pplll(pll(x, y), z));
    var q = floor((((x + (sqrtd * y))) / z));
    if (0)
    {
      write("x,y,z=(", x, "+", y, "*sqrt(d))/", z, "\n");
      write("q=", q, "\n");
    }
    ans.push_back(q);
    x -= (q * z);
    var norm = ((x * x) - ((d * y) * y));
    y = (-y);
    z = (norm / z);
    if ((z < 0))
    {
      x = (-x);
      y = (-y);
      z = (-z);
    }
  }
  var num = 0;
  var den = 1;
  {
    var i = (cpp_cast(ans.size()) - 2);
    while ((i >= 0))
    {
      var z = (num + (ans[i] * den));
      num = den;
      den = z;
      i -= 1;
    }
  }
  if ((((den * den) - ((d * num) * num)) == -1))
  {
    var x = ((den * den) + ((d * num) * num));
    var y = ((2 * den) * num);
    den = x;
    num = y;
  }
  assert((((den * den) - ((d * num) * num)) == 1));
  return pll(den, num);
}

func solve(n: dynamic)
{
  cpp_statement("rep(i,200)");
  if (((i * i) == (2 * n)))
  {
    return pll(i, 1);
  }
  return fundamental_unit((2 * n));
}

func main()
{
  {
    var t = 1;
    while (true)
    {
      var n: dynamic;
      read(n);
      if ((n == 0))
      {
        break;
      }
      var ans = solve(n);
      write("Case ", t, ": ", ans.first, " ", ans.second, "\n");
      t += 1;
    }
  }
}
