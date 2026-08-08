// Translated from solution.cpp.

func getInt()
{
  var s: dynamic;
  scanf("%d", (&s));
  return s;
}

func main()
{
  var n = getInt();
  var k = getInt();
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      a[i] = getInt();
      i += 1;
    }
  }
  var cnt = 1;
  var d = 0;
  {
    var i = 1;
    while ((i < n))
    {
      d -= (((((n - 1) - i)) * a[i]) * cnt);
      var f = (d < k);
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
