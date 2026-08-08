// Translated from solution.cpp.

func main()
{
  write(fixed, setprecision(10));
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  var d: dynamic;
  read(n, d);
  var a = cpp_array(n);
  var b = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      b[i] = (if (i) (b[(i - 1)] + a[i]) else a[i]);
      if ((b[i] > d))
      {
        write(-1);
        return 0;
      }
      i += 1;
    }
  }
  var maxi = cpp_array((n + 1));
  maxi[n] = 0;
  maxi[(n - 1)] = b[(n - 1)];
  {
    var i = (n - 2);
    while ((i >= 0))
    {
      maxi[i] = max(b[i], maxi[(i + 1)]);
      i -= 1;
    }
  }
  var added = 0;
  var ans = 0;
  if (((a[0] == 0) && (b[0] < 0)))
  {
    added += (d - maxi[0]);
    if (((b[0] + added) < 0))
    {
      write(-1);
      return 0;
    }
    ans += 1;
  }
  {
    var i = 1;
    while ((i < n))
    {
      b[i] += added;
      if (((a[i] == 0) && (b[i] < 0)))
      {
        var here = ((d - maxi[i]) - added);
        added += here;
        ans += 1;
        b[i] += here;
        if ((b[i] < 0))
        {
          write(-1);
          return 0;
        }
      }
      i += 1;
    }
  }
  write(ans);
}
