// Translated from solution.cpp.

var pi = acos(-1.0);

var eps = 1e-9;

var MAXN = 1000;

var a = cpp_array(MAXN, MAXN);

var u = cpp_array(MAXN, MAXN);

var b: dynamic;

var r: dynamic;

var n: dynamic;

var m: dynamic;

var dx = [-1, 0, 0, 1];

var dy = [0, -1, 1, 0];

func dfs(x: dynamic, y: dynamic, st: dynamic = true)
{
  u[x][y] = true;
  {
    var t = 0;
    while ((t < 4))
    {
      var xx = (x + dx[t]);
      var yy = (y + dy[t]);
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

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= n))
    {
      var s: dynamic;
      read(s);
      {
        var j = 1;
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
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
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
    var i = 0;
    while ((i < b.size()))
    {
      printf("B %d %d\n", b[i].first, b[i].second);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < r.size()))
    {
      printf("D %d %d\n", r[i].first, r[i].second);
      printf("R %d %d\n", r[i].first, r[i].second);
      i += 1;
    }
  }
}
