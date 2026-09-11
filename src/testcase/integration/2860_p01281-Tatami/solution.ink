// Translated from solution.cpp.

func rep(i: dynamic, j: dynamic) -> dynamic
{
  return cpp_expression("#include <iostre");
}

func REP(i: dynamic, j: dynamic, k: dynamic) -> dynamic
{
  cpp_macro("for(int i=(j);(i)<(k);++i)");
}

func BW(a: dynamic, x: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream>");
}

var MP: dynamic = cpp_expression("#include");

var PB: dynamic = cpp_expression("#include");

var F: dynamic = cpp_expression("#incl");

var S: dynamic = cpp_expression("#inclu");

var INF: dynamic = cpp_expression("#includ");

var EPS: dynamic = cpp_expression("#incl");

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var t: dynamic = cpp_array(20, 20);

var dy: dynamic = [0, 1];

var dx: dynamic = [1, 0];

func judge(y: dynamic, x: dynamic) -> dynamic
{
  var m: dynamic = cpp_uninitialized();
  {
    var i: dynamic = -1;
    while ((i < 1))
    {
      {
        var j: dynamic = -1;
        while ((j < 1))
        {
          var ny: dynamic = (y + i);
          var nx: dynamic = (x + j);
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

func dfs(n: dynamic) -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var res: dynamic = 0;
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
  var tmp: dynamic = cpp_array(20, 20);
  rep(i, H);
  rep(j, W)[i][j] = t[i][j];
  t[i][j] = n;
  rep(d, 2);
  {
    var x: dynamic = (j + dx[d]);
    var y: dynamic = (i + dy[d]);
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

func main() -> dynamic
{
  while ((scanf("%d%d", (&H), (&W)) && (H + W)))
  {
    memset(t, -1, cpp_sizeof((t)));
    printf("%d\n", dfs(0));
  }
  return 0;
}
