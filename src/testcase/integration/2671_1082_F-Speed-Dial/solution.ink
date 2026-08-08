// Translated from solution.cpp.

func cmin(a: dynamic, b: dynamic)
{
  ((((a > b)) && (cpp_assign(a, "=", b))));
}

func cmax(a: dynamic, b: dynamic)
{
  ((((a < b)) && (cpp_assign(a, "=", b))));
}

var IO: dynamic;

func rd()
{
  var s = 0;
  var f = 0;
  while ((!isdigit(cpp_assign(IO, "=", getchar()))))
  {
    f |= (IO == cpp_char("-"));
  }
  while (true)
  {
    s = ((((s << 1)) + ((s << 3))) + ((IO ^ cpp_char("0"))));
    if (!((isdigit(cpp_assign(IO, "=", getchar())))))
    {
      break;
    }
  }
  return if (f) (-s) else s;
}

var N = 510;

var INF = (1e9 + 10);

var n: dynamic;

var m: dynamic;

var trie = cpp_array(10, N);

var cnt: dynamic;

var c = cpp_array(N);

var s = cpp_array(N);

var dp = cpp_array(12, N, N);

var F = cpp_array(12, N);

var G = cpp_array(12);

var dep = cpp_array(N);

func dfs(u: dynamic)
{
  for (var v in trie[u])
  {
    if (v)
    {
      dep[v] = (dep[u] + 1);
      dfs(v);
    }
  }
  memset(F, 63, cpp_sizeof(F));
  {
    var i = 0;
    var iend = dep[u];
    while ((i <= iend))
    {
      F[i][0] = (c[u] * ((dep[u] - i)));
      i += 1;
    }
  }
  for (var v in trie[u])
  {
    if (v)
    {
      {
        var j = 0;
        var jend = dep[u];
        while ((j <= jend))
        {
          {
            var k = 0;
            var kend = m;
            while ((k <= kend))
            {
              G[k] = F[j][k];
              F[j][k] = INF;
              k += 1;
            }
          }
          {
            var k = 0;
            var kend = m;
            while ((k <= kend))
            {
              {
                var d = 0;
                var dend = (m - k);
                while ((d <= dend))
                {
                  cmin(F[j][(k + d)], (G[k] + dp[v][j][d]));
                  d += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
    }
  }
  {
    var d = 0;
    var dend = dep[u];
    while ((d <= dend))
    {
      {
        var i = 0;
        var iend = m;
        while ((i <= iend))
        {
          dp[u][d][i] = INF;
          i += 1;
        }
      }
      {
        var i = 0;
        var iend = m;
        while ((i <= iend))
        {
          cmin(dp[u][d][(i + 1)], F[dep[u]][i]);
          cmin(dp[u][d][i], F[d][i]);
          i += 1;
        }
      }
      d += 1;
    }
  }
}

func main()
{
  n = rd();
  m = rd();
  {
    var i = 1;
    var iend = n;
    while ((i <= iend))
    {
      scanf("%s", (s + 1));
      var u = 0;
      {
        var j = 1;
        while (s[j])
        {
          var v = trie[u][(s[j] - cpp_char("0"))];
          if ((!v))
          {
            v = cpp_update(cnt, "++");
          }
          u = v;
          j += 1;
        }
      }
      c[u] += rd();
      i += 1;
    }
  }
  dfs(0);
  var ans = INF;
  {
    var i = 0;
    var iend = m;
    while ((i <= iend))
    {
      cmin(ans, dp[0][0][i]);
      i += 1;
    }
  }
  printf("%d\n", ans);
}
