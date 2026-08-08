// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((b > a))
  {
    return gcd(b, a);
  }
  return if ((b == 0)) a else gcd(b, (a % b));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  var n: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    read(n);
    var a = cpp_array((n + 1));
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        read(a[i]);
        i += 1;
      }
    }
    var count: dynamic;
    var ans = n;
    var d = 0;
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        if (count[a[i]])
        {
          ans = min(ans, ((i - count[a[i]]) + 1));
          d = 1;
        }
        count[a[i]] = i;
        i += 1;
      }
    }
    if (((ans == 1) || (d == 0)))
    {
      write(-1, "\n");
    } else
    {
      write(ans, "\n");
    }
  }
  return 0;
}
