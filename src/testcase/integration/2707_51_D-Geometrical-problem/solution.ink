// Translated from solution.cpp.

var a = cpp_array(100005);

var b = cpp_array(100005);

var n: dynamic;

func solve1(m: dynamic)
{
  {
    var i = 1;
    while ((i < (m - 1)))
    {
      if ((((b[i] * 1) * b[i]) != ((b[(i - 1)] * 1) * b[(i + 1)])))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func solve()
{
  if (((n <= 2) && (a[0] != 0)))
  {
    puts("0");
    return;
  }
  var nzc = 0;
  {
    var i = 1;
    while ((i < n))
    {
      if ((a[i] != 0))
      {
        nzc += 1;
      }
      i += 1;
    }
  }
  if ((!nzc))
  {
    puts("0");
    return;
  }
  if ((nzc == 1))
  {
    puts("1");
    return;
  }
  var zc = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((a[i] == 0))
      {
        zc += 1;
      }
      i += 1;
    }
  }
  if ((zc > 1))
  {
    puts("2");
    return;
  }
  if ((zc == 1))
  {
    var m = 0;
    {
      var i = 0;
      while ((i < n))
      {
        if ((a[i] != 0))
        {
          b[cpp_update(m, "++")] = a[i];
        }
        i += 1;
      }
    }
    puts(if (solve1(m)) "1" else "2");
    return;
  }
  var m = 0;
  {
    var i = 0;
    while ((i < n))
    {
      b[cpp_update(m, "++")] = a[i];
      i += 1;
    }
  }
  if (solve1(m))
  {
    puts("0");
    return;
  }
  m = 0;
  {
    var i = 1;
    while ((i < n))
    {
      b[cpp_update(m, "++")] = a[i];
      i += 1;
    }
  }
  if (solve1(m))
  {
    puts("1");
    return;
  }
  m = 0;
  {
    var i = 0;
    while ((i < n))
    {
      if ((i != 1))
      {
        b[cpp_update(m, "++")] = a[i];
      }
      i += 1;
    }
  }
  if (solve1(m))
  {
    puts("1");
    return;
  }
  m = 0;
  b[cpp_update(m, "++")] = a[0];
  b[cpp_update(m, "++")] = a[1];
  {
    var i = 2;
    while ((i < n))
    {
      b[cpp_update(m, "++")] = a[i];
      if ((((b[(m - 2)] * 1) * b[(m - 2)]) != ((b[(m - 1)] * 1) * b[(m - 3)])))
      {
        m -= 1;
      }
      i += 1;
    }
  }
  if ((m >= (n - 1)))
  {
    puts("1");
  } else
  {
    puts("2");
  }
}

func main(argument_0: dynamic)
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  solve();
  return 0;
}
