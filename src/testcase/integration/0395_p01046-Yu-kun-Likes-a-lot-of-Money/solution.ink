// Translated from solution.cpp.

var INF: dynamic = cpp_expression("#include <");

class uftree
{
  var par: dynamic = cpp_array(25);
  var rank: dynamic = cpp_array(25);
  func uftree() -> dynamic
  {
    }
  func init(n: dynamic) -> dynamic
  {
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          par[i] = i;
          rank[i] = 0;
          i += 1;
        }
      }
    }
  func find(x: dynamic) -> dynamic
  {
      if ((par[x] == x))
      {
        return x;
      }
      return cpp_assign(par[x], "=", find(par[x]));
    }
  func unite(x: dynamic, y: dynamic) -> dynamic
  {
      x = find(x);
      y = find(y);
      if ((x == y))
      {
        return;
      }
      if ((rank[x] < rank[y]))
      {
        par[x] = y;
      } else
      {
        if ((rank[x] == rank[y]))
        {
          rank[x] += 1;
        }
        par[y] = x;
      }
    }
  func same(x: dynamic, y: dynamic) -> dynamic
  {
      return (find(x) == find(y));
    }
}

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var vec: dynamic = cpp_uninitialized();

var tmp: dynamic = cpp_array(8);

var ki: dynamic = cpp_array(8);

var id: dynamic = cpp_array((1 << 25));

func dfs(x: dynamic, c: dynamic) -> dynamic
{
  if ((x == w))
  {
    var val: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        val += (tmp[i] * ki[i]);
        i += 1;
      }
    }
    vec.push_back(val);
  } else
  {
    if ((x == 0))
    {
      tmp[0] = 0;
      dfs((x + 1), 0);
      tmp[0] = 1;
      dfs((x + 1), 1);
    } else
    {
      if ((tmp[(x - 1)] == 0))
      {
        tmp[x] = (c + 1);
        dfs((x + 1), (c + 1));
      }
      {
        var i: dynamic = 0;
        while ((i <= c))
        {
          tmp[x] = i;
          dfs((x + 1), c);
          i += 1;
        }
      }
    }
  }
}

var dp: dynamic = cpp_array(2, 100000, 10);

func init() -> dynamic
{
  ki[0] = 1;
  {
    var i: dynamic = 1;
    while ((i < w))
    {
      ki[i] = (ki[(i - 1)] * 5);
      i += 1;
    }
  }
  dfs(0, 0);
  sort(vec.begin(), vec.end());
  memset(id, -1, cpp_sizeof((id)));
  {
    var i: dynamic = 0;
    while ((i < vec.size()))
    {
      id[vec[i]] = i;
      i += 1;
    }
  }
}

var fie: dynamic = cpp_array(8, 8);

var val: dynamic = cpp_array(64);

var sx: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

func is_person(r: dynamic) -> dynamic
{
  if ((sy != r))
  {
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if (((tmp[i] >= 1) && (sx == i)))
      {
        return 1;
      }
      i += 1;
    }
  }
  return 0;
}

func calc_rock(r: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if (((tmp[i] >= 1) && (fie[r][i] == -1)))
      {
        return -1;
      }
      if (((tmp[i] >= 1) && (fie[r][i] == -2)))
      {
        sum += R;
      }
      i += 1;
    }
  }
  return sum;
}

func calc_item(r: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if (((tmp[i] >= 1) && (fie[r][i] >= 1)))
      {
        sum += val[fie[r][i]];
      }
      i += 1;
    }
  }
  return sum;
}

func calc_id() -> dynamic
{
  var val: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      val += (tmp[i] * ki[i]);
      i += 1;
    }
  }
  if ((id[val] == -1))
  {
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        printf("%d ", tmp[i]);
        i += 1;
      }
    }
    printf("\n");
  }
  return id[val];
}

func calc_row0(r: dynamic, bit: dynamic) -> dynamic
{
  var sz: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if ((((bit >> i)) & 1))
      {
        if ((i == 0))
        {
          sz += 1;
          tmp[i] = sz;
        } else
        {
          if ((tmp[(i - 1)] != 0))
          {
            tmp[i] = tmp[(i - 1)];
          } else
          {
            sz += 1;
            tmp[i] = sz;
          }
        }
      } else
      {
        tmp[i] = 0;
      }
      i += 1;
    }
  }
  var cost_r: dynamic = calc_rock(r);
  if ((cost_r == -1))
  {
    return;
  }
  var flag: dynamic = is_person(r);
  var cost_i: dynamic = calc_item(r);
  var index: dynamic = calc_id();
  dp[(r + 1)][index][flag] = (cost_i - cost_r);
}

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

