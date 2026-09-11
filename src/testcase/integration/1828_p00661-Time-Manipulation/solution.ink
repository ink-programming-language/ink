// Translated from solution.cpp.

var eps: dynamic = 1e-9;

func gcd(l: dynamic, r: dynamic) -> dynamic
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

func main() -> dynamic
{
  write(setprecision(11), fixed);
  while (1)
  {
    var N: dynamic = cpp_uninitialized();
    var M: dynamic = cpp_uninitialized();
    read(N, M);
    var sum: dynamic = 0;
    var year: dynamic = 0;
    if ((!N))
    {
      break;
    }
    {
      var i: dynamic = 0;
      while ((i < M))
      {
        read(nums[i]);
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < ((1 << M))))
      {
        var anum: dynamic = 1;
        {
          var j: dynamic = 0;
          while ((j < M))
          {
            if (bs[j])
            {
              anum = ((anum * nums[j]) / gcd(anum, nums[j]));
            }
            j += 1;
          }
        }
        var asum: dynamic = (N / anum);
        var ayear: dynamic = ((((anum + N)) * ((N / anum))) / 2);
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
