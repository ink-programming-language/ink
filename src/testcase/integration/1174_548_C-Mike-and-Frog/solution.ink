// Translated from solution.cpp.

func FastIO()
{
  ios_base.sync_with_stdio(0);
  cin.tie(null);
  cout.tie(null);
}

func main()
{
  FastIO();
  var m: dynamic;
  read(m);
  var h = cpp_array(2);
  var a = cpp_array(2);
  var x = cpp_array(2);
  var y = cpp_array(2);
  {
    var i = 0;
    while ((i < cpp_cast(2)))
    {
      read(h[i], a[i], x[i], y[i]);
      i += 1;
    }
  }
  var steps1 = 0;
  {
    var i = cpp_cast(1);
    while ((i <= cpp_cast(m)))
    {
      h[0] = ((((x[0] * h[0]) + y[0])) % m);
      if ((h[0] == a[0]))
      {
        steps1 = i;
        break;
      }
      i += 1;
    }
  }
  if ((!steps1))
  {
    write("-1", cpp_char("\n"));
    return 0;
  }
  var steps2 = 0;
  {
    var i = cpp_cast(1);
    while ((i <= cpp_cast(m)))
    {
      h[0] = ((((x[0] * h[0]) + y[0])) % m);
      if ((h[0] == a[0]))
      {
        steps2 = i;
        break;
      }
      i += 1;
    }
  }
  var steps3 = 0;
  {
    var i = cpp_cast(1);
    while ((i <= cpp_cast(m)))
    {
      h[1] = ((((x[1] * h[1]) + y[1])) % m);
      if ((h[1] == a[1]))
      {
        steps3 = i;
        break;
      }
      i += 1;
    }
  }
  if ((!steps3))
  {
    write("-1", cpp_char("\n"));
    return 0;
  }
  var steps4 = 0;
  {
    var i = cpp_cast(1);
    while ((i <= cpp_cast(m)))
    {
      h[1] = ((((x[1] * h[1]) + y[1])) % m);
      if ((h[1] == a[1]))
      {
        steps4 = i;
        break;
      }
      i += 1;
    }
  }
  var ans = 1e18;
  if ((steps2 && steps4))
  {
    {
      var i = 0;
      while ((i < cpp_cast(1e7)))
      {
        if (((((((steps1 + (i * steps2)) - steps3)) % steps4) == 0) && ((((steps1 + (i * steps2)) - steps3)) >= 0)))
        {
          ans = min(ans, (steps1 + (i * steps2)));
          break;
        }
        i += 1;
      }
    }
  } else if (steps2)
  {
    {
      var i = 0;
      while ((i < cpp_cast(1e7)))
      {
        if ((((steps1 + (i * steps2)) == steps3)))
        {
          ans = min(ans, (steps1 + (i * steps2)));
          break;
        }
        i += 1;
      }
    }
  } else if (steps4)
  {
    {
      var i = 0;
      while ((i < cpp_cast(1e7)))
      {
        if ((((steps3 + (i * steps4)) == steps1)))
        {
          ans = min(ans, (steps3 + (i * steps4)));
          break;
        }
        i += 1;
      }
    }
  } else
  {
    ans = min(ans, (steps1 * ((steps1 == steps3))));
  }
  if ((ans == 1e18))
  {
    ans = -1;
  }
  write(ans, cpp_char("\n"));
  return 0;
}
