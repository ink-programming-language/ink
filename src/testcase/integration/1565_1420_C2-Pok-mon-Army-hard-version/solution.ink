// Translated from solution.cpp.

var s: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(300005);

func maximum(k: dynamic, n: dynamic) -> dynamic
{
  if (((k > 0) && (a[k] < a[(k - 1)])))
  {
    return 0;
  }
  if (((k < (n - 1)) && (a[k] < a[(k + 1)])))
  {
    return 0;
  }
  return 1;
}

func minimum(k: dynamic, n: dynamic) -> dynamic
{
  if (((k == (n - 1)) || (k == 0)))
  {
    return 0;
  }
  if (((a[k] > a[(k - 1)]) || (a[k] > a[(k + 1)])))
  {
    return 0;
  }
  return 1;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    var sum: dynamic = 0;
    var n: dynamic = cpp_uninitialized();
    var q: dynamic = cpp_uninitialized();
    read(n, q);
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        if (maximum(i, n))
        {
          sum += a[i];
        }
        if (minimum(i, n))
        {
          sum -= a[i];
        }
        i += 1;
      }
    }
    write(sum, cpp_char("\n"));
    while (cpp_update(q, "--"))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      u -= 1;
      v -= 1;
      {
        var i: dynamic = -1;
        while ((i <= 1))
        {
          if ((((u + i) < 0) || ((u + i) >= n)))
          {
            i += 1;
            continue;
          }
          s.insert((u + i));
          i += 1;
        }
      }
      swap(u, v);
      {
        var i: dynamic = -1;
        while ((i <= 1))
        {
          if ((((u + i) < 0) || ((u + i) >= n)))
          {
            i += 1;
            continue;
          }
          s.insert((u + i));
          i += 1;
        }
      }
      for (var i: dynamic in s)
      {
        if (maximum(i, n))
        {
          sum -= a[i];
        }
        if (minimum(i, n))
        {
          sum += a[i];
        }
      }
      swap(a[u], a[v]);
      for (var i: dynamic in s)
      {
        if (maximum(i, n))
        {
          sum += a[i];
        }
        if (minimum(i, n))
        {
          sum -= a[i];
        }
      }
      write(sum, cpp_char("\n"));
      s.clear();
    }
  }
  return 0;
}
