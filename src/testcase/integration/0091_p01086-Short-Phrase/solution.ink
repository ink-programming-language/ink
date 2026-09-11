// Translated from solution.cpp.

func lp(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func main() -> dynamic
{
  while (1)
  {
    var n: dynamic = cpp_uninitialized();
    read(n);
    if ((n == 0))
    {
      break;
    }
    var a: dynamic = cpp_array(50);
    var memo: dynamic = cpp_uninitialized();
  }
  return 0;
}

func lp(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(memo);
      a[i] = memo.size();
    }

func lp(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      var ans: dynamic = 0;
      var count: dynamic = 0;
      var stats: dynamic = 0;
      {
        var j: dynamic = i;
        while ((j < n))
        {
          if (((stats == 0) || (stats == 2)))
          {
            count += a[j];
            if ((count == 5))
            {
              stats += 1;
              count = 0;
            }
            if ((count > 5))
            {
              break;
            }
          } else if ((((stats == 1) || (stats == 3)) || (stats == 4)))
          {
            count += a[j];
            if ((count == 7))
            {
              stats += 1;
              count = 0;
            }
            if ((count > 7))
            {
              break;
            }
          }
          if ((stats == 5))
          {
            ans = 1;
            break;
          }
          j += 1;
        }
      }
      if ((ans == 1))
      {
        write((i + 1), "\n");
        break;
      }
    }
