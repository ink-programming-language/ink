// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(111);

var u: dynamic = cpp_array(111);

func pr(x: dynamic) -> dynamic
{
  var y: dynamic = -1;
  var z: dynamic = -1;
  {
    var i: dynamic = x;
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
    var i: dynamic = x;
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  read(n);
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      var mx: dynamic = 0;
      var mi: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 0;
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
