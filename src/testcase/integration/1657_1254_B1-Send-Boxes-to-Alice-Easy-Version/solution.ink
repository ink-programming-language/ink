// Translated from solution.cpp.

var r: dynamic;

var c: dynamic;

var k: dynamic;

var t: dynamic;

var n: dynamic;

var a = cpp_array(((cpp_cast(1e6) + 10)));

var b = cpp_array(((cpp_cast(1e6) + 10)));

var sum = 0;

func calc(l: dynamic, r: dynamic, x: dynamic, sum: dynamic)
{
  var res = 0;
  {
    var i = l;
    while ((i <= r))
    {
      var val = min(sum, b[i]);
      sum -= val;
      res += (val * abs((i - x)));
      i += 1;
    }
  }
  return res;
}

func search(l: dynamic, r: dynamic, sum: dynamic)
{
  var s = l;
  var e = r;
  while (((r - l) >= 3))
  {
    var m1 = (l + (((r - l)) / 3));
    var m2 = (r - (((r - l)) / 3));
    var f1 = calc(s, e, m1, sum);
    var f2 = calc(s, e, m2, sum);
    if ((f1 < f2))
    {
      r = m2;
    } else
    {
      l = m1;
    }
  }
  var ans = calc(s, e, l, sum);
  {
    var j = max(l, s);
    while ((j <= min(e, r)))
    {
      ans = min(ans, calc(s, e, j, sum));
      j += 1;
    }
  }
  return ans;
}

func try_this(d: dynamic)
{
  var res = 0;
  var l = 0;
  var r = -1;
  var s = 0;
  {
    var i = 0;
    while ((i < n))
    {
      b[i] = a[i];
      i += 1;
    }
  }
  {
    var i = 0;
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
        var ds = (s - ((s % d)));
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

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
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
  var f: dynamic;
  var s2 = sum;
  {
    var i = 2;
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
    var i = 1;
    while ((i < n))
    {
      b[i] += b[(i - 1)];
      b[i] += a[i];
      i += 1;
    }
  }
  var ans = 1e18;
  {
    var j = 0;
    while ((j < f.size()))
    {
      ans = min(ans, try_this(f[j]));
      j += 1;
    }
  }
  printf("%lld\n", ans);
  return 0;
}
