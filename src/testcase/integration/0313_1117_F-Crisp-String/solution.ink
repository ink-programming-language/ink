// Translated from solution.cpp.

var inf: dynamic = (1e9 + 7);

var mod: dynamic = (1e9 + 7);

var maxn: dynamic = (1e5 + 7);

var maxmsk: dynamic = (((1 << 17)) + 7);

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var num: dynamic = cpp_array(maxn);

var ok: dynamic = cpp_array(27, 27);

var bad: dynamic = cpp_array(maxmsk);

var met: dynamic = cpp_array(27);

var sum: dynamic = cpp_array(27);

var hve: dynamic = cpp_array(27);

var s: dynamic = cpp_uninitialized();

func init() -> dynamic
{
  scanf("%d%d", (&n), (&p));
  read(s);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      num[i] = (s[i] - cpp_char("a"));
      sum[num[i]] += 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < p))
    {
      {
        var j: dynamic = 0;
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

func solve() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < p))
    {
      {
        var j: dynamic = 0;
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
  var ans: dynamic = n;
  {
    var i: dynamic = 1;
    while ((i < ((1 << p))))
    {
      if (bad[i])
      {
        i += 1;
        continue;
      }
      var isbad: dynamic = true;
      {
        var j: dynamic = 0;
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
      var res: dynamic = 0;
      {
        var j: dynamic = 0;
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

func main() -> dynamic
{
  init();
  solve();
  return 0;
}
