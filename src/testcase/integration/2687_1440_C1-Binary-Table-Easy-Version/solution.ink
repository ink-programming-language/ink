// Translated from solution.cpp.

func setIO(name: dynamic = "")
{
  ios_base.sync_with_stdio(null);
  cin.tie(null);
  cout.tie(null);
  if (cpp_cast((name).size()))
  {
    freopen(((name + ".in")).c_str(), "r", stdin);
    freopen(((name + ".out")).c_str(), "w", stdout);
  }
}

var inf = 1e9;

var INF = 1e18;

var mod = (1e9 + 7);

var MAXN = (1e6 + 5);

class point
{
  var x: dynamic;
  var y: dynamic;
}

var n: dynamic;

var m: dynamic;

var col = cpp_array(105, 105);

var res: dynamic;

func add(a: dynamic)
{
  col[a.x][a.y] ^= 1;
  res.push_back(a);
}

func fix(a: dynamic, b: dynamic, c: dynamic, d: dynamic)
{
  var black: dynamic;
  if (col[a.x][a.y])
  {
    black.push_back(a);
  }
  if (col[b.x][b.y])
  {
    black.push_back(b);
  }
  if (col[c.x][c.y])
  {
    black.push_back(c);
  }
  if (col[d.x][d.y])
  {
    black.push_back(d);
  }
  if ((cpp_cast((black).size()) == 0))
  {
    return;
  }
  if ((cpp_cast((black).size()) >= 3))
  {
    var j = 0;
    for (var i in black)
    {
      add(i);
      j += 1;
      if ((j == 3))
      {
        break;
      }
    }
    if ((cpp_cast((black).size()) > 3))
    {
      fix(a, b, c, d);
    }
    return;
  }
  if ((cpp_cast((black).size()) == 2))
  {
    var used: dynamic;
    for (var i in black)
    {
      add(i);
      used[make_pair(i.x, i.y)] = 1;
    }
    if ((used[make_pair(a.x, a.y)] == 0))
    {
      add(a);
      fix(a, b, c, d);
      return;
    }
    if ((used[make_pair(b.x, b.y)] == 0))
    {
      add(b);
      fix(a, b, c, d);
      return;
    }
    if ((used[make_pair(c.x, c.y)] == 0))
    {
      add(c);
      fix(a, b, c, d);
      return;
    }
    if ((used[make_pair(d.x, d.y)] == 0))
    {
      add(d);
      fix(a, b, c, d);
      return;
    }
  }
  if ((cpp_cast((black).size()) == 1))
  {
    if (col[a.x][a.y])
    {
      add(a);
      add(b);
      add(c);
      add(a);
      add(b);
      add(d);
      add(a);
      add(c);
      add(d);
      return;
    }
    if (col[b.x][b.y])
    {
      add(a);
      add(b);
      add(d);
      add(a);
      add(b);
      add(c);
      add(b);
      add(c);
      add(d);
      return;
    }
    if (col[c.x][c.y])
    {
      add(a);
      add(c);
      add(d);
      add(a);
      add(c);
      add(b);
      add(b);
      add(c);
      add(d);
      return;
    }
    if (col[d.x][d.y])
    {
      add(b);
      add(c);
      add(d);
      add(a);
      add(c);
      add(d);
      add(a);
      add(b);
      add(d);
      return;
    }
  }
}

func solve()
{
  read(n, m);
  res.clear();
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          var c: dynamic;
          read(c);
          if ((c == cpp_char("1")))
          {
            col[i][j] = 1;
          } else
          {
            col[i][j] = 0;
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
          if (((i < n) && (j < m)))
          {
            var a: dynamic;
            var b: dynamic;
            var c: dynamic;
            var d: dynamic;
            a.x = i;
            a.y = j;
            b.x = i;
            b.y = (j + 1);
            c.x = (i + 1);
            c.y = j;
            d.x = (i + 1);
            d.y = (j + 1);
            fix(a, b, c, d);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  write((cpp_cast((res).size()) / 3), cpp_char("\n"));
  {
    var i = 0;
    while ((i < cpp_cast((res).size())))
    {
      write(res[i].x, cpp_char(" "), res[i].y, cpp_char(" "), res[(i + 1)].x, cpp_char(" "), res[(i + 1)].y, cpp_char(" "), res[(i + 2)].x, cpp_char(" "), res[(i + 2)].y, cpp_char("\n"));
      i += 3;
    }
  }
}

func main()
{
  setIO();
  var tt = 1;
  read(tt);
  while (cpp_update(tt, "--"))
  {
    solve();
  }
  return 0;
}
