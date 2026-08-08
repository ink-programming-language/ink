// Translated from solution.cpp.

func readi(x: dynamic)
{
  var v = 0;
  var f = 1;
  var c = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func readll(x: dynamic)
{
  var v = 0;
  var f = 1;
  var c = getchar();
  while (((!isdigit(c)) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    f = -1;
  } else
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  while (isdigit(cpp_assign(c, "=", getchar())))
  {
    v = (((v * 10) + c) - cpp_char("0"));
  }
  x = (v * f);
}

func readc(x: dynamic)
{
  var c: dynamic;
  while (((cpp_assign(c, "=", getchar())) == cpp_char(" ")))
  {
  }
  x = c;
}

func writes(s: dynamic)
{
  puts(s.c_str());
}

func writeln()
{
  writes("");
}

func writei(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = abs(x);
  }
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a = cpp_array(25);
  var top = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

func writell(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = abs(x);
  }
  if ((!x))
  {
    putchar(cpp_char("0"));
  }
  var a = cpp_array(25);
  var top = 0;
  while (x)
  {
    a[cpp_update(top, "++")] = (((x % 10)) + cpp_char("0"));
    x /= 10;
  }
  while (top)
  {
    putchar(a[top]);
    top -= 1;
  }
}

func inc(x: dynamic)
{
  return cpp_update(x, "++");
}

func inc(x: dynamic)
{
  return cpp_update(x, "++");
}

func inc(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "+=", y);
}

func inc(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "+=", y);
}

func inc(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "+=", y);
}

func dec(x: dynamic)
{
  return cpp_update(x, "--");
}

func dec(x: dynamic)
{
  return cpp_update(x, "--");
}

func dec(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "-=", y);
}

func dec(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "-=", y);
}

func dec(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "-=", y);
}

func mul(x: dynamic)
{
  return cpp_assign(x, "=", ((cpp_cast(x)) * x));
}

func mul(x: dynamic)
{
  return cpp_assign(x, "=", (x * x));
}

func mul(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "*=", y);
}

func mul(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "*=", y);
}

func mul(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "*=", y);
}

func divi(x: dynamic)
{
  var ans: dynamic;
  var l: dynamic;
  var r: dynamic;
  var mid: dynamic;
  ans = 0;
  l = 0;
  r = 0x3fffffff;
  while ((l < r))
  {
    mid = (((l + r)) / 2);
    if (((mid * mid) <= x))
    {
      ans = mid;
      l = (mid + 1);
    } else
    {
      r = mid;
    }
  }
  return ans;
}

func divi(x: dynamic)
{
  var ans: dynamic;
  var l: dynamic;
  var r: dynamic;
  var mid: dynamic;
  ans = 0;
  l = 0;
  r = 0x3fffffff;
  while ((l < r))
  {
    mid = (((l + r)) / 2);
    if (((mid * mid) <= x))
    {
      ans = mid;
      l = (mid + 1);
    } else
    {
      r = mid;
    }
  }
  return ans;
}

func divi(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "/=", y);
}

func divi(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "/=", y);
}

func divi(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "/=", y);
}

func mod(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "%=", y);
}

func mod(x: dynamic, y: dynamic)
{
  return cpp_assign(x, "%=", y);
}

class matrix
{
  var a: dynamic = cpp_array(205, 205);
}

var f = cpp_array(17);

var n: dynamic;

var m: dynamic;

var i: dynamic;

var j: dynamic;

var k: dynamic;

var px = cpp_array(205);

var py = cpp_array(205);

var x: dynamic;

var y: dynamic;

func mul(x: dynamic, y: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var ans: dynamic;
  memset((ans.a), (0), (cpp_sizeof(((ans.a)))));
  if (((1) <= ((n))))
  {
    {
      ((i)) = (1);
      while ((((i)) <= ((n))))
      {
        if (((1) <= ((n))))
        {
          {
            ((j)) = (1);
            while ((((j)) <= ((n))))
            {
              if (((1) <= ((n))))
              {
                {
                  ((k)) = (1);
                  while ((((k)) <= ((n))))
                  {
                    ans.a[i][j] += (x.a[i][k] * y.a[k][j]);
                    ((k)) += 1;
                  }
                }
              }
              ((j)) += 1;
            }
          }
        }
        ((i)) += 1;
      }
    }
  }
  return ans;
}

