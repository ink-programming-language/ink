// Translated from solution.cpp.

class rge
{
  var b: dynamic = cpp_uninitialized();
  var e: dynamic = cpp_uninitialized();
}

func range(i: dynamic, j: dynamic) -> dynamic
{
  return [i, j];
}

class debug
{
  func operator_shift_left(argument_0: dynamic) -> dynamic
  {
      return (*self);
    }
}

func min_self(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

func max_self(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

var inf: dynamic = (1e9 + 5);

func calc(a: dynamic, b: dynamic, p: dynamic) -> dynamic
{
  return ((b / p) - (((a - 1)) / p));
}

func factor(n: dynamic) -> dynamic
{
  var f: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 2;
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

func test_case() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(x, p, k);
  var f: dynamic = factor(p);
  var contrib: dynamic = cpp_uninitialized();
  var n: dynamic = f.size();
  {
    var s: dynamic = 1;
    while ((s < ((1 << n))))
    {
      var c: dynamic =  (((builtin_popcount(s) % 2) == 1)) ? -1 : 1;
      var y: dynamic = 1;
      {
        var i: dynamic = 0;
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
  var lo: dynamic = (x + 1);
  var hi: dynamic = inf;
  while ((lo < hi))
  {
    var mid: dynamic = (lo + (((hi - lo)) / 2));
    var cnt: dynamic = (mid - x);
    for (var pp: dynamic in contrib)
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var tc: dynamic = cpp_uninitialized();
  read(tc);
  while (cpp_update(tc, "--"))
  {
    test_case();
  }
}
