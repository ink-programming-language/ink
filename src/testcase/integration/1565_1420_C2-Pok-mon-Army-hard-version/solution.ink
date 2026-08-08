// Translated from solution.cpp.

var s: dynamic;

var a = cpp_array(300005);

func maximum(k: dynamic, n: dynamic)
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

func minimum(k: dynamic, n: dynamic)
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

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    var sum = 0;
    var n: dynamic;
    var q: dynamic;
    read(n, q);
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
      var u: dynamic;
      var v: dynamic;
      read(u, v);
      u -= 1;
      v -= 1;
      {
        var i = -1;
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
        var i = -1;
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
      for (var i in s)
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
      for (var i in s)
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
