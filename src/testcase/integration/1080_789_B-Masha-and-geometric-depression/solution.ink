// Translated from solution.cpp.

var b: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100001);

var p: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

func abso(x: dynamic) -> dynamic
{
  if ((x >= 0))
  {
    return x;
  }
  return (-x);
}

func main() -> dynamic
{
  c = 0;
  scanf("%lld %lld %lld %d", (&b), (&q), (&l), (&m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%lld", (&a[i]));
      i += 1;
    }
  }
  n = m;
  if ((b == 0))
  {
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        if ((a[i] == 0))
        {
          printf("0\n");
          return 0;
        }
        i += 1;
      }
    }
    printf("inf\n");
    return 0;
  }
  if ((q == 0))
  {
    if ((abso(b) > l))
    {
      printf("0\n");
      return 0;
    }
    var zf: dynamic = false;
    var bf: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        if ((a[i] == 0))
        {
          zf = true;
        } else if ((a[i] == b))
        {
          bf = true;
        }
        i += 1;
      }
    }
    if ((!zf))
    {
      printf("inf\n");
    } else if ((!bf))
    {
      printf("1\n");
    } else
    {
      printf("0\n");
    }
    return 0;
  }
  if ((q == 1))
  {
    if ((abso(b) > l))
    {
      printf("0\n");
      return 0;
    }
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        if ((a[i] == b))
        {
          printf("0\n");
          return 0;
        }
        i += 1;
      }
    }
    printf("inf\n");
    return 0;
  }
  if ((q == -1))
  {
    if ((abso(b) > l))
    {
      printf("0\n");
      return 0;
    }
    var pf: dynamic = false;
    var nf: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i < m))
      {
        if ((a[i] == b))
        {
          pf = true;
        } else if ((a[i] == (-b)))
        {
          nf = true;
        }
        i += 1;
      }
    }
    if (((!pf) || (!nf)))
    {
      printf("inf\n");
    } else
    {
      printf("0\n");
    }
    return 0;
  }
  p = m;
  n = -1;
  sort(a, (a + m));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if ((a[i] >= 0))
      {
        p = i;
        n = (i - 1);
        break;
      }
      i += 1;
    }
  }
  if (((n == -1) && (a[0] < 0)))
  {
    n = (m - 1);
  }
  while ((abs(b) <= l))
  {
    if ((b > 0))
    {
      while (((p < m) && (a[p] < b)))
      {
        p += 1;
      }
      if (((p < m) && (a[p] != b)))
      {
        c += 1;
      }
      if ((p >= m))
      {
        c += 1;
      }
    } else
    {
      while (((n >= 0) && (a[n] > b)))
      {
        n -= 1;
      }
      if (((n >= 0) && (a[n] != b)))
      {
        c += 1;
      }
      if ((n < 0))
      {
        c += 1;
      }
    }
    b *= q;
  }
  printf("%d\n", c);
  return 0;
}
