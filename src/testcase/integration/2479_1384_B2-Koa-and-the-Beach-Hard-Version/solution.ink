// Translated from solution.cpp.

func dep(t: dynamic, k: dynamic)
{
  t %= ((2 * k));
  if ((t <= k))
  {
    return t;
  } else
  {
    return (((2 * k) - t));
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var n: dynamic;
    var k: dynamic;
    var l: dynamic;
    read(n, k, l);
    var d = cpp_construct((n + 1), 0);
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        read(d[i]);
        i += 1;
      }
    }
    var fl = 0;
    var safe = cpp_construct((n + 1));
    safe[0] = 1;
    {
      var i = 1;
      while ((i < (n + 1)))
      {
        if ((d[i] > l))
        {
          fl = 1;
          break;
        }
        if (((d[i] + k) <= l))
        {
          safe[i] = 1;
        } else
        {
          safe[i] = 0;
        }
        i += 1;
      }
    }
    if (fl)
    {
      write("No", "\n");
      continue;
    }
    fl = 0;
    var t = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if (safe[i])
        {
          if (safe[(i + 1)])
          {
            i += 1;
            continue;
          } else
          {
            var h = (((d[(i + 1)] + k)) - l);
            t = (((k + h)) % ((2 * k)));
          }
        } else
        {
          if (safe[(i + 1)])
          {
            i += 1;
            continue;
          } else
          {
            if (((d[(i + 1)] + dep((t + 1), k)) <= l))
            {
              t += 1;
              t %= ((2 * k));
            } else
            {
              if ((t < k))
              {
                fl = 1;
                break;
              } else
              {
                var h = ((d[(i + 1)] + dep((t + 1), k)) - l);
                t = (((t + h)) % ((2 * k)));
                t += 1;
                t %= ((2 * k));
              }
            }
          }
        }
        i += 1;
      }
    }
    if (fl)
    {
      write("No", "\n");
    } else
    {
      write("Yes", "\n");
    }
  }
}
