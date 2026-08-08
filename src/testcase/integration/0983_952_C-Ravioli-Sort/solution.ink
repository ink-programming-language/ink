// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(111);

var u = cpp_array(111);

func pr(x: dynamic)
{
  var y = -1;
  var z = -1;
  {
    var i = x;
    while ((i < n))
    {
      if ((!u[i]))
      {
        y = a[i];
        break;
      }
      i += 1;
    }
  }
  {
    var i = x;
    while ((i >= 0))
    {
      if ((!u[i]))
      {
        z = a[i];
        break;
      }
      i -= 1;
    }
  }
  if (((y < 0) || (z < 0)))
  {
    return 0;
  } else
  {
    return (abs((y - z)) > 1);
  }
}

func main()
{
  ios_base.sync_with_stdio(0);
  read(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((i > 0))
      {
        if ((abs((a[i] - a[(i - 1)])) > 1))
        {
          return cpp_comma((cout << "NO"), 0);
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      var mx = 0;
      var mi: dynamic;
      {
        var j = 0;
        while ((j < n))
        {
          if (((a[j] > mx) && (!u[j])))
          {
            mx = a[j];
            mi = j;
          }
          j += 1;
        }
      }
      u[mi] = 1;
      if (pr(mi))
      {
        return cpp_comma((cout << "NO"), 0);
      }
      i += 1;
    }
  }
  write("YES");
  return 0;
}
