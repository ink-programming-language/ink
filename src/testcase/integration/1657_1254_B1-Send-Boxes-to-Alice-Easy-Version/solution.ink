// Translated from solution.cpp.

var r: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(((cpp_cast(1e6) + 10)));

var b: dynamic = cpp_array(((cpp_cast(1e6) + 10)));

var sum: dynamic = 0;

func calc(l: dynamic, r: dynamic, x: dynamic, sum: dynamic) -> dynamic
{
  var res: dynamic = 0;
  {
    var i: dynamic = l;
    while ((i <= r))
    {
      var val: dynamic = min(sum, b[i]);
      sum -= val;
      res += (val * abs((i - x)));
      i += 1;
    }
  }
  return res;
}

func search(l: dynamic, r: dynamic, sum: dynamic) -> dynamic
{
  var s: dynamic = l;
  var e: dynamic = r;
  while (((r - l) >= 3))
  {
    var m1: dynamic = (l + (((r - l)) / 3));
    var m2: dynamic = (r - (((r - l)) / 3));
    var f1: dynamic = calc(s, e, m1, sum);
    var f2: dynamic = calc(s, e, m2, sum);
    if ((f1 < f2))
    {
      r = m2;
    } else
    {
      l = m1;
    }
  }
  var ans: dynamic = calc(s, e, l, sum);
  {
    var j: dynamic = max(l, s);
    while ((j <= min(e, r)))
    {
      ans = min(ans, calc(s, e, j, sum));
      j += 1;
    }
  }
  return ans;
}

func try_this(d: dynamic) -> dynamic
{
  var res: dynamic = 0;
  var l: dynamic = 0;
  var r: dynamic = -1;
  var s: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      b[i] = a[i];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      if ((s == 0))
      {
        l = i;
      }
      s += a[i];
      r = i;
      if (((s / d) > 0))
      {
        var ds: dynamic = (s - ((s % d)));
        res += search(l, r, ds);
        s = (s % d);
        b[i] = s;
        l = i;
      }
      i += 1;
    }
  }
  return res;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%lld", (&a[i]));
      sum += a[i];
      i += 1;
    }
  }
  if ((sum == 1))
  {
    printf("%d\n", -1);
    return 0;
  }
  var f: dynamic = cpp_uninitialized();
  var s2: dynamic = sum;
  {
    var i: dynamic = 2;
    while (((i * i) <= sum))
    {
      if (((s2 % i) == 0))
      {
        f.push_back(i);
      }
      while (((s2 % i) == 0))
      {
        s2 /= i;
      }
      i += 1;
    }
  }
  if ((s2 > 1))
  {
    f.push_back(s2);
  }
  b[0] = a[0];
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      b[i] += b[(i - 1)];
      b[i] += a[i];
      i += 1;
    }
  }
  var ans: dynamic = 1e18;
  {
    var j: dynamic = 0;
    while ((j < f.size()))
    {
      ans = min(ans, try_this(f[j]));
      j += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
