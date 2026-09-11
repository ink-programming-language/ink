// Translated from solution.cpp.

var N: dynamic = cpp_expression("#inc");

var W: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var mp: dynamic = cpp_array((N + 100), (N + 100));

func compress(x1: dynamic, w: dynamic) -> dynamic
{
  var xs: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      xs.push_back(x1[i]);
      i += 1;
    }
  }
  sort(xs.begin(), xs.end());
  xs.erase(unique(xs.begin(), xs.end()), xs.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      x1[i] = (find(xs.begin(), xs.end(), x1[i]) - xs.begin());
      i += 1;
    }
  }
  return xs.size();
}

func dfs(x: dynamic, y: dynamic, dir: dynamic) -> dynamic
{
  if (((((x < 0) || (y < 0)) || (x >= W)) || (y >= H)))
  {
    return 0;
  }
  var dx: dynamic = [0, 0, -1, 1];
  var dy: dynamic = [-1, 1, 0, 0];
  var ch: dynamic = mp[y][x];
  mp[y][x] = 0;
  if ((ch == cpp_char("^")))
  {
    dir = 0;
  }
  if ((ch == cpp_char("v")))
  {
    dir = 1;
  }
  if ((ch == cpp_char("<")))
  {
    dir = 2;
  }
  if ((ch == cpp_char(">")))
  {
    dir = 3;
  }
  var res: dynamic = (dfs((x + dx[dir]), (y + dy[dir]), dir) + ((ch != 0)));
  mp[y][x] = ch;
  return res;
}

func main() -> dynamic
{
  read(n);
  var x: dynamic = cpp_array(N);
  var y: dynamic = cpp_array(N);
  var ch: dynamic = cpp_array(N);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(x[i], y[i], ch[i]);
      i += 1;
    }
  }
  W = compress(x, 1e9);
  H = compress(y, 1e9);
  memset(mp, 0, cpp_sizeof((mp)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      mp[y[i]][x[i]] = ch[i];
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      {
        var j: dynamic = 0;
        while ((j < W))
        {
          if ((mp[i][j] != 0))
          {
            ans = max(ans, dfs(j, i, 0));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
