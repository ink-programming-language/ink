// Translated from solution.cpp.

var RI = cpp_expression("#include<bit");

func read()
{
  var q = 0;
  var ch = cpp_char(" ");
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    q = (((q * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return q;
}

var mod = (1e9 + 7);

var N = 200005;

var inf = 0x3f3f3f3f;

var orz = cpp_array(N);

var mi = inf;

var mic: dynamic;

var mii: dynamic;

var X: dynamic;

var Y: dynamic;

var ans: dynamic;

var js: dynamic;

var n: dynamic;

var a = cpp_array(N);

var fac = cpp_array(N);

var ni = cpp_array(N);

func ksm(x: dynamic, y: dynamic)
{
  var re = 1;
  {
    while (y)
    {
      if ((y & 1))
      {
        re = (((1 * re) * x) % mod);
      }
      y >>= 1;
      x = (((1 * x) * x) % mod);
    }
  }
  return re;
}

func main()
{
  var c: dynamic;
  var w: dynamic;
  n = read();
  X = read();
  Y = read();
  {
    var i = 1;
    while ((i <= n))
    {
      c = read();
      w = read();
      orz[c].push_back(w);
      if ((w <= mi))
      {
        mii = mi;
        mi = w;
        mic = c;
      } else if ((w < mii))
      {
        mii = w;
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      sort(orz[i].begin(), orz[i].end());
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((!cpp_cast((orz[i].size()))))
      {
        i += 1;
        continue;
      }
      if ((i == mic))
      {
        a[i] = 1;
        js += 1;
        {
          var j = 1;
          while ((j < orz[i].size()))
          {
            if ((((orz[i][j] + mii) <= Y) || ((orz[i][j] + orz[i][0]) <= X)))
            {
              a[i] += 1;
              js += 1;
            }
            j += 1;
          }
        }
      } else
      {
        if (((orz[i][0] + mi) > Y))
        {
          i += 1;
          continue;
        }
        a[i] = 1;
        js += 1;
        {
          var j = 1;
          while ((j < orz[i].size()))
          {
            if ((((orz[i][j] + mi) <= Y) || ((orz[i][j] + orz[i][0]) <= X)))
            {
              a[i] += 1;
              js += 1;
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  fac[0] = 1;
  {
    var i = 1;
    while ((i <= n))
    {
      fac[i] = (((1 * fac[(i - 1)]) * i) % mod);
      i += 1;
    }
  }
  ni[n] = ksm(fac[n], (mod - 2));
  {
    var i = (n - 1);
    while ((i >= 0))
    {
      ni[i] = (((1 * ni[(i + 1)]) * ((i + 1))) % mod);
      i -= 1;
    }
  }
  ans = fac[js];
  {
    var i = 1;
    while ((i <= n))
    {
      ans = (((1 * ans) * ni[a[i]]) % mod);
      i += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
