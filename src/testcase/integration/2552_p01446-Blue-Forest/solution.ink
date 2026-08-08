// Translated from solution.cpp.

var X = cpp_array(30, 60);

var Y = cpp_array(30, 60);

var g2 = cpp_array(30, 30, 60);

var A = cpp_array(60);

var B = cpp_array(60);

var C = cpp_array(60);

var ijk = cpp_array(30, 60);

var v = cpp_array(30, 60);

var conv = cpp_array(30);

var m = cpp_array(60);

var EPS = 1e-9;

var g = cpp_array(30, 60);

func dist(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  return sqrt(((((X[a][b] - X[c][d])) * ((X[a][b] - X[c][d]))) + (((Y[a][b] - Y[c][d])) * ((Y[a][b] - Y[c][d])))));
}

func ABS(a: dynamic)
{
  return max(a, (-a));
}

var UF = cpp_array(60);

func FIND(a: dynamic)
{
  if ((UF[a] < 0))
  {
    return a;
  }
  return cpp_assign(UF[a], "=", FIND(UF[a]));
}

func UNION(a: dynamic, b: dynamic)
{
  a = FIND(a);
  b = FIND(b);
  if ((a == b))
  {
    return;
  }
  UF[a] += UF[b];
  UF[b] = a;
}

func main()
{
  var a: dynamic;
  while (cpp_comma(scanf("%d", (&a)), a))
  {
    {
      var i = 0;
      while ((i < 60))
      {
        {
          var j = 0;
          while ((j < 30))
          {
            g[i][j].clear();
            {
              var k = 0;
              while ((k < 30))
              {
                g2[i][j][k] = 0;
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
      var i = 0;
      while ((i < a))
      {
        UF[i] = -1;
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < a))
      {
        m[i].clear();
        var b: dynamic;
        scanf("%d", (&b));
        A[i] = b;
        {
          var j = 0;
          while ((j < b))
          {
            scanf("%d%d", (&X[i][j]), (&Y[i][j]));
            m[i][make_pair(X[i][j], Y[i][j])] = j;
            j += 1;
          }
        }
        var c: dynamic;
        scanf("%d", (&c));
        B[i] = c;
        {
          var j = 0;
          while ((j < c))
          {
            var p: dynamic;
            var q: dynamic;
            scanf("%d%d", (&p), (&q));
            p -= 1;
            q -= 1;
            g[i][p].push_back(make_pair(make_pair(i, q), dist(i, p, i, q)));
            g[i][q].push_back(make_pair(make_pair(i, p), dist(i, p, i, q)));
            g2[i][p][q] = cpp_assign(g2[i][q][p], "=", 1);
            j += 1;
          }
        }
        var d: dynamic;
        scanf("%d", (&d));
        C[i] = d;
        {
          var j = 0;
          while ((j < d))
          {
            var p: dynamic;
            var q: dynamic;
            var r: dynamic;
            scanf("%d%d%d", (&p), (&q), (&r));
            p -= 1;
            q -= 1;
            r -= 1;
            g[i][p].push_back(make_pair(make_pair(q, r), 0));
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < a))
      {
        {
          var j = (i + 1);
          while ((j < a))
          {
            if (((A[i] != A[j]) || (B[i] != B[j])))
            {
              j += 1;
              continue;
            }
            if ((FIND(i) == FIND(j)))
            {
              j += 1;
              continue;
            }
            {
              var k = 0;
              while ((k < A[j]))
              {
                if ((FIND(i) == FIND(j)))
                {
                  break;
                }
                {
                  var l = 0;
                  while ((l < A[j]))
                  {
                    if ((ABS((dist(i, 0, i, 1) - dist(j, k, j, l))) > EPS))
                    {
                      l += 1;
                      continue;
                    }
                    var th = (atan2((Y[i][1] - Y[i][0]), (X[i][1] - X[i][0])) - atan2((Y[j][l] - Y[j][k]), (X[j][l] - X[j][k])));
                    var ok = true;
                    {
                      var x = 0;
                      while ((x < A[j]))
                      {
                        var vx = (X[j][x] - X[j][k]);
                        var vy = (Y[j][x] - Y[j][k]);
                        var tx = ((cos(th) * vx) - (sin(th) * vy));
                        var ty = ((sin(th) * vx) + (cos(th) * vy));
                        var sx = cpp_cast((((X[i][0] + tx) + 0.5)));
                        var sy = cpp_cast((((Y[i][0] + ty) + 0.5)));
                        if ((((X[i][0] + tx) + 0.5) < 0))
                        {
                          sx -= 1;
                        }
                        if ((((Y[i][0] + ty) + 0.5) < 0))
                        {
                          sy -= 1;
                        }
                        if (((ABS(((X[i][0] + tx) - sx)) > EPS) || (ABS(((Y[i][0] + ty) - sy)) > EPS)))
                        {
                          ok = false;
                          break;
                        }
                        if ((m[i].count(make_pair(sx, sy)) == 0))
                        {
                          ok = false;
                          break;
                        }
                        conv[m[i][make_pair(sx, sy)]] = x;
                        x += 1;
                      }
                    }
                    if ((!ok))
                    {
                      l += 1;
                      continue;
                    }
                    {
                      var x = 0;
                      while ((x < A[i]))
                      {
                        {
                          var y = 0;
                          while ((y < g[i][x].size()))
                          {
                            if ((g[i][x][y].second < EPS))
                            {
                              y += 1;
                              continue;
                            }
                            if ((!g2[j][conv[x]][conv[g[i][x][y].first.second]]))
                            {
                              ok = false;
                              break;
                            }
                            y += 1;
                          }
                        }
                        x += 1;
                      }
                    }
                    if (ok)
                    {
                      UNION(i, j);
                      {
                        var x = 0;
                        while ((x < A[i]))
                        {
                          g[i][x].push_back(make_pair(make_pair(j, conv[x]), 0));
                          g[j][conv[x]].push_back(make_pair(make_pair(i, x), 0));
                          x += 1;
                        }
                      }
                      break;
                    }
                    l += 1;
                  }
                }
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var Q: dynamic;
    var s1: dynamic;
    var s2: dynamic;
    var d1: dynamic;
    var d2: dynamic;
    scanf("%d%d%d%d", (&s1), (&s2), (&d1), (&d2));
    s1 -= 1;
    s2 -= 1;
    d1 -= 1;
    d2 -= 1;
    Q.push(make_pair(0, make_pair(s1, s2)));
    {
      var i = 0;
      while ((i < 60))
      {
        {
          var j = 0;
          while ((j < 30))
          {
            ijk[i][j] = 999999999;
            v[i][j] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    ijk[s1][s2] = 0;
    while (Q.size())
    {
      var cost = (-Q.top().first);
      var at1 = Q.top().second.first;
      var at2 = Q.top().second.second;
      Q.pop();
      if (v[at1][at2])
      {
        continue;
      }
      v[at1][at2] = 1;
      {
        var i = 0;
        while ((i < g[at1][at2].size()))
        {
          var tr = g[at1][at2][i].first.first;
          var tc = g[at1][at2][i].first.second;
          var val = g[at1][at2][i].second;
          if (((!v[tr][tc]) && (ijk[tr][tc] > ((cost + val) + EPS))))
          {
            ijk[tr][tc] = (cost + val);
            Q.push(make_pair((-ijk[tr][tc]), make_pair(tr, tc)));
          }
          i += 1;
        }
      }
    }
    if ((ijk[d1][d2] > 999999998))
    {
      printf("-1\n");
    } else
    {
      printf("%f\n", ijk[d1][d2]);
    }
  }
}
