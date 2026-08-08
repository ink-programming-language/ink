// Translated from solution.cpp.

var INF = 1e9;

var LINF = 1e18;

func operator_shift_left(out: dynamic, o: dynamic)
{
  (((((out << "(") << o.first) << ",") << o.second) << ")");
  return out;
}

func operator_shift_left(out: dynamic, V: dynamic)
{
  {
    var i = 0;
    while ((i < V.size()))
    {
      (out << V[i]);
      if ((i != (V.size() - 1)))
      {
        (out << " ");
      }
      i += 1;
    }
  }
  return out;
}

func operator_shift_left(out: dynamic, Mat: dynamic)
{
  {
    var i = 0;
    while ((i < Mat.size()))
    {
      if ((i != 0))
      {
        (out << endl);
      }
      (out << Mat[i]);
      i += 1;
    }
  }
  return out;
}

func operator_shift_left(out: dynamic, mp: dynamic)
{
  (out << "{ ");
  {
    var it = mp.begin();
    while ((it != mp.end()))
    {
      (((out << it->first) << ":") << it->second);
      if (((mp.size() - 1) != distance(mp.begin(), it)))
      {
        (out << ", ");
      }
      it += 1;
    }
  }
  (out << " }");
  return out;
}

var dx = [1, 0];

var dy = [0, 1];

var MAX_WH = cpp_expression("#i");

var alpha = cpp_array(30);

var dp = cpp_array(MAX_WH, MAX_WH, MAX_WH, MAX_WH);

var can = cpp_array(MAX_WH, MAX_WH, MAX_WH, MAX_WH);

func dfs(y1: dynamic, x1: dynamic, y2: dynamic, x2: dynamic, maze: dynamic)
{
  var ret = dp[y1][x1][y2][x2];
  if ((ret != -1))
  {
    return ret;
  }
  if (((y1 == y2) && (x1 == x2)))
  {
    return cpp_assign(ret, "=", 0);
  }
  ret = (-INF);
  if ((maze[y1][x1] == cpp_char("#")))
  {
    return ret;
  }
  if ((((y1 + 1) <= y2) && (maze[(y1 + 1)][x1] != cpp_char("#"))))
  {
    ret = max(ret, dfs((y1 + 1), x1, y2, x2, maze));
  }
  if ((((x1 + 1) <= x2) && (maze[y1][(x1 + 1)] != cpp_char("#"))))
  {
    ret = max(ret, dfs(y1, (x1 + 1), y2, x2, maze));
  }
  var c = maze[y1][x1];
  if (((c >= cpp_char("a")) && (c <= cpp_char("z"))))
  {
    var idx = (c - cpp_char("a"));
    for (var p in alpha[idx])
    {
      var ny = p.first;
      var nx = p.second;
      if (((ny > y2) || (nx > x2)))
      {
        continue;
      }
      if (((abs((ny - y1)) + abs((nx - x1))) == 1))
      {
        ret = max(ret, (dfs(ny, nx, y2, x2, maze) + 1));
      } else
      {
        {
          var i = 0;
          while ((i < 2))
          {
            {
              var j = 0;
              while ((j < 2))
              {
                var innery1 = (y1 + dy[i]);
                var innerx1 = (x1 + dx[i]);
                var innery2 = (ny - dy[j]);
                var innerx2 = (nx - dx[j]);
                if (((((innery1 > y2) || (innerx1 > x2)) || (innery2 < y1)) || (innerx2 < x1)))
                {
                  j += 1;
                  continue;
                }
                if ((!can[innery1][innerx1][innery2][innerx2]))
                {
                  j += 1;
                  continue;
                }
                ret = max(ret, ((dfs(innery1, innerx1, innery2, innerx2, maze) + dfs(ny, nx, y2, x2, maze)) + 1));
                j += 1;
              }
            }
            i += 1;
          }
        }
      }
    }
  }
  return ret;
}

func solve(H: dynamic, W: dynamic)
{
  var res = -1;
  var maze = cpp_construct((H + 2), vector((W + 2), cpp_char("#")));
  {
    var i = 0;
    while ((i < 30))
    {
      alpha[i].clear();
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= H))
    {
      {
        var j = 1;
        while ((j <= W))
        {
          var c: dynamic;
          read(c);
          maze[i][j] = c;
          if (((c >= cpp_char("A")) && (c <= cpp_char("Z"))))
          {
            alpha[(c - cpp_char("A"))].push_back([i, j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(can, false, cpp_sizeof((can)));
  {
    var i = H;
    while ((i >= 1))
    {
      {
        var j = W;
        while ((j >= 1))
        {
          if ((maze[i][j] == cpp_char("#")))
          {
            j -= 1;
            continue;
          }
          can[i][j][i][j] = 1;
          if ((maze[(i + 1)][j] != cpp_char("#")))
          {
            {
              var ii = (i + 1);
              while ((ii <= H))
              {
                {
                  var jj = j;
                  while ((jj <= W))
                  {
                    can[i][j][ii][jj] |= can[(i + 1)][j][ii][jj];
                    jj += 1;
                  }
                }
                ii += 1;
              }
            }
          }
          if ((maze[i][(j + 1)] != cpp_char("#")))
          {
            {
              var ii = i;
              while ((ii <= H))
              {
                {
                  var jj = (j + 1);
                  while ((jj <= W))
                  {
                    can[i][j][ii][jj] |= can[i][(j + 1)][ii][jj];
                    jj += 1;
                  }
                }
                ii += 1;
              }
            }
          }
          j -= 1;
        }
      }
      i -= 1;
    }
  }
  memset(dp, -1, cpp_sizeof((dp)));
  res = max(res, dfs(1, 1, H, W, maze));
  return res;
}

func main(argument_0: dynamic)
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var H: dynamic;
  var W: dynamic;
  while (cpp_comma(((cin >> H) >> W), (H | W)))
  {
    write(solve(H, W), "\n");
  }
  return 0;
}
