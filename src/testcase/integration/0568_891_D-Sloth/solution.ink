// Translated from solution.cpp.

var s = cpp_array(500001);

var su = cpp_array(500001);

var ru = cpp_array(500001);

var ss1 = cpp_array(500001);

var ss2 = cpp_array(500001);

var m: dynamic;

var fir = cpp_array(500001);

var nex = cpp_array(1000001);

var sto = cpp_array(1000001);

var fa = cpp_array(500001);

var a1: dynamic;

var b1: dynamic;

var tot: dynamic;

var n: dynamic;

var sum: dynamic;

var dp = cpp_array(2, 2, 500001);

var f = cpp_array(2, 2, 500001);

var s1: dynamic;

var s2: dynamic;

var s3: dynamic;

var s4: dynamic;

var siz = cpp_array(500001);

var f1 = cpp_array(2, 2);

var ans: dynamic;

var p = cpp_array(2, 500001);

func addbian(aa: dynamic, bb: dynamic)
{
  tot += 1;
  nex[tot] = fir[aa];
  fir[aa] = tot;
  sto[tot] = bb;
}

func dfs(x: dynamic)
{
  var aa = fir[x];
  siz[x] = 1;
  dp[x][0][0] = 1;
  while ((aa != 0))
  {
    if ((fa[x] != sto[aa]))
    {
      fa[sto[aa]] = x;
      dfs(sto[aa]);
      siz[x] = (siz[x] + siz[sto[aa]]);
      s1 = dp[x][0][0];
      s2 = dp[x][0][1];
      s3 = dp[x][1][0];
      s4 = dp[x][1][1];
      if ((dp[sto[aa]][1][0] == 0))
      {
        dp[x][0][0] = 0;
        dp[x][0][1] = 0;
        dp[x][1][0] = 0;
        dp[x][1][1] = 0;
      }
      if ((dp[sto[aa]][1][0] > 0))
      {
        ss1[x] += 1;
      } else if ((dp[sto[aa]][0][0] > 0))
      {
        ss2[x] += 1;
      }
      dp[x][0][1] = (dp[x][0][1] + (s1 * ((dp[sto[aa]][0][0] + dp[sto[aa]][1][1]))));
      dp[x][1][1] = (((dp[x][1][1] + (s1 * dp[sto[aa]][0][1])) + (s2 * dp[sto[aa]][0][0])) + (s3 * ((dp[sto[aa]][1][1] + dp[sto[aa]][0][0]))));
      if ((((s1 > 0)) && ((dp[sto[aa]][0][0] > 0))))
      {
        dp[x][1][0] = 1;
      }
    }
    aa = nex[aa];
  }
}

