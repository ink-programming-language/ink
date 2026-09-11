// Translated from solution.cpp.

var a: dynamic = cpp_array(1007);

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var tmp: dynamic = cpp_uninitialized();
  var maxi: dynamic = -1;
  var dig: dynamic = cpp_uninitialized();
  var flag: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  {
    i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      i += 1;
    }
  }
  {
    i = 1;
    while ((i < n))
    {
      {
        j = (i + 1);
        while ((j <= n))
        {
          tmp = (a[i] * a[j]);
          flag = 1;
          dig = (tmp % 10);
          while (1)
          {
            tmp /= 10;
            if ((!tmp))
            {
              break;
            }
            if (((tmp % 10) != (dig - 1)))
            {
              flag = 0;
            }
            dig = (tmp % 10);
          }
          if (flag)
          {
            maxi = max(maxi, (a[i] * a[j]));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  printf("%d\n", maxi);
  return 0;
}
