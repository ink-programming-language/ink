// Translated from solution.cpp.

var inf = (1e9 + 7);

var mod = (1e9 + 7);

var maxn = (1e5 + 7);

var maxmsk = (((1 << 17)) + 7);

var n: dynamic;

var p: dynamic;

var num = cpp_array(maxn);

var ok = cpp_array(27, 27);

var bad = cpp_array(maxmsk);

var met = cpp_array(27);

var sum = cpp_array(27);

var hve = cpp_array(27);

var s: dynamic;

func init()
{
  scanf("%d%d", (&n), (&p));
  read(s);
  {
    var i = 0;
    while ((i < n))
    {
      num[i] = (s[i] - cpp_char("a"));
      sum[num[i]] += 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < p))
    {
      {
        var j = 0;
        while ((j < p))
        {
          scanf("%d", (ok[i] + j));
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func solve()
{
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < p))
        {
          if ((!hve[j]))
          {
            j += 1;
            continue;
          }
          if ((((met[j] >> (num[i]))) & 1))
          {
            j += 1;
            continue;
          }
          if (ok[num[i]][j])
          {
            j += 1;
            continue;
          }
          bad[met[j]] += 1;
          bad[(met[j] | ((1 << j)))] -= 1;
          bad[(met[j] | ((1 << num[i])))] -= 1;
          bad[((met[j] | ((1 << j))) | ((1 << num[i])))] += 1;
          j += 1;
        }
      }
      hve[num[i]] = true;
      {
        var j = 0;
        while ((j < p))
        {
          met[j] |= ((1 << num[i]));
          j += 1;
        }
      }
      met[num[i]] = 0;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < p))
    {
      {
        var j = 0;
        while ((j < ((1 << p))))
        {
          if ((((j >> i)) & 1))
          {
            bad[j] += bad[(j ^ ((1 << i)))];
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = n;
  {
    var i = 1;
    while ((i < ((1 << p))))
    {
      if (bad[i])
      {
        i += 1;
        continue;
      }
      var isbad = true;
      {
        var j = 0;
        while ((j < p))
        {
          if ((((i >> j)) & 1))
          {
            if ((!bad[(i ^ ((1 << j)))]))
            {
              isbad = false;
              break;
            }
          }
          j += 1;
        }
      }
      if (isbad)
      {
        bad[i] = 1;
        i += 1;
        continue;
      }
      var res = 0;
      {
        var j = 0;
        while ((j < p))
        {
          if ((!((((i >> j)) & 1))))
          {
            res += sum[j];
          }
          j += 1;
        }
      }
      ans = min(ans, res);
      i += 1;
    }
  }
  printf("%d\n", ans);
}

func main()
{
  init();
  solve();
  return 0;
}
