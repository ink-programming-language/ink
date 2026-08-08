// Translated from solution.cpp.

var arr = cpp_array(6);

var bar = cpp_array(6);

var ans = 6;

var n: dynamic;

func f(pos: dynamic)
{
  if ((pos == 6))
  {
    var tmp = 0;
    {
      var i = 0;
      while ((i < 6))
      {
        if ((bar[i] < n))
        {
          tmp += 1;
        }
        i += 1;
      }
    }
    ans = min(ans, tmp);
    return;
  }
  {
    var i = 0;
    while ((i < 6))
    {
      if ((bar[i] >= arr[pos]))
      {
        bar[i] -= arr[pos];
        f((pos + 1));
        bar[i] += arr[pos];
      }
      i += 1;
    }
  }
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 0;
    while ((i < 6))
    {
      bar[i] = n;
      i += 1;
    }
  }
  var a: dynamic;
  var b: dynamic;
  scanf("%d%d", (&a), (&b));
  arr[0] = cpp_assign(arr[1], "=", cpp_assign(arr[2], "=", cpp_assign(arr[3], "=", a)));
  arr[4] = cpp_assign(arr[5], "=", b);
  f(0);
  printf("%d", ans);
  return 0;
}
