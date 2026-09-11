// Translated from solution.cpp.

var eps: dynamic = 1e-8;

var MOD: dynamic = 1000000007;

var INF: dynamic = 0x3f3f3f3f;

var val: dynamic = cpp_array(111, 111);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var pos: dynamic = cpp_uninitialized();

var ddist: dynamic = cpp_array(111, 111);

var way: dynamic = cpp_array(10, 10);

var dist: dynamic = cpp_array(10, 211);

var used: dynamic = cpp_array(111, 111);

var pre: dynamic = cpp_array((1 << 8), 211);

var dp: dynamic = cpp_array((1 << 8), 211);

var dx: dynamic = [0, 0, 1, -1];

var dy: dynamic = [1, -1, 0, 0];

var que: dynamic = cpp_uninitialized();

func bfs(x: dynamic, y: dynamic, o: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  while ((!que.empty()))
  {
    que.pop();
  }
  que.push(make_pair(x, y));
  {
    i = 0;
    while ((i <= (n + 1)))
    {
      {
        j = 0;
        while ((j <= (m + 1)))
        {
          ddist[i][j] = -1;
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
        while ((j <= m))
        {
          ddist[i][j] = MOD;
          j += 1;
        }
      }
      i += 1;
    }
  }
  ddist[x][y] = val[x][y];
  dp[((((x - 1)) * m) + y)][(1 << o)] = val[x][y];
  while ((!que.empty()))
  {
    u = que.front().first;
    v = que.front().second;
    que.pop();
    {
      i = 0;
      while ((i < 4))
      {
        x = (u + dx[i]);
        y = (v + dy[i]);
        if ((ddist[x][y] == -1))
        {
          i += 1;
          continue;
        }
        if ((ddist[x][y] > (ddist[u][v] + val[x][y])))
        {
          ddist[x][y] = (ddist[u][v] + val[x][y]);
          dp[((((x - 1)) * m) + y)][(1 << o)] = ddist[x][y];
          pre[((((x - 1)) * m) + y)][(1 << o)] = make_pair(((((u - 1)) * m) + v), (1 << o));
          que.push(make_pair(x, y));
        }
        i += 1;
      }
    }
  }
  {
    i = 1;
    while ((i <= n))
    {
      {
        j = 1;
        while ((j <= m))
        {
          dist[((((i - 1)) * m) + j)][o] = ddist[i][j];
          j += 1;
        }
      }
      i += 1;
    }
  }
}

var S: dynamic = cpp_array(2);

func doit(p: dynamic, sta: dynamic) -> dynamic
{
  var u: dynamic = ((((p - 1)) / m) + 1);
  var v: dynamic = ((((p - 1)) % m) + 1);
  used[u][v] = 1;
  if ((pre[p][sta].first == -1))
  {
    return;
  }
  doit(pre[p][sta].first, pre[p][sta].second);
  doit(p, (sta - pre[p][sta].second));
}

func main() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var ll: dynamic = cpp_uninitialized();
  var u: dynamic = cpp_uninitialized();
  var v: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  var st: dynamic = cpp_uninitialized();
  var pt: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  while ((scanf("%d%d%d", (&n), (&m), (&k)) != EOF))
  {
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= m))
          {
            scanf("%d", (&val[i][j]));
            j += 1;
          }
        }
        i += 1;
      }
    }
    pos.clear();
    {
      i = 0;
      while ((i < 211))
      {
        {
          j = 0;
          while ((j < (1 << 8)))
          {
            pre[i][j] = make_pair(-1, -1);
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      u = 1;
      while ((u <= n))
      {
        {
          v = 1;
          while ((v <= m))
          {
            i = ((((u - 1)) * m) + v);
            {
              j = 0;
              while ((j < (1 << k)))
              {
                dp[i][j] = MOD;
                j += 1;
              }
            }
            v += 1;
          }
        }
        u += 1;
      }
    }
    {
      i = 0;
      while ((i < k))
      {
        scanf("%d%d", (&u), (&v));
        pos.push_back(make_pair(u, v));
        dp[((((u - 1)) * m) + v)][(1 << i)] = val[u][v];
        i += 1;
      }
    }
    S[0].clear();
    S[1].clear();
    {
      i = 0;
      while ((i < k))
      {
        S[0].insert(make_pair(pos[i].first, pos[i].second));
        i += 1;
      }
    }
    var uu: dynamic = 0;
    var vv: dynamic = 1;
    {
      j = 1;
      while ((j <= (n * m)))
      {
        S[vv].clear();
        if ((S[uu].size() == 0))
        {
          break;
        }
        while (S[uu].size())
        {
          u = S[uu].begin()->first;
          v = S[uu].begin()->second;
          S[uu].erase(S[uu].begin());
          p = ((((u - 1)) * m) + v);
          {
            i = 0;
            while ((i < 4))
            {
              x = (u + dx[i]);
              y = (v + dy[i]);
              if (((((x < 1) || (x > n)) || (y < 1)) || (y > m)))
              {
                i += 1;
                continue;
              }
              q = ((((x - 1)) * m) + y);
              {
                l = 0;
                while ((l < ((1 << k))))
                {
                  if ((dp[q][l] > (dp[p][l] + val[x][y])))
                  {
                    dp[q][l] = (dp[p][l] + val[x][y]);
                    pre[q][l] = make_pair(p, l);
                    S[vv].insert(make_pair(x, y));
                  }
                  {
                    ll = 0;
                    while ((ll < ((1 << k))))
                    {
                      if ((ll & l))
                      {
                        ll += 1;
                        continue;
                      }
                      if ((dp[q][(ll | l)] > (dp[p][l] + dp[q][ll])))
                      {
                        dp[q][(ll | l)] = (dp[p][l] + dp[q][ll]);
                        pre[q][(ll | l)] = make_pair(p, l);
                        S[vv].insert(make_pair(x, y));
                      }
                      ll += 1;
                    }
                  }
                  l += 1;
                }
              }
              i += 1;
            }
          }
        }
        swap(uu, vv);
        j += 1;
      }
    }
    var ans: dynamic = MOD;
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= m))
          {
            used[i][j] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      i = 1;
      while ((i <= (n * m)))
      {
        if ((ans > dp[i][((((1 << k)) - 1))]))
        {
          ans = dp[i][((((1 << k)) - 1))];
          p = i;
        }
        i += 1;
      }
    }
    doit(p, ((((1 << k)) - 1)));
    printf("%d\n", ans);
    {
      i = 1;
      while ((i <= n))
      {
        {
          j = 1;
          while ((j <= m))
          {
            if (used[i][j])
            {
              putchar(cpp_char("X"));
            } else
            {
              putchar(cpp_char("."));
            }
            j += 1;
          }
        }
        puts("");
        i += 1;
      }
    }
  }
  return 0;
}
