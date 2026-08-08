// Translated from solution.cpp.

var MOD = 1000000007;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solve();
    write("\n");
  }
  write("time taken : ", (cpp_cast(clock()) / CLOCKS_PER_SEC), " secs", "\n");
  return 0;
}

func ceils(x: dynamic, y: dynamic)
{
  return ((x / y) + ((((x % y)) != 0)));
}

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  } else
  {
    return gcd(b, (a % b));
  }
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a / gcd(a, b)) * b);
}

func solve()
{
  var n: dynamic;
  read(n);
  var a = cpp_array(n);
  var b = cpp_array(n);
  var c = cpp_array(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(b[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      read(c[i]);
      i += 1;
    }
  }
  var ans = cpp_array(n);
  ans[0] = a[0];
  {
    var i = 1;
    while ((i < n))
    {
      ans[i] = a[i];
      if ((ans[i] == ans[(i - 1)]))
      {
        ans[i] = b[i];
      }
      if ((ans[i] == ans[(i - 1)]))
      {
        ans[i] = c[i];
      }
      i += 1;
    }
  }
  var ok = 0;
  {
    var i = 1;
    while ((i < n))
    {
      if ((ans[i] == ans[(i - 1)]))
      {
        ok = 1;
        break;
      }
      i += 1;
    }
  }
  if (ok)
  {
    ans[0] = b[0];
    {
      var i = 1;
      while ((i < n))
      {
        ans[i] = b[i];
        if ((ans[i] == ans[(i - 1)]))
        {
          ans[i] = a[i];
        }
        if ((ans[i] == ans[(i - 1)]))
        {
          ans[i] = c[i];
        }
        i += 1;
      }
    }
    ok = 0;
    {
      var i = 1;
      while ((i < n))
      {
        if ((ans[i] == ans[(i - 1)]))
        {
          ok = 1;
          break;
        }
        i += 1;
      }
    }
  }
  if (ok)
  {
    ans[0] = c[0];
    {
      var i = 1;
      while ((i < n))
      {
        ans[i] = c[i];
        if ((ans[i] == ans[(i - 1)]))
        {
          ans[i] = a[i];
        }
        if ((ans[i] == ans[(i - 1)]))
        {
          ans[i] = b[i];
        }
        i += 1;
      }
    }
    ok = 0;
    {
      var i = 1;
      while ((i < n))
      {
        if ((ans[i] == ans[(i - 1)]))
        {
          ok = 1;
          break;
        }
        i += 1;
      }
    }
  }
  if ((ans[(n - 1)] == ans[0]))
  {
    if ((ans[(n - 1)] == a[(n - 1)]))
    {
      if ((ans[(n - 2)] == b[(n - 1)]))
      {
        ans[(n - 1)] = c[(n - 1)];
      } else
      {
        ans[(n - 1)] = b[(n - 1)];
      }
    } else if ((ans[(n - 1)] == b[(n - 1)]))
    {
      if ((ans[(n - 2)] == a[(n - 1)]))
      {
        ans[(n - 1)] = c[(n - 1)];
      } else
      {
        ans[(n - 1)] = a[(n - 1)];
      }
    } else
    {
      if ((ans[(n - 2)] == b[(n - 1)]))
      {
        ans[(n - 1)] = a[(n - 1)];
      } else
      {
        ans[(n - 1)] = b[(n - 1)];
      }
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write(ans[i], " ");
      i += 1;
    }
  }
}
