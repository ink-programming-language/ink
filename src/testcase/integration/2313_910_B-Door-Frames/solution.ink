// Translated from solution.cpp.

var arr: dynamic = cpp_array(6);

var bar: dynamic = cpp_array(6);

var ans: dynamic = 6;

var n: dynamic = cpp_uninitialized();

func f(pos: dynamic) -> dynamic
{
  if ((pos == 6))
  {
    var tmp: dynamic = 0;
    {
      var i: dynamic = 0;
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
    var i: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 0;
    while ((i < 6))
    {
      bar[i] = n;
      i += 1;
    }
  }
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  scanf("%d%d", (&a), (&b));
  arr[0] = cpp_assign(arr[1], "=", cpp_assign(arr[2], "=", cpp_assign(arr[3], "=", a)));
  arr[4] = cpp_assign(arr[5], "=", b);
  f(0);
  printf("%d", ans);
  return 0;
}
