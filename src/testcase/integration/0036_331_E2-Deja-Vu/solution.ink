// Translated from solution.cpp.

var MAX = (100 + 10);

var Mod = (cpp_cast(1e9) + 7);

var n: dynamic;

var m: dynamic;

var g = cpp_array(MAX, MAX);

var can = cpp_array(MAX, MAX);

var p = cpp_array(MAX, MAX);

func get(before: dynamic, kind: dynamic, could: dynamic, have: dynamic)
{
  var j: dynamic;
  var now = 0;
  var cc = have;
  {
    while ((now < cpp_cast(before.size())))
    {
      if (((now + 1) == cpp_cast(before.size())))
      {
        break;
      }
      var a = before[now];
      var b = before[(now + 1)];
      if (((g[a][b] != kind) || (((!could) && can[a][b]))))
      {
        return 0;
      }
      var nL = p[a][b];
      {
        (j) = (0);
        while (((j) != (cpp_cast(nL.size()))))
        {
          if ((cc < cpp_cast(before.size())))
          {
            if ((nL[j] == before[cc]))
            {
              cc += 1;
            } else
            {
              return 0;
            }
          } else
          {
            while ((j < cpp_cast(nL.size())))
            {
              before.push_back(nL[cpp_update(j, "++")]);
              if ((cpp_cast(before.size()) > ((2 * n) + 1)))
              {
                return 0;
              }
            }
            cc = before.size();
            break;
          }
          (j) += 1;
        }
      }
      now += 1;
    }
  }
  return (cc == cpp_cast(before.size()));
}

func isCan(a: dynamic, b: dynamic)
{
  var i: dynamic;
  var L = p[a][b];
  var len = L.size();
  {
    (i) = (0);
    while (((i) <= ((len - 2))))
    {
      if (((L[i] == a) && (L[(i + 1)] == b)))
      {
        return 1;
      }
      (i) += 1;
    }
  }
  return 0;
}

var f_Before = cpp_array(MAX, MAX, MAX);

var f_After = cpp_array(MAX, MAX, MAX);

var tmp = cpp_array(MAX, MAX, MAX);

var Before = cpp_array(MAX, MAX);

var After = cpp_array(MAX, MAX);

var ans = cpp_array(MAX);

func work(u: dynamic, kind: dynamic, f: dynamic)
{
  var v: dynamic;
  {
    (v) = (1);
    while (((v) <= (n)))
    {
      if (((g[u][v] == kind) && (!can[v][u])))
      {
        var after: dynamic;
        after.push_back(u);
        after.push_back(v);
        if ((!get(after, kind, (kind != 2), 1)))
        {
          (v) += 1;
          continue;
        }
        var Len = after.size();
        f[u][after[(Len - 1)]][(Len - 1)] += 1;
      }
      (v) += 1;
    }
  }
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  if ((a >= Mod))
  {
    a -= Mod;
  }
}

func work2(a: dynamic, b: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var L = p[a][b];
  var len = L.size();
  {
    (i) = (0);
    while (((i) <= ((len - 2))))
    {
      if (((L[i] == a) && (L[(i + 1)] == b)))
      {
        var before: dynamic;
        var after: dynamic;
        {
          j = i;
          while ((j >= 0))
          {
            before.push_back(L[j]);
            j -= 1;
          }
        }
        {
          j = (i + 1);
          while ((j < len))
          {
            after.push_back(L[j]);
            j += 1;
          }
        }
        if ((get(before, 2, 0, before.size()) && get(after, 1, 1, after.size())))
        {
          reverse(before.begin(), before.end());
          var A = before[0];
          var B = after[(after.size() - 1)];
          var Len = ((before.size() + after.size()) - 1);
          var k: dynamic;
          var l: dynamic;
          var o: dynamic;
          {
            (l) = (1);
            while (((l) <= (n)))
            {
              {
                (o) = (0);
                while (((o) <= ((2 * n))))
                {
                  if (tmp[A][l][o])
                  {
                    {
                      (k) = (1);
                      while (((k) <= (n)))
                      {
                        if ((((g[k][l] == 1) && (p[k][l].size() == 0)) && (((Len + 1) + o) <= (2 * n))))
                        {
                          add(f_After[k][B][((Len + 1) + o)], tmp[A][l][o]);
                        }
                        (k) += 1;
                      }
                    }
                  }
                  (o) += 1;
                }
              }
              (l) += 1;
            }
          }
        } else
        {
          return;
        }
      }
      (i) += 1;
    }
  }
  return;
}

var tot = 0;

var First: dynamic;