var fl1: dynamic = cpp_array(5);

var fl2: dynamic = cpp_array(5);

var tmp2: dynamic = cpp_array(8, 2);

var tmp3: dynamic = cpp_array(8, 2);

var colo: dynamic = cpp_array(8);

var st: dynamic = cpp_uninitialized();

var po: dynamic = cpp_uninitialized();

var uf: dynamic = cpp_uninitialized();

func dfs_row(x: dynamic, y: dynamic, sz: dynamic) -> dynamic
{
  tmp3[y][x] = sz;
  if ((y == 0))
  {
    if ((tmp2[y][x] > 0))
    {
      uf.unite(tmp2[y][x], sz);
    }
    st.push(tmp2[y][x]);
  } else
  {
    po = true;
  }
  {
    var i: dynamic = 0;
    while ((i < 4))
    {
      var nx: dynamic = (x + dx[i]);
      var ny: dynamic = (y + dy[i]);
      if (((((nx >= 0) && (nx < w)) && (ny >= 0)) && (ny < 2)))
      {
        if (((tmp2[ny][nx] != 0) && (tmp3[ny][nx] == 0)))
        {
          dfs_row(nx, ny, sz);
        }
      }
      i += 1;
    }
  }
}

func calc(r: dynamic, index_p: dynamic, flag_p: dynamic, bit: dynamic) -> dynamic
{
  memset(tmp2, 0, cpp_sizeof((tmp2)));
  {
    var v: dynamic = vec[index_p];
    var x: dynamic = 0;
    while ((v > 0))
    {
      if (((v % 5) >= 1))
      {
        tmp2[0][x] = (v % 5);
      } else
      {
        tmp2[0][x] = 0;
      }
      v /= 5;
      x += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if ((((bit >> i)) & 1))
      {
        tmp2[1][i] = 1;
      }
      i += 1;
    }
  }
  memset(fl1, false, cpp_sizeof((fl1)));
  memset(fl2, false, cpp_sizeof((fl2)));
  memset(tmp3, 0, cpp_sizeof((tmp3)));
  uf.init(6);
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if (((tmp2[0][i] != 0) && (tmp3[0][i] == 0)))
      {
        while (st.size())
        {
          st.pop();
        }
        po = false;
        dfs_row(i, 0, tmp2[0][i]);
        if (po)
        {
          while (st.size())
          {
            var v: dynamic = st.top();
            st.pop();
            fl2[v] = true;
          }
        }
      }
      i += 1;
    }
  }
  var cn: dynamic = 0;
  var cn2: dynamic = 0;
  {
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if ((tmp2[0][i] > 0))
        {
          fl1[tmp2[0][i]] = true;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i <= 4))
      {
        if (fl1[i])
        {
          cn += 1;
        }
        if (fl2[i])
        {
          cn2 += 1;
        }
        i += 1;
      }
    }
    if ((cn >= 2))
    {
      {
        var i: dynamic = 1;
        while ((i <= 4))
        {
          if ((fl1[i] && (!fl2[i])))
          {
            return;
          }
          i += 1;
        }
      }
    }
  }
  if ((cn2 > 0))
  {
    var prev: dynamic = -2;
    var kero: dynamic = -100;
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if (((tmp3[1][i] == 0) && (tmp2[1][i] == 1)))
        {
          if (((prev + 1) == i))
          {
            tmp3[1][i] = kero;
          } else
          {
            tmp3[1][i] = cpp_update(kero, "++");
          }
          prev = i;
        }
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if (((tmp3[1][i] == 0) && (tmp2[1][i] == 1)))
        {
          return;
        }
        i += 1;
      }
    }
  }
  {
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if ((tmp3[1][i] > 0))
        {
          tmp3[1][i] = uf.find(tmp3[1][i]);
        }
        i += 1;
      }
    }
  }
  {
    var vt: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        tmp[i] = tmp3[1][i];
        if ((tmp[i] != 0))
        {
          vt.push_back(tmp[i]);
        }
        i += 1;
      }
    }
    vt.push_back(-105);
    sort(vt.begin(), vt.end());
    vt.erase(unique(vt.begin(), vt.end()), vt.end());
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if ((tmp[i] == 0))
        {
          i += 1;
          continue;
        }
        tmp[i] = (lower_bound(vt.begin(), vt.end(), tmp[i]) - vt.begin());
        i += 1;
      }
    }
    var mp: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if ((tmp[i] == 0))
        {
          i += 1;
          continue;
        }
        if ((mp.find(tmp[i]) == mp.end()))
        {
          mp[tmp[i]] = mp.size();
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < w))
      {
        if ((tmp[i] == 0))
        {
          i += 1;
          continue;
        }
        tmp[i] = mp[tmp[i]];
        i += 1;
      }
    }
  }
  var cost_r: dynamic = calc_rock(r);
  if ((cost_r == -1))
  {
    return;
  }
  var flag: dynamic = (is_person(r) | flag_p);
  var cost_i: dynamic = calc_item(r);
  var index: dynamic = calc_id();
  dp[(r + 1)][index][flag] = max(dp[(r + 1)][index][flag], ((dp[r][index_p][flag_p] + cost_i) - cost_r));
  if ((((dp[r][index_p][flag_p] + cost_i) - cost_r) >= 0))
  {
  }
}