func dfs1(x: dynamic)
{
  var aa = fir[x];
  s1 = dp[x][0][0];
  s2 = dp[x][0][1];
  s3 = dp[x][1][0];
  s4 = dp[x][1][1];
  if ((f[x][1][0] == 0))
  {
    dp[x][0][0] = 0;
    dp[x][0][1] = 0;
    dp[x][1][0] = 0;
    dp[x][1][1] = 0;
  }
  dp[x][0][1] = (dp[x][0][1] + (s1 * ((f[x][0][0] + f[x][1][1]))));
  dp[x][1][1] = (((dp[x][1][1] + (s1 * f[x][0][1])) + (s2 * f[x][0][0])) + (s3 * ((f[x][1][1] + f[x][0][0]))));
  if ((((s1 > 0)) && ((f[x][0][0] > 0))))
  {
    dp[x][1][0] = 1;
  }
  while ((aa != 0))
  {
    if ((fa[x] != sto[aa]))
    {
      if ((dp[sto[aa]][1][0] > 0))
      {
        ss1[x] -= 1;
      }
      if ((dp[sto[aa]][0][0] > 0))
      {
        ss2[x] -= 1;
      }
      if ((dp[sto[aa]][1][0] == 0))
      {
        {
          var i = 0;
          while ((i <= 1))
          {
            {
              var j = 0;
              while ((j <= 1))
              {
                f1[i][j] = 0;
                j += 1;
              }
            }
            i += 1;
          }
        }
        if ((ss1[x] > (ru[x] - 4)))
        {
          f1[0][0] = 1;
          var bb = fir[x];
          while ((bb != 0))
          {
            if ((((fa[x] != sto[bb])) && ((sto[bb] != sto[aa]))))
            {
              s1 = f1[0][0];
              s2 = f1[0][1];
              s3 = f1[1][0];
              s4 = f1[1][1];
              if ((dp[sto[bb]][1][0] == 0))
              {
                f1[0][0] = 0;
                f1[0][1] = 0;
                f1[1][0] = 0;
                f1[1][1] = 0;
              }
              f1[0][1] = (f1[0][1] + (s1 * ((dp[sto[bb]][0][0] + dp[sto[bb]][1][1]))));
              f1[1][1] = (((f1[1][1] + (s1 * dp[sto[bb]][0][1])) + (s2 * dp[sto[bb]][0][0])) + (s3 * ((dp[sto[bb]][1][1] + dp[sto[bb]][0][0]))));
              if ((((s1 > 0)) && ((dp[sto[bb]][0][0] > 0))))
              {
                f1[1][0] = 1;
              }
            }
            bb = nex[bb];
          }
          s1 = f1[0][0];
          s2 = f1[0][1];
          s3 = f1[1][0];
          s4 = f1[1][1];
          if ((f[x][1][0] == 0))
          {
            f1[0][0] = 0;
            f1[0][1] = 0;
            f1[1][0] = 0;
            f1[1][1] = 0;
          }
          f1[0][1] = (f1[0][1] + (s1 * ((f[x][0][0] + f[x][1][1]))));
          f1[1][1] = (((f1[1][1] + (s1 * f[x][0][1])) + (s2 * f[x][0][0])) + (s3 * ((f[x][1][1] + f[x][0][0]))));
          if ((((s1 > 0)) && ((f[x][0][0] > 0))))
          {
            f1[1][0] = 1;
          }
        }
      } else
      {
        if ((ss1[x] == (ru[x] - 1)))
        {
          f1[0][0] = 1;
        } else
        {
          f1[0][0] = 0;
        }
        if ((((ss1[x] == (ru[x] - 2))) && ((ss2[x] == 1))))
        {
          f1[1][0] = 1;
        } else
        {
          f1[1][0] = 0;
        }
        f1[0][1] = (dp[x][0][1] - (f1[0][0] * ((dp[sto[aa]][0][0] + dp[sto[aa]][1][1]))));
        f1[1][1] = (((dp[x][1][1] - (f1[0][0] * dp[sto[aa]][0][1])) - (f1[0][1] * dp[sto[aa]][0][0])) + (f1[1][0] * ((dp[sto[aa]][1][1] + dp[sto[aa]][0][0]))));
      }
      if ((dp[sto[aa]][1][0] > 0))
      {
        ss1[x] += 1;
      }
      if ((dp[sto[aa]][0][0] > 0))
      {
        ss2[x] += 1;
      }
      if ((f1[1][0] > 0))
      {
        ss1[sto[aa]] += 1;
      } else if ((f1[0][0] > 0))
      {
        ss2[sto[aa]] += 1;
      }
      {
        var i = 0;
        while ((i <= 1))
        {
          {
            var j = 0;
            while ((j <= 1))
            {
              f[sto[aa]][i][j] = f1[i][j];
              j += 1;
            }
          }
          i += 1;
        }
      }
      if (((siz[sto[aa]] % 2) == 0))
      {
        if ((((f1[1][0] > 0)) && ((dp[sto[aa]][1][0] > 0))))
        {
          ans = (ans + (siz[sto[aa]] * ((n - siz[sto[aa]]))));
        }
      } else
      {
        ans = (ans + (((f1[0][0] + f1[1][1])) * ((dp[sto[aa]][0][0] + dp[sto[aa]][1][1]))));
      }
    }
    aa = nex[aa];
  }
  aa = fir[x];
  while ((aa != 0))
  {
    if ((sto[aa] != fa[x]))
    {
      dfs1(sto[aa]);
    }
    aa = nex[aa];
  }
}

func main()
{
  scanf("%I64d", (&n));
  ans = 0;
  tot = 0;
  {
    var i = 1;
    while ((i <= (n - 1)))
    {
      scanf("%d%d", (&a1), (&b1));
      addbian(a1, b1);
      addbian(b1, a1);
      ru[a1] += 1;
      ru[b1] += 1;
      i += 1;
    }
  }
  if (((n % 2) == 1))
  {
    printf("0");
  } else
  {
    dfs(1);
    f[1][1][0] = 1;
    dfs1(1);
    printf("%I64d\n", ans);
  }
}
