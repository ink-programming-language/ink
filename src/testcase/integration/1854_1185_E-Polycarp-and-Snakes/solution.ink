// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if ((a < b))
  {
    swap(a, b);
  }
  if ((b == 0))
  {
    return a;
  }
  while (((cpp_assign(a, "=", (a % b))) != 0))
  {
    swap(a, b);
  }
  return b;
}

func modpow(x: dynamic, y: dynamic)
{
  var res = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      res = ((res * x) % 1000000007);
    }
    y = (y >> 1);
    x = ((x * x) % 1000000007);
  }
  return res;
}

func ncr(n: dynamic, r: dynamic)
{
  var f1 = 1;
  var f2 = 1;
  {
    var i = (((n - r) + 1));
    while ((i < ((n + 1))))
    {
      f1 = ((f1 * i) % 998244353);
      i += 1;
    }
  }
  {
    var i = (1);
    while ((i < ((r + 1))))
    {
      f2 = ((f2 * i) % 998244353);
      i += 1;
    }
  }
  return ((f1 * modpow(f2, (998244353 - 2))) % 998244353);
}

func sieve(n: dynamic)
{
  var lpf = cpp_new();
  {
    var i = 1;
    while ((i <= n))
    {
      lpf[i] = i;
      i += 1;
    }
  }
  var rt = (cpp_cast(floor(sqrt(n))) + 1);
  {
    var i = (2);
    while ((i < (rt)))
    {
      if ((lpf[i] != i))
      {
        i += 1;
        continue;
      }
      {
        var j = (i * i);
        while ((j <= n))
        {
          if ((lpf[j] == j))
          {
            lpf[j] = i;
          }
          j += i;
        }
      }
      i += 1;
    }
  }
  return lpf;
}

func solve()
{
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  var a = cpp_array(m, n);
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (m)))
        {
          read(a[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var cord = cpp_array(2, 26);
  {
    var i = (0);
    while ((i < (26)))
    {
      cord[i][0] = cpp_assign(cord[i][1], "=", make_pair(-1, -1));
      i += 1;
    }
  }
  var cnt = 0;
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (m)))
        {
          if ((a[i][j] == cpp_char(".")))
          {
            j += 1;
            continue;
          }
          cnt = max(cnt, ((a[i][j] - cpp_char("a")) + 1));
          if ((cord[(a[i][j] - cpp_char("a"))][0] == make_pair(-1, -1)))
          {
            cord[(a[i][j] - cpp_char("a"))][0] = make_pair(i, j);
          }
          cord[(a[i][j] - cpp_char("a"))][1] = make_pair(i, j);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var test = cpp_array(m, n);
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (m)))
        {
          test[i][j] = cpp_char(".");
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (cnt)))
    {
      if ((cord[i][0] == make_pair(-1, -1)))
      {
        cord[i][0] = cpp_assign(cord[i][1], "=", cord[(cnt - 1)][0]);
        i += 1;
        continue;
      }
      if ((cord[i][0].first == cord[i][1].first))
      {
        {
          var j = (cord[i][0].second);
          while ((j < ((cord[i][1].second + 1))))
          {
            test[cord[i][0].first][j] = (cpp_char("a") + i);
            j += 1;
          }
        }
      } else if ((cord[i][0].second == cord[i][1].second))
      {
        {
          var j = (cord[i][0].first);
          while ((j < ((cord[i][1].first + 1))))
          {
            test[j][cord[i][0].second] = (cpp_char("a") + i);
            j += 1;
          }
        }
      } else
      {
        write("NO\n");
        return;
      }
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (n)))
    {
      {
        var j = (0);
        while ((j < (m)))
        {
          if ((a[i][j] != test[i][j]))
          {
            write("NO\n");
            return;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write("YES\n", cnt, "\n");
  {
    var i = (0);
    while ((i < (cnt)))
    {
      write((cord[i][0].first + 1), " ", (cord[i][0].second + 1), " ", (cord[i][1].first + 1), " ", (cord[i][1].second + 1), "\n");
      i += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  var t = 1;
  read(t);
  {
    var i = (1);
    while ((i < ((t + 1))))
    {
      solve();
      write("\n");
      i += 1;
    }
  }
}
