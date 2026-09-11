// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var f: dynamic = cpp_array(1000, 1000);

var sum: dynamic = cpp_array(1000);

class arr
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(1000);

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return (((x.x > y.x)) || ((((x.x == y.x)) && ((x.y > y.y)))));
}

func check(x: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
        while ((j <= n))
        {
          f[i][j] = 10000000000000;
          j += 1;
        }
      }
      i += 1;
    }
  }
  f[0][0] = 0;
  {
    var i: dynamic = 1;
    var j: dynamic = cpp_uninitialized();
    while ((i <= n))
    {
      {
        j = (i + 1);
        while (((a[j].x == a[i].x) && (j <= n)))
        {
          j += 1;
        }
      }
      {
        var k: dynamic = i;
        while ((k < j))
        {
          sum[((k - i) + 1)] = ((sum[(k - i)] + a[k].x) - (a[k].y * x));
          k += 1;
        }
      }
      {
        var k: dynamic = 0;
        while ((k <= n))
        {
          {
            var ij: dynamic = 0;
            while ((ij <= min(k, (j - i))))
            {
              f[(j - 1)][((((k - ij)) + ((j - i))) - ij)] = min(f[(j - 1)][((((k - ij)) + ((j - i))) - ij)], (sum[(((j - i)) - ij)] + f[(i - 1)][k]));
              ij += 1;
            }
          }
          k += 1;
        }
      }
      i = j;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= n))
    {
      if ((f[n][i] <= 0))
      {
        return 1;
      }
      i += 1;
    }
  }
  return 0;
}

func main() -> dynamic
{
  scanf("%d", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i].x));
      a[i].x *= 1000;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i].y));
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  var l: dynamic = 0;
  var r: dynamic = 100000000000;
  var mid: dynamic = (((l + r)) >> 1);
  while (true)
  {
    if (check(mid))
    {
      r = mid;
    } else
    {
      l = mid;
    }
    mid = (((l + r)) >> 1);
    if (!((((l + 1) < r))))
    {
      break;
    }
  }
  printf("%lld", r);
  return 0;
}
