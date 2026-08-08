// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(65);

var mask = cpp_array(65);

var cnt = cpp_array((1 << 15));

var bl = cpp_array(65);

var fa = cpp_array(65);

func qpow(x: dynamic, y: dynamic)
{
  var ret = 1;
  while (y)
  {
    if ((y & 1))
    {
      ret = ((cpp_cast(ret) * x) % 1000000007);
    }
    y >>= 1;
    x = ((cpp_cast(x) * x) % 1000000007);
  }
  return ret;
}

func C(n: dynamic, m: dynamic)
{
  if ((m > n))
  {
    return 0;
  }
  var a = 1;
  var b = 1;
  {
    var i = ((n - m) + 1);
    while ((i <= n))
    {
      a = ((cpp_cast(a) * i) % 1000000007);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      b = ((cpp_cast(b) * i) % 1000000007);
      i += 1;
    }
  }
  return ((cpp_cast(a) * qpow(b, (1000000007 - 2))) % 1000000007);
}

func find(x: dynamic)
{
  if ((fa[x] == x))
  {
    return x;
  }
  return cpp_assign(fa[x], "=", find(fa[x]));
}

func merge(x: dynamic, y: dynamic)
{
  var fx = find(x);
  var fy = find(y);
  if ((fx != fy))
  {
    fa[fx] = fy;
  }
}

var f = cpp_array((1 << 15), 65);

var rk = cpp_array(65);

func calc(x: dynamic)
{
  var num = 0;
  var m = 0;
  f[0][0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      if (((find(i) == x) && bl[i]))
      {
        rk[i] = cpp_update(m, "++");
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if (((find(i) == x) && (!bl[i])))
      {
        num += 1;
        {
          var j = 1;
          while ((j <= n))
          {
            if ((bl[j] && ((a[i] % a[j]) == 0)))
            {
              mask[i] |= (1 << ((rk[j] - 1)));
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  if ((num == 0))
  {
    return make_pair(1, 0);
  }
  {
    var i = 0;
    while ((i <= (((1 << m)) - 1)))
    {
      cnt[i] = 0;
      {
        var j = 1;
        while ((j <= n))
        {
          if ((((find(j) == x) && (!bl[j])) && (((mask[j] | i)) == i)))
          {
            cnt[i] += 1;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(f, 0, cpp_sizeof((f)));
  {
    var i = 1;
    while ((i <= n))
    {
      if (((find(i) == x) && (!bl[i])))
      {
        f[1][mask[i]] += 1;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= (num - 1)))
    {
      {
        var j = 0;
        while ((j <= (((1 << m)) - 1)))
        {
          if (f[i][j])
          {
            (cpp_assign(f[(i + 1)][j], "+=", ((cpp_cast(f[i][j]) * ((cnt[j] - i))) % 1000000007))) %= 1000000007;
            {
              var k = 1;
              while ((k <= n))
              {
                if (((((find(k) == x) && (!bl[k])) && (((mask[k] & j)) != 0)) && (((mask[k] | j)) != j)))
                {
                  (cpp_assign(f[(i + 1)][(mask[k] | j)], "+=", f[i][j])) %= 1000000007;
                }
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return make_pair(f[num][(((1 << m)) - 1)], (num - 1));
}

func main()
{
  scanf("%d", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&a[i]));
      fa[i] = i;
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      var flag = true;
      {
        var j = 1;
        while ((j <= n))
        {
          if (((i != j) && ((a[i] % a[j]) == 0)))
          {
            merge(i, j);
            flag = false;
          }
          j += 1;
        }
      }
      bl[i] = flag;
      i += 1;
    }
  }
  var ans = 1;
  var tot_cnt = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      if ((find(i) == i))
      {
        var tmp = calc(i);
        ans = ((((cpp_cast(ans) * tmp.first) % 1000000007) * C((tot_cnt + tmp.second), tot_cnt)) % 1000000007);
        tot_cnt += tmp.second;
      }
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