func is_ok(v: dynamic) -> dynamic
{
  while ((v > 0))
  {
    if (((v % 5) >= 2))
    {
      return false;
    }
    v /= 5;
  }
  return true;
}

func main(argument_0: dynamic) -> dynamic
{
  scanf("%d%d%d%d", (&h), (&w), (&n), (&R));
  init();
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      var s: dynamic = cpp_uninitialized();
      read(s);
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          if (((s[j] >= cpp_char("0")) && (s[j] <= cpp_char("9"))))
          {
            fie[i][j] = (((s[j] - cpp_char("0"))) + 1);
          }
          if (((s[j] >= cpp_char("a")) && (s[j] <= cpp_char("z"))))
          {
            fie[i][j] = (((s[j] - cpp_char("a"))) + 11);
          }
          if (((s[j] >= cpp_char("A")) && (s[j] <= cpp_char("Z"))))
          {
            fie[i][j] = (((s[j] - cpp_char("A"))) + 37);
          }
          if ((s[j] == cpp_char("@")))
          {
            sx = j;
            sy = i;
          }
          if ((s[j] == cpp_char("#")))
          {
            fie[i][j] = -1;
          }
          if ((s[j] == cpp_char("*")))
          {
            fie[i][j] = -2;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var c: dynamic = cpp_uninitialized();
      var a: dynamic = cpp_uninitialized();
      scanf(" %c%d", (&c), (&a));
      if (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
      {
        val[(((c - cpp_char("0"))) + 1)] = a;
      }
      if (((c >= cpp_char("a")) && (c <= cpp_char("z"))))
      {
        val[(((c - cpp_char("a"))) + 11)] = a;
      }
      if (((c >= cpp_char("A")) && (c <= cpp_char("Z"))))
      {
        val[(((c - cpp_char("A"))) + 37)] = a;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= h))
    {
      {
        var j: dynamic = 0;
        while ((j < vec.size()))
        {
          {
            var k: dynamic = 0;
            while ((k < 2))
            {
              dp[i][j][k] = (-INF);
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
    var j: dynamic = 0;
    while ((j < h))
    {
      {
        var i: dynamic = 0;
        while ((i < ((1 << w))))
        {
          calc_row0(j, i);
          i += 1;
        }
      }
      j += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < h))
    {
      {
        var j: dynamic = 1;
        while ((j < vec.size()))
        {
          {
            var k: dynamic = 0;
            while ((k < 2))
            {
              if ((dp[i][j][k] == (-INF)))
              {
                k += 1;
                continue;
              }
              {
                var l: dynamic = 0;
                while ((l < ((1 << w))))
                {
                  calc(i, j, k, l);
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
  var ans: dynamic = 0;
  {
    var j: dynamic = 1;
    while ((j <= h))
    {
      {
        var i: dynamic = 0;
        while ((i < vec.size()))
        {
          if (is_ok(vec[i]))
          {
            ans = max(ans, dp[j][i][1]);
          }
          i += 1;
        }
      }
      j += 1;
    }
  }
  printf("%d\n", ans);
  return 0;
}