func Dp1(f: dynamic, after: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  var o: dynamic;
  memset(tmp, 0, cpp_sizeof(tmp));
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      tmp[i][i][0] = 1;
      (i) += 1;
    }
  }
  var up = (2 * n);
  {
    (l) = (1);
    while (((l) <= (up)))
    {
      {
        (i) = (1);
        while (((i) <= (n)))
        {
          {
            (j) = (1);
            while (((j) <= (n)))
            {
              {
                (o) = (0);
                while (((o) <= (l)))
                {
                  if (f[i][j][o])
                  {
                    {
                      (k) = (1);
                      while (((k) <= (n)))
                      {
                        if (tmp[j][k][(l - o)])
                        {
                          add(tmp[i][k][l], ((cpp_cast(f[i][j][o]) * tmp[j][k][(l - o)]) % Mod));
                        }
                        (k) += 1;
                      }
                    }
                  }
                  (o) += 1;
                }
              }
              (j) += 1;
            }
          }
          (i) += 1;
        }
      }
      (l) += 1;
    }
  }
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      {
        (j) = (1);
        while (((j) <= (n)))
        {
          {
            (l) = (0);
            while (((l) <= ((2 * n))))
            {
              add(after[i][l], tmp[i][j][l]);
              (l) += 1;
            }
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
}

func Dp2(f: dynamic, after: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  var l: dynamic;
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      after[i][0] = 1;
      (i) += 1;
    }
  }
  var up = (2 * n);
  {
    (l) = (1);
    while (((l) <= (up)))
    {
      {
        (i) = (1);
        while (((i) <= (n)))
        {
          {
            (j) = (1);
            while (((j) <= (n)))
            {
              {
                (k) = (0);
                while (((k) != (l)))
                {
                  add(After[i][l], ((cpp_cast(f[i][j][(l - k)]) * After[j][k]) % Mod));
                  (k) += 1;
                }
              }
              (j) += 1;
            }
          }
          (i) += 1;
        }
      }
      (l) += 1;
    }
  }
}

func check(a: dynamic, b: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var L = p[a][b];
  var len = L.size();
  {
    (i) = (0);
    while (((i) <= ((len - 2))))
    {
      if (((L[i] == a) && (L[(i + 1)] == b)))
      {
        var before: dynamic;
        var after: dynamic;
        {
          j = i;
          while ((j >= 0))
          {
            before.push_back(L[j]);
            j -= 1;
          }
        }
        {
          j = (i + 1);
          while ((j < len))
          {
            after.push_back(L[j]);
            j += 1;
          }
        }
        if ((get(before, 2, 0, before.size()) && get(after, 1, 1, after.size())))
        {
          reverse(before.begin(), before.end());
          var A = before[0];
          var B = after[(after.size() - 1)];
          var Len = ((before.size() + after.size()) - 1);
          var l1: dynamic;
          var l2: dynamic;
          {
            (l1) = (0);
            while (((l1) <= ((2 * n))))
            {
              {
                (l2) = (0);
                while (((l2) <= ((2 * n))))
                {
                  if ((((l1 + l2) + Len) <= (2 * n)))
                  {
                    add(ans[((l1 + l2) + Len)], ((cpp_cast(Before[A][l1]) * After[B][l2]) % Mod));
                  }
                  (l2) += 1;
                }
              }
              (l1) += 1;
            }
          }
        } else
        {
          return;
        }
      }
      (i) += 1;
    }
  }
  return;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  scanf("%d%d", (&n), (&m));
  {
    (i) = (1);
    while (((i) <= (m)))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d%d", (&a), (&b));
      if ((!First))
      {
        First = a;
      }
      g[a][b] = 1;
      g[b][a] = 2;
      var k: dynamic;
      var first: dynamic;
      scanf("%d", (&k));
      while (cpp_update(k, "--"))
      {
        scanf("%d", (&first));
        p[a][b].push_back(first);
      }
      p[b][a] = p[a][b];
      reverse(p[b][a].begin(), p[b][a].end());
      (i) += 1;
    }
  }
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      {
        (j) = (1);
        while (((j) <= (n)))
        {
          if (((g[i][j] == 1) && isCan(i, j)))
          {
            can[j][i] = cpp_assign(can[i][j], "=", 1);
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      work(i, 1, f_After);
      (i) += 1;
    }
  }
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      work(i, 2, f_Before);
      (i) += 1;
    }
  }
  Dp1(f_Before, Before);
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      {
        (j) = (1);
        while (((j) <= (n)))
        {
          if (((g[i][j] == 1) && (can[i][j] == 1)))
          {
            work2(i, j);
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  Dp2(f_After, After);
  {
    (i) = (1);
    while (((i) <= (n)))
    {
      {
        (j) = (1);
        while (((j) <= (n)))
        {
          if (((g[i][j] == 1) && (can[i][j] == 1)))
          {
            check(i, j);
          }
          (j) += 1;
        }
      }
      (i) += 1;
    }
  }
  {
    (i) = (1);
    while (((i) <= ((2 * n))))
    {
      write(ans[i], "\n");
      (i) += 1;
    }
  }
  return 0;
}
