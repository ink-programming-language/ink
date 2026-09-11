// Translated from solution.cpp.

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((*cpp_cast(b)) - (*cpp_cast(a)));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  scanf("%d %d", (&n), (&m));
  var a: dynamic = cpp_array(n);
  var b: dynamic = cpp_array(m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d", (&b[i]));
      i += 1;
    }
  }
  qsort(b, m, cpp_sizeof((b[0])), cmp);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var k: dynamic = 0;
      if ((a[i] == 0))
      {
        a[i] = b[k];
        k += 1;
      }
      i += 1;
    }
  }
  var flag: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < (n - 1)))
    {
      if ((a[i] >= a[(i + 1)]))
      {
        flag = 1;
      }
      i += 1;
    }
  }
  if ((flag == 1))
  {
    printf("YES");
  } else
  {
    printf("NO\n");
  }
}
