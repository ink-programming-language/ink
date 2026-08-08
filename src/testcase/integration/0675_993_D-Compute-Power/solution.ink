// Translated from solution.cpp.

var n: dynamic;

var f = cpp_array(1000, 1000);

var sum = cpp_array(1000);

class arr
{
  var x: dynamic;
  var y: dynamic;
}

var a = cpp_array(1000);

func cmp(x: dynamic, y: dynamic)
{
  return (((x.x > y.x)) || ((((x.x == y.x)) && ((x.y > y.y)))));
}

func check(x: dynamic)
{
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
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
    var i = 1;
    var j: dynamic;
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
        var k = i;
        while ((k < j))
        {
          sum[((k - i) + 1)] = ((sum[(k - i)] + a[k].x) - (a[k].y * x));
          k += 1;
        }
      }
      {
        var k = 0;
        while ((k <= n))
        {
          {
            var ij = 0;
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
    var i = 0;
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

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i].x));
      a[i].x *= 1000;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&a[i].y));
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  var l = 0;
  var r = 100000000000;
  var mid = (((l + r)) >> 1);
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
