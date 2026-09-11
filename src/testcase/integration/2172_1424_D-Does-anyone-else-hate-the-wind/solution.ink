// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while ((!isdigit(ch)))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (isdigit(ch))
  {
    x = (((((x << 1)) + ((x << 3))) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var N: dynamic = (2e2 + 10);

var M: dynamic = (2e6 + 10);

var dx: dynamic = [-1, 1, 0, 0, 0];

var dy: dynamic = [0, 0, -1, 1, 0];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var W: dynamic = cpp_array(M);

var f: dynamic = cpp_array(M);

var sx: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

var mat: dynamic = cpp_array(N, N);

var vis: dynamic = cpp_array(N, N, N, 25);

var vec: dynamic = cpp_array(N, N);

class Node
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var last: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
}

var mp: dynamic = [[cpp_char("N"), 0], [cpp_char("S"), 1], [cpp_char("W"), 2], [cpp_char("E"), 3], [cpp_char("C"), 4]];

func check(x: dynamic, y: dynamic) -> dynamic
{
  if (((((x < 0) || (y < 0)) || (x >= n)) || (y >= m)))
  {
    return false;
  }
  if ((mat[x][y] == cpp_char("L")))
  {
    return false;
  }
  return true;
}

func ha(x: dynamic) -> dynamic
{
  printf("ha:%d %d %d %d\n", x.x, x.y, x.last, x.k);
}

func bfs() -> dynamic
{
  var q: dynamic = cpp_uninitialized();
  q.push([sx, sy, 0, k]);
  while ((!q.empty()))
  {
    var u: dynamic = q.front();
    q.pop();
    var x: dynamic = u.x;
    var y: dynamic = u.y;
    var last: dynamic = u.last;
    var cur: dynamic = u.k;
    var t: dynamic = ((f[last] + k) - cur);
    if ((mat[x][y] == cpp_char("P")))
    {
      return t;
    }
    if ((mat[x][y] == cpp_char("L")))
    {
      continue;
    }
    if (vis[last][x][y][cur])
    {
      continue;
    }
    vis[last][x][y][cur] = true;
    {
      var i: dynamic = 0;
      while ((i < vec[x][y].size()))
      {
        if ((t == f[vec[x][y][i]]))
        {
          cur = k;
          last = vec[x][y][i];
        }
        i += 1;
      }
    }
    if ((cur == 0))
    {
      continue;
    }
    u = [(u.x + dx[W[t]]), (u.y + dy[W[t]]), last, (cur - 1)];
    if (check(u.x, u.y))
    {
      q.push(u);
    }
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        var tx: dynamic = ((x + dx[i]) + dx[W[t]]);
        var ty: dynamic = ((y + dy[i]) + dy[W[t]]);
        if ((((check((x + dx[i]), (y + dy[i])) && check(tx, ty))) || ((check((x + dx[W[t]]), (y + dy[W[t]])) && check(tx, ty)))))
        {
          u = [tx, ty, last, (cur - 1)];
          if ((!vis[last][tx][ty][(cur - 1)]))
          {
            q.push(u);
          }
        }
        i += 1;
      }
    }
  }
  return -1;
}

func main() -> dynamic
{
  n = read();
  m = read();
  k = read();
  t = read();
  w = read();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(mat[i][j]);
          if ((mat[i][j] == cpp_char("M")))
          {
            sx = i;
            sy = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      var ch: dynamic = cpp_uninitialized();
      read(ch);
      W[i] = mp[ch];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= t))
    {
      var x: dynamic = read();
      var y: dynamic = read();
      vec[x][y].push_back(i);
      f[i] = read();
      i += 1;
    }
  }
  printf("%d\n", bfs());
  return 0;
}
