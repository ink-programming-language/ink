// Translated from solution.cpp.

func TEN(n: dynamic)
{
  return if (((n == 0))) 1 else (10 * TEN((n - 1)));
}

var N = 210;

class MT
{
  var g: dynamic = cpp_array(N, N);
  var mt: dynamic = cpp_array(N);
}

var res: dynamic;

var used = cpp_array(2, N);

func dfs(mt: dynamic, p: dynamic, b: dynamic)
{
  used[p][b] = true;
  if (cpp_binary((mt.mt[p] == -1), "and", (b == 1)))
  {
    res.push_back(p);
    return true;
  }
  if ((b == 0))
  {
    {
      var q = 0;
      while ((q < N))
      {
        if ((mt.g[p][q] == false))
        {
          q += 1;
          continue;
        }
        if (used[q][(1 - b)])
        {
          q += 1;
          continue;
        }
        if (dfs(mt, q, (1 - b)))
        {
          res.push_back(p);
          return true;
        }
        q += 1;
      }
    }
  } else
  {
    var q = mt.mt[p];
    if (cpp_binary((!used[q][(1 - b)]), "and", dfs(mt, q, (1 - b))))
    {
      res.push_back(p);
      return true;
    }
  }
  return false;
}

func findMwalk(mt: dynamic)
{
  res.clear();
  memset(used, 0, cpp_sizeof((used)));
  {
    var i = 0;
    while ((i < N))
    {
      if (cpp_binary((mt.mt[i] != -1), "or", used[i][0]))
      {
        i += 1;
        continue;
      }
      if (dfs(mt, i, 0))
      {
        return res;
      }
      i += 1;
    }
  }
  return vector();
}

func maxmt(mt: dynamic)
{
  var v = findMwalk(mt);
  if ((v.size() == 0))
  {
    return false;
  }
  var m = cpp_cast(v.size());
  var ok = cpp_array(N);
  fill_n(ok, N, -1);
  {
    var i = 0;
    while ((i < m))
    {
      var d = v[i];
      if ((ok[d] != -1))
      {
        var l = ok[d];
        var r = i;
        var nmt = mt;
        {
          var idx = (l + 1);
          while ((idx < r))
          {
            var j = v[idx];
            {
              var k = 0;
              while ((k < N))
              {
                if (cpp_binary(nmt.g[j][k], "and", (d != k)))
                {
                  nmt.g[d][k] = true;
                  nmt.g[k][d] = true;
                }
                nmt.g[j][k] = cpp_assign(nmt.g[k][j], "=", false);
                k += 1;
              }
            }
            nmt.mt[j] = -1;
            idx += 1;
          }
        }
        var f = maxmt(nmt);
        if ((!f))
        {
          return false;
        }
        copy_n(nmt.mt, N, mt.mt);
        var e = mt.mt[d];
        if ((e == -1))
        {
          return true;
        }
        mt.mt[d] = cpp_assign(mt.mt[e], "=", -1);
        {
          var idx = l;
          while ((idx < r))
          {
            var j = v[idx];
            if ((!mt.g[e][j]))
            {
              idx += 1;
              continue;
            }
            mt.mt[e] = j;
            mt.mt[j] = e;
            rotate((v.begin() + l), (v.begin() + idx), (v.begin() + r));
            assert(((((r - ((l + 1)))) % 2) == 0));
            {
              var k = (l + 1);
              while ((k < r))
              {
                mt.mt[v[k]] = v[(k + 1)];
                mt.mt[v[(k + 1)]] = v[k];
                k += 2;
              }
            }
            return true;
            idx += 1;
          }
        }
        assert(false);
      }
      ok[d] = i;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      mt.mt[v[i]] = v[(i + 1)];
      mt.mt[v[(i + 1)]] = v[i];
      i += 2;
    }
  }
  return true;
}

var first: dynamic;

func solve()
{
  var co = 0;
  while (maxmt(first))
  {
    co += 1;
  }
  return co;
}

func main()
{
  memset(first.g, 0, cpp_sizeof((first.g)));
  memset(first.mt, -1, cpp_sizeof((first.mt)));
  var n: dynamic;
  var m: dynamic;
  scanf("%d %d", (&n), (&m));
  n -= 1;
  var one = [];
  {
    var i = 0;
    while ((i < m))
    {
      var a: dynamic;
      var b: dynamic;
      scanf("%d %d", (&a), (&b));
      if ((a > b))
      {
        swap(a, b);
      }
      if ((a == 1))
      {
        one[(b - 2)] = true;
      } else
      {
        a -= 2;
        b -= 2;
        first.g[a][b] = cpp_assign(first.g[b][a], "=", true);
      }
      i += 1;
    }
  }
  solve();
  {
    var i = 0;
    while ((i < n))
    {
      if ((first.mt[i] == -1))
      {
        printf("No\n");
        return 0;
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      if ((!one[i]))
      {
        i += 1;
        continue;
      }
      first.g[i][n] = cpp_assign(first.g[n][i], "=", true);
      i += 1;
    }
  }
  var v = findMwalk(first);
  if ((v.size() == 0))
  {
    printf("No\n");
  } else
  {
    printf("Yes\n");
  }
  return 0;
}
