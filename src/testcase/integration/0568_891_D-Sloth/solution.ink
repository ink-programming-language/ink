// Translated from solution.cpp.

var s: dynamic = cpp_array(500001);

var su: dynamic = cpp_array(500001);

var ru: dynamic = cpp_array(500001);

var ss1: dynamic = cpp_array(500001);

var ss2: dynamic = cpp_array(500001);

var m: dynamic = cpp_uninitialized();

var fir: dynamic = cpp_array(500001);

var nex: dynamic = cpp_array(1000001);

var sto: dynamic = cpp_array(1000001);

var fa: dynamic = cpp_array(500001);

var a1: dynamic = cpp_uninitialized();

var b1: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var sum: dynamic = cpp_uninitialized();

var dp: dynamic = cpp_array(2, 2, 500001);

var f: dynamic = cpp_array(2, 2, 500001);

var s1: dynamic = cpp_uninitialized();

var s2: dynamic = cpp_uninitialized();

var s3: dynamic = cpp_uninitialized();

var s4: dynamic = cpp_uninitialized();

var siz: dynamic = cpp_array(500001);

var f1: dynamic = cpp_array(2, 2);

var ans: dynamic = cpp_uninitialized();

var p: dynamic = cpp_array(2, 500001);

func addbian(aa: dynamic, bb: dynamic) -> dynamic
{
  tot += 1;
  nex[tot] = fir[aa];
  fir[aa] = tot;
  sto[tot] = bb;
}

func dfs(x: dynamic) -> dynamic
{
  var aa: dynamic = fir[x];
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

func dfs1(x: dynamic) -> dynamic
{
  var aa: dynamic = fir[x];
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
          var i: dynamic = 0;
          while ((i <= 1))
          {
            {
              var j: dynamic = 0;
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
          var bb: dynamic = fir[x];
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
        var i: dynamic = 0;
        while ((i <= 1))
        {
          {
            var j: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%I64d", (&n));
  ans = 0;
  tot = 0;
  {
    var i: dynamic = 1;
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
