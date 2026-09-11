// Translated from solution.cpp.

func getInt() -> dynamic
{
  var s: dynamic = cpp_uninitialized();
  scanf("%d", (&s));
  return s;
}

func main() -> dynamic
{
  var n: dynamic = getInt();
  var k: dynamic = getInt();
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((n))))
    {
      a[i] = getInt();
      i += 1;
    }
  }
  var cnt: dynamic = 1;
  var d: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      d -= (((((n - 1) - i)) * a[i]) * cnt);
      var f: dynamic = (d < k);
      d += (((((n - 1) - i)) * a[i]) * cnt);
      if (f)
      {
        printf("%d\n", (i + 1));
      } else
      {
        cnt += 1;
        d += (a[i] * ((cnt - 1)));
      }
      i += 1;
    }
  }
  return 0;
}
