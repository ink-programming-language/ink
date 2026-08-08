// Translated from solution.cpp.

var N = cpp_expression("#inc");

var W: dynamic;

var H: dynamic;

var n: dynamic;

var mp = cpp_array((N + 100), (N + 100));

func compress(x1: dynamic, w: dynamic)
{
  var xs: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      xs.push_back(x1[i]);
      i += 1;
    }
  }
  sort(xs.begin(), xs.end());
  xs.erase(unique(xs.begin(), xs.end()), xs.end());
  {
    var i = 0;
    while ((i < n))
    {
      x1[i] = (find(xs.begin(), xs.end(), x1[i]) - xs.begin());
      i += 1;
    }
  }
  return xs.size();
}

func dfs(x: dynamic, y: dynamic, dir: dynamic)
{
  if (((((x < 0) || (y < 0)) || (x >= W)) || (y >= H)))
  {
    return 0;
  }
  var dx = [0, 0, -1, 1];
  var dy = [-1, 1, 0, 0];
  var ch = mp[y][x];
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
  var res = (dfs((x + dx[dir]), (y + dy[dir]), dir) + ((ch != 0)));
  mp[y][x] = ch;
  return res;
}

func main()
{
  read(n);
  var x = cpp_array(N);
  var y = cpp_array(N);
  var ch = cpp_array(N);
  {
    var i = 0;
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
    var i = 0;
    while ((i < n))
    {
      mp[y[i]][x[i]] = ch[i];
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
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
