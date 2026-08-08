// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var f = cpp_array(2, 100510);

var ch = cpp_array(100510, 4);

func main()
{
  scanf("%d%d%*d", (&n), (&m));
  scanf("%s%s%s%s", (ch[0] + 1), (ch[1] + 1), (ch[2] + 1), (ch[3] + 1));
  {
    var i = 1;
    while ((i <= m))
    {
      if ((ch[2][i] == cpp_char("R")))
      {
        f[1][1] += 1;
      } else
      {
        f[1][0] += 1;
      }
      i += 1;
    }
  }
  if ((ch[0][1] == cpp_char("R")))
  {
    f[1][1] += 1;
  } else
  {
    f[1][0] += 1;
  }
  if ((ch[1][1] == cpp_char("R")))
  {
    f[1][1] += 1;
  } else
  {
    f[1][0] += 1;
  }
  {
    var i = 2;
    while ((i <= n))
    {
      var c0 = 0;
      var c1 = 0;
      if ((ch[0][i] == cpp_char("R")))
      {
        c1 += 1;
      } else
      {
        c0 += 1;
      }
      if ((ch[1][i] == cpp_char("R")))
      {
        c1 += 1;
      } else
      {
        c0 += 1;
      }
      f[i][1] = (min((f[(i - 1)][0] + m), f[(i - 1)][1]) + c1);
      f[i][0] = (min((f[(i - 1)][1] + m), f[(i - 1)][0]) + c0);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      if ((ch[3][i] == cpp_char("R")))
      {
        f[n][1] += 1;
      } else
      {
        f[n][0] += 1;
      }
      i += 1;
    }
  }
  var ans = min(f[n][0], f[n][1]);
  memset(f, 0, cpp_sizeof((f)));
  {
    var i = 1;
    while ((i <= n))
    {
      if ((ch[0][i] == cpp_char("R")))
      {
        f[1][1] += 1;
      } else
      {
        f[1][0] += 1;
      }
      i += 1;
    }
  }
  if ((ch[2][1] == cpp_char("R")))
  {
    f[1][1] += 1;
  } else
  {
    f[1][0] += 1;
  }
  if ((ch[3][1] == cpp_char("R")))
  {
    f[1][1] += 1;
  } else
  {
    f[1][0] += 1;
  }
  {
    var i = 2;
    while ((i <= m))
    {
      var c0 = 0;
      var c1 = 0;
      if ((ch[2][i] == cpp_char("R")))
      {
        c1 += 1;
      } else
      {
        c0 += 1;
      }
      if ((ch[3][i] == cpp_char("R")))
      {
        c1 += 1;
      } else
      {
        c0 += 1;
      }
      f[i][0] = (min(f[(i - 1)][0], (f[(i - 1)][1] + n)) + c0);
      f[i][1] = (min(f[(i - 1)][1], (f[(i - 1)][0] + n)) + c1);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      if ((ch[1][i] == cpp_char("R")))
      {
        f[m][1] += 1;
      } else
      {
        f[m][0] += 1;
      }
      i += 1;
    }
  }
  ans = min(ans, min(f[m][0], f[m][1]));
  printf("%d\n", ans);
}
