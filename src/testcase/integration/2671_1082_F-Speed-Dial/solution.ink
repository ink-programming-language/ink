// Translated from solution.cpp.

func cmin(a: dynamic, b: dynamic) -> dynamic
{
  ((((a > b)) && (cpp_assign(a, "=", b))));
}

func cmax(a: dynamic, b: dynamic) -> dynamic
{
  ((((a < b)) && (cpp_assign(a, "=", b))));
}

var IO: dynamic = cpp_uninitialized();

func rd() -> dynamic
{
  var s: dynamic = 0;
  var f: dynamic = 0;
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
  return  (f) ? (-s) : s;
}

var N: dynamic = 510;

var INF: dynamic = (1e9 + 10);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var trie: dynamic = cpp_array(10, N);

var cnt: dynamic = cpp_uninitialized();

var c: dynamic = cpp_array(N);

var s: dynamic = cpp_array(N);

var dp: dynamic = cpp_array(12, N, N);

var F: dynamic = cpp_array(12, N);

var G: dynamic = cpp_array(12);

var dep: dynamic = cpp_array(N);

func dfs(u: dynamic) -> dynamic
{
  for (var v: dynamic in trie[u])
  {
    if (v)
    {
      dep[v] = (dep[u] + 1);
      dfs(v);
    }
  }
  memset(F, 63, cpp_sizeof(F));
  {
    var i: dynamic = 0;
    var iend: dynamic = dep[u];
    while ((i <= iend))
    {
      F[i][0] = (c[u] * ((dep[u] - i)));
      i += 1;
    }
  }
  for (var v: dynamic in trie[u])
  {
    if (v)
    {
      {
        var j: dynamic = 0;
        var jend: dynamic = dep[u];
        while ((j <= jend))
        {
          {
            var k: dynamic = 0;
            var kend: dynamic = m;
            while ((k <= kend))
            {
              G[k] = F[j][k];
              F[j][k] = INF;
              k += 1;
            }
          }
          {
            var k: dynamic = 0;
            var kend: dynamic = m;
            while ((k <= kend))
            {
              {
                var d: dynamic = 0;
                var dend: dynamic = (m - k);
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
    var d: dynamic = 0;
    var dend: dynamic = dep[u];
    while ((d <= dend))
    {
      {
        var i: dynamic = 0;
        var iend: dynamic = m;
        while ((i <= iend))
        {
          dp[u][d][i] = INF;
          i += 1;
        }
      }
      {
        var i: dynamic = 0;
        var iend: dynamic = m;
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

func main() -> dynamic
{
  n = rd();
  m = rd();
  {
    var i: dynamic = 1;
    var iend: dynamic = n;
    while ((i <= iend))
    {
      scanf("%s", (s + 1));
      var u: dynamic = 0;
      {
        var j: dynamic = 1;
        while (s[j])
        {
          var v: dynamic = trie[u][(s[j] - cpp_char("0"))];
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
  var ans: dynamic = INF;
  {
    var i: dynamic = 0;
    var iend: dynamic = m;
    while ((i <= iend))
    {
      cmin(ans, dp[0][0][i]);
      i += 1;
    }
  }
  printf("%d\n", ans);
}
