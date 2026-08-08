// Translated from solution.cpp.

class rge
{
  var b: dynamic;
  var e: dynamic;
}

func range(i: dynamic, j: dynamic)
{
  return [i, j];
}

class debug
{
  func operator_shift_left(argument_0: dynamic)
  {
      return (*this);
    }
}

func min_self(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

func max_self(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

var inf = (1e9 + 5);

func calc(a: dynamic, b: dynamic, p: dynamic)
{
  return ((b / p) - (((a - 1)) / p));
}

func factor(n: dynamic)
{
  var f: dynamic;
  {
    var i = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        f.push_back(i);
        while (((n % i) == 0))
        {
          n /= i;
        }
      }
      i += 1;
    }
  }
  if ((n > 1))
  {
    f.push_back(n);
  }
  return f;
}

func test_case()
{
  var x: dynamic;
  var p: dynamic;
  var k: dynamic;
  read(x, p, k);
  var f = factor(p);
  var contrib: dynamic;
  var n = f.size();
  {
    var s = 1;
    while ((s < ((1 << n))))
    {
      var c = if (((builtin_popcount(s) % 2) == 1)) -1 else 1;
      var y = 1;
      {
        var i = 0;
        while ((i < n))
        {
          if (((s >> i) & 1))
          {
            y *= f[i];
          }
          i += 1;
        }
      }
      contrib.emplace_back(c, y);
      s += 1;
    }
  }
  var lo = (x + 1);
  var hi = inf;
  while ((lo < hi))
  {
    var mid = (lo + (((hi - lo)) / 2));
    var cnt = (mid - x);
    for (var pp in contrib)
    {
      cnt += (pp.first * calc((x + 1), mid, pp.second));
    }
    if ((cnt >= k))
    {
      hi = mid;
    } else
    {
      lo = (mid + 1);
    }
  }
  write(hi, cpp_char("\n"));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var tc: dynamic;
  read(tc);
  while (cpp_update(tc, "--"))
  {
    test_case();
  }
}
