// Translated from solution.cpp.

var eps = 1e-9;

func gcd(l: dynamic, r: dynamic)
{
  if ((l > r))
  {
    return gcd(r, l);
  } else
  {
    if ((r % l))
    {
      return gcd(l, (r % l));
    } else
    {
      return l;
    }
  }
}

func main()
{
  write(setprecision(11), fixed);
  while (1)
  {
    var N: dynamic;
    var M: dynamic;
    read(N, M);
    var sum = 0;
    var year = 0;
    if ((!N))
    {
      break;
    }
    {
      var i = 0;
      while ((i < M))
      {
        read(nums[i]);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < ((1 << M))))
      {
        var anum = 1;
        {
          var j = 0;
          while ((j < M))
          {
            if (bs[j])
            {
              anum = ((anum * nums[j]) / gcd(anum, nums[j]));
            }
            j += 1;
          }
        }
        var asum = (N / anum);
        var ayear = ((((anum + N)) * ((N / anum))) / 2);
        if ((bs.count() % 2))
        {
          sum -= asum;
          year -= ayear;
        } else
        {
          sum += asum;
          year += ayear;
        }
        i += 1;
      }
    }
    if ((sum < eps))
    {
      write(0, "\n");
    } else
    {
      write((year / sum), "\n");
    }
  }
  return 0;
}
