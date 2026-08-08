// Translated from solution.cpp.

func gi()
{
  var w = 0;
  var q = 1;
  var c = getchar();
  while (((((c < cpp_char("0")) || (c > cpp_char("9")))) && (c != cpp_char("-"))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    q = 0;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    w = (((w * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return if (q) w else (-w);
}

var N = 510;

var f = cpp_array(N, N, 10);

var g = cpp_array(N, N, 10);

var h = cpp_array(N, N);

var H = cpp_array(N, N);

func main()
{
  var n = gi();
  var m = gi();
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var a: dynamic;
  var b: dynamic;
  var t: dynamic;
  var ans: dynamic;
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          f[0][i][j] = (-1 << 60);
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    a = gi();
    b = gi();
    f[0][a][b] = max(f[0][a][b], cpp_cast(gi()));
    f[0][b][a] = max(f[0][b][a], cpp_cast(gi()));
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          g[0][i][j] = f[0][i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    t = 1;
    while ((t < 10))
    {
      {
        i = 1;
        while ((i <= n))
        {
          {
            j = 1;
            while ((j <= n))
            {
              g[t][i][j] = (-1 << 60);
              f[t][i][j] = f[(t - 1)][i][j];
              {
                k = 1;
                while ((k <= n))
                {
                  g[t][i][j] = max(g[t][i][j], (g[(t - 1)][i][k] + g[(t - 1)][k][j]));
                  f[t][i][j] = max(f[t][i][j], (g[(t - 1)][i][k] + f[(t - 1)][k][j]));
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= n))
        {
          if ((f[t][i][i] > 0))
          {
            break;
          }
          i += 1;
        }
      }
      if ((i <= n))
      {
        break;
      }
      t += 1;
    }
  }
  if ((t == 10))
  {
    return cpp_comma(puts("0"), 0);
  }
  {
    i = 1;
    ans = (1 << (cpp_update(t, "--")));
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= n))
        {
          h[i][j] = g[t][i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
  while ((cpp_update(t, "--") >= 0))
  {
    {
      i = 1;
      while ((i <= n))
      {
        {
          k = 1;
          while ((k <= n))
          {
            if (((h[i][k] + f[t][k][i]) > 0))
            {
              break;
            }
            k += 1;
          }
        }
        if ((k <= n))
        {
          break;
        }
        i += 1;
      }
    }
    if ((i > n))
    {
      ans |= (1 << t);
      {
        i = 1;
        while ((i <= n))
        {
          {
            j = 1;
            while ((j <= n))
            {
              H[i][j] = (-1 << 60);
              {
                k = 1;
                while ((k <= n))
                {
                  H[i][j] = max(H[i][j], (h[i][k] + g[t][k][j]));
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        i = 1;
        while ((i <= n))
        {
          {
            j = 1;
            while ((j <= n))
            {
              h[i][j] = H[i][j];
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
  }
  printf("%d\n", (ans + 1));
  return 0;
}
