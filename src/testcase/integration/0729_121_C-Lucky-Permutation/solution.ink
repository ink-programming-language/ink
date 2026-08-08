// Translated from solution.cpp.

var res: dynamic;

var a = cpp_array(100);

var ins = cpp_array(100);

func IsLucky(x: dynamic)
{
  {
    while (x)
    {
      if ((((x % 10) != 4) && ((x % 10) != 7)))
      {
        return 0;
      }
      x /= 10;
    }
  }
  return 1;
}

func Dfs(now: dynamic, lim: dynamic)
{
  res += 1;
  if ((((cpp_cast(now) * 10) + 4) <= lim))
  {
    Dfs(((now * 10) + 4), lim);
  }
  if ((((cpp_cast(now) * 10) + 7) <= lim))
  {
    Dfs(((now * 10) + 7), lim);
  }
}

func main()
{
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  scanf("%d%d", (&n), (&m));
  var cnt = 1;
  {
    var i = 1;
    while (true)
    {
      cnt *= i;
      if ((cnt >= m))
      {
        k = i;
        break;
      }
      i += 1;
    }
  }
  if ((k > n))
  {
    puts("-1");
    return 0;
  }
  memset(ins, 0, cpp_sizeof((ins)));
  {
    var i = 0;
    while ((i < k))
    {
      cnt = 1;
      {
        var j = 1;
        while ((j < (k - i)))
        {
          cnt *= j;
          j += 1;
        }
      }
      var ret = (((m - 1)) / cnt);
      m -= (ret * cnt);
      {
        var j = 0;
        while ((j < k))
        {
          if ((!ins[j]))
          {
            if ((ret == 0))
            {
              ins[j] = 1;
              a[i] = j;
              break;
            } else
            {
              ret -= 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  res = 0;
  {
    var i = 0;
    while ((i < k))
    {
      if ((IsLucky((((n - k) + 1) + i)) && IsLucky((((n - k) + 1) + a[i]))))
      {
        res += 1;
      }
      i += 1;
    }
  }
  if ((4 <= (n - k)))
  {
    Dfs(4, (n - k));
  }
  if ((7 <= (n - k)))
  {
    Dfs(7, (n - k));
  }
  printf("%d\n", res);
  return 0;
}
