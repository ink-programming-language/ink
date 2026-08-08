// Translated from solution.cpp.

var a = cpp_array(1007);

func main(argument_0: dynamic)
{
  var n: dynamic;
  var i: dynamic;
  var j: dynamic;
  var tmp: dynamic;
  var maxi = -1;
  var dig: dynamic;
  var flag: dynamic;
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
