// Translated from solution.cpp.

var N = (2e5 + 5);

var mod = (1e9 + 7);

var inf = 1e18;

var n: dynamic;

var a = cpp_array(N);

var l: dynamic;

var r: dynamic;

var b = cpp_array(N);

func first(vl: dynamic)
{
  {
    var i = 1;
    while ((i <= n))
    {
      b[i] = ((1.0 * a[i]) - vl);
      i += 1;
    }
  }
  var sum: dynamic;
  var minsum: dynamic;
  var ans = (-inf);
  sum = cpp_assign(minsum, "=", 0);
  {
    var i = 1;
    while ((i <= n))
    {
      sum += b[i];
      ans = max(ans, fabs((sum - minsum)));
      minsum = min(minsum, sum);
      i += 1;
    }
  }
  var maxsum = 0;
  sum = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      sum += b[i];
      ans = max(ans, fabs((sum - maxsum)));
      maxsum = max(maxsum, sum);
      i += 1;
    }
  }
  return ans;
}

func main()
{
  ios_base.sync_with_stdio(false);
  read(n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  l = (-N);
  r = N;
  {
    var i = 0;
    while ((i < 100))
    {
      var m1 = (l + (((r - l)) / 3.0));
      var m2 = (r - (((r - l)) / 3.0));
      if ((first(m1) >= first(m2)))
      {
        l = m1;
      } else
      {
        r = m2;
      }
      i += 1;
    }
  }
  printf("%.12f", min(first(l), first(r)));
  return 0;
}
