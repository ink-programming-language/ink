// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var dif: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_uninitialized();
  while ((scanf("%d %d", (&n), (&c)) == 2))
  {
    var num: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        scanf("%d", (&m));
        num.push_back(m);
        i += 1;
      }
    }
    dif = cpp_assign(ans, "=", 0);
    {
      var i: dynamic = 1;
      while ((i < num.size()))
      {
        if (((num[(i - 1)] - num[i]) > dif))
        {
          dif = (num[(i - 1)] - num[i]);
          ans = ((num[(i - 1)] - num[i]) - c);
        }
        i += 1;
      }
    }
    if ((ans < 0))
    {
      printf("0\n");
    } else
    {
      printf("%d\n", ans);
    }
  }
  return 0;
}
