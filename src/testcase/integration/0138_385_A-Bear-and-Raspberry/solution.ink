// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var c: dynamic;
  var m: dynamic;
  var dif: dynamic;
  var ans: dynamic;
  while ((scanf("%d %d", (&n), (&c)) == 2))
  {
    var num: dynamic;
    {
      var i = 0;
      while ((i < n))
      {
        scanf("%d", (&m));
        num.push_back(m);
        i += 1;
      }
    }
    dif = cpp_assign(ans, "=", 0);
    {
      var i = 1;
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
