// Translated from solution.cpp.

func rep(i: dynamic, j: dynamic)
{
  return cpp_expression("#include <iostre");
}

func REP(i: dynamic, j: dynamic, k: dynamic)
{
  cpp_macro("for(int i=(j);(i)<(k);++i)");
}

func BW(a: dynamic, x: dynamic, b: dynamic)
{
  return cpp_expression("#include <iostream>");
}

var MP = cpp_expression("#include");

var PB = cpp_expression("#include");

var F = cpp_expression("#incl");

var S = cpp_expression("#inclu");

var INF = cpp_expression("#includ");

var EPS = cpp_expression("#incl");

var H: dynamic;

var W: dynamic;

var t = cpp_array(20, 20);

var dy = [0, 1];

var dx = [1, 0];

func judge(y: dynamic, x: dynamic)
{
  var m: dynamic;
  {
    var i = -1;
    while ((i < 1))
    {
      {
        var j = -1;
        while ((j < 1))
        {
          var ny = (y + i);
          var nx = (x + j);
          if (((((ny < 0) || (nx < 0)) || (t[ny][nx] == -1)) || m.count(t[ny][nx])))
          {
            j += 1;
            continue;
          }
          m[t[ny][nx]] = 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((m.size() < 4))
  {
    return 1;
  }
  return 0;
}

func dfs(n: dynamic)
{
  var i: dynamic;
  var j: dynamic;
  var res = 0;
  {
    i = 0;
    while ((i < H))
    {
      {
        j = 0;
        while ((j < W))
        {
          if ((t[i][j] == -1))
          {
            cpp_goto("goto b;");
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if (((i == H) && (j == W)))
  {
    return 1;
  }
  var tmp = cpp_array(20, 20);
  rep(i, H);
  rep(j, W)[i][j] = t[i][j];
  t[i][j] = n;
  rep(d, 2);
  {
    var x = (j + dx[d]);
    var y = (i + dy[d]);
    if ((((((x < 0) || (x >= W)) || (y < 0)) || (y >= H)) || (t[y][x] != -1)))
    {
      continue;
    }
    t[y][x] = n;
    if (judge(i, j))
    {
      res += dfs((n + 1));
    }
    t[y][x] = -1;
  }
  rep(i, H);
  rep(j, W)[i][j] = tmp[i][j];
  return res;
}

func main()
{
  while ((scanf("%d%d", (&H), (&W)) && (H + W)))
  {
    memset(t, -1, cpp_sizeof((t)));
    printf("%d\n", dfs(0));
  }
  return 0;
}
