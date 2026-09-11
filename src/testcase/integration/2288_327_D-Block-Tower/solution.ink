// Translated from solution.cpp.

var pi: dynamic = acos(-1.0);

var eps: dynamic = 1e-9;

var MAXN: dynamic = 1000;

var a: dynamic = cpp_array(MAXN, MAXN);

var u: dynamic = cpp_array(MAXN, MAXN);

var b: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var dx: dynamic = [-1, 0, 0, 1];

var dy: dynamic = [0, -1, 1, 0];

func dfs(x: dynamic, y: dynamic, st: dynamic = true) -> dynamic
{
  u[x][y] = true;
  {
    var t: dynamic = 0;
    while ((t < 4))
    {
      var xx: dynamic = (x + dx[t]);
      var yy: dynamic = (y + dy[t]);
      if (((a[xx][yy] == 1) && (!u[xx][yy])))
      {
        dfs(xx, yy, false);
      }
      t += 1;
    }
  }
  if ((!st))
  {
    r.push_back(pair(x, y));
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      var s: dynamic = cpp_uninitialized();
      read(s);
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          if ((s[(j - 1)] == cpp_char(".")))
          {
            a[i][j] = 1;
            b.push_back(pair(i, j));
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 1;
        while ((j <= m))
        {
          if ((a[i][j] && (!u[i][j])))
          {
            dfs(i, j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write((b.size() + (r.size() * 2)), "\n");
  {
    var i: dynamic = 0;
    while ((i < b.size()))
    {
      printf("B %d %d\n", b[i].first, b[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < r.size()))
    {
      printf("D %d %d\n", r[i].first, r[i].second);
      printf("R %d %d\n", r[i].first, r[i].second);
      i += 1;
    }
  }
}