var cnt = cpp_array(205);

var g = cpp_array(205, 2);

var lne: dynamic;

func ind()
{
  read(n);
  if (((1) <= ((n))))
  {
    {
      ((i)) = (1);
      while ((((i)) <= ((n))))
      {
        read(px[i], py[i]);
        ((i)) += 1;
      }
    }
  }
  if (((1) <= ((n))))
  {
    {
      ((i)) = (1);
      while ((((i)) <= ((n))))
      {
        if (((1) <= (((i - 1)))))
        {
          {
            ((j)) = (1);
            while ((((j)) <= (((i - 1)))))
            {
              var dx = (px[i] - px[j]);
              var dy = (py[i] - py[j]);
              var tis: dynamic;
              if (((1) <= ((n))))
              {
                {
                  ((k)) = (1);
                  while ((((k)) <= ((n))))
                  {
                    if (((((px[i] - px[k])) * dy) == (((py[i] - py[k])) * dx)))
                    {
                      tis.push_back(k);
                    }
                    ((k)) += 1;
                  }
                }
              }
              if ((tis.size() >= 2))
              {
                lne.push_back(tis);
              }
              ((j)) += 1;
            }
          }
        }
        ((i)) += 1;
      }
    }
  }
  stable_sort(lne.begin(), lne.end());
  lne.resize((unique(lne.begin(), lne.end()) - lne.begin()));
  {
    typeof((lne).begin()) = (lne).begin();
    while ((it != (lne).end()))
    {
      {
        typeof(((*it)).begin()) = ((*it)).begin();
        while ((it2 != ((*it)).end()))
        {
          cnt[(*it2)] += 1.0;
          it2 += 1;
        }
      }
      it += 1;
    }
  }
  {
    typeof((lne).begin()) = (lne).begin();
    while ((it != (lne).end()))
    {
      {
        typeof(((*it)).begin()) = ((*it)).begin();
        while ((it2 != ((*it)).end()))
        {
          {
            typeof(((*it)).begin()) = ((*it)).begin();
            while ((it3 != ((*it)).end()))
            {
              f[0].a[(*it2)][(*it3)] += ((1.0 / cnt[(*it2)]) / it->size());
              it3 += 1;
            }
          }
          it2 += 1;
        }
      }
      it += 1;
    }
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  ind();
  if (((1) <= ((15))))
  {
    {
      ((i)) = (1);
      while ((((i)) <= ((15))))
      {
        f[i] = mul(f[(i - 1)], f[(i - 1)]);
        ((i)) += 1;
      }
    }
  }
  read(m);
  while (cpp_update(m, "--"))
  {
    read(y, x);
    memset((g), (0), (cpp_sizeof(((g)))));
    g[0][y] = 1;
    x -= 1;
    var z = 0;
    if (((0) <= (15)))
    {
      {
        (i) = (0);
        while (((i) <= (15)))
        {
          if ((((x >> i)) & 1))
          {
            z ^= 1;
            memset((g[z]), (0), (cpp_sizeof(((g[z])))));
            if (((1) <= ((n))))
            {
              {
                ((j)) = (1);
                while ((((j)) <= ((n))))
                {
                  if (((1) <= ((n))))
                  {
                    {
                      ((k)) = (1);
                      while ((((k)) <= ((n))))
                      {
                        g[z][j] += (g[(!z)][k] * f[i].a[j][k]);
                        ((k)) += 1;
                      }
                    }
                  }
                  ((j)) += 1;
                }
              }
            }
          }
          (i) += 1;
        }
      }
    }
    var res = 0;
    {
      typeof((lne).begin()) = (lne).begin();
      while ((it != (lne).end()))
      {
        var sum = 0;
        {
          typeof(((*it)).begin()) = ((*it)).begin();
          while ((it2 != ((*it)).end()))
          {
            sum += g[z][(*it2)];
            it2 += 1;
          }
        }
        sum /= it->size();
        res = max(res, sum);
        it += 1;
      }
    }
    printf("%.50f\n", res);
  }
  return 0;
}
