// Translated from solution.cpp.

func for_cpp(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i<b;++i)");
}

func for_rev(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a;i>=b;--i)");
}

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func allof(a: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func minit(a: dynamic, b: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func size_of(a: dynamic)
{
  return cpp_expression("#include <bit");
}

var dx = [0, 1, 1, 0, -1, -1];

var dy = [[1, 1, 0, -1, 0, 1], [1, 0, -1, -1, -1, 0]];

var H: dynamic;

var W: dynamic;

var Q: dynamic;

var hanic = cpp_array(55);

func rotate(x: dynamic, y: dynamic)
{
  if (((((x == 0) || (y == 0)) || (x == (W - 1))) || (y == (H - 1))))
  {
    return;
  }
  var xp = (x % 2);
  var piv = hanic[(y + dy[xp][5])][(x + dx[5])];
  for_rev(i, 5, 1);
  {
    hanic[(y + dy[xp][i])][(x + dx[i])] = hanic[(y + dy[xp][(i - 1)])][(x + dx[(i - 1)])];
  }
  hanic[(y + dy[xp][0])][(x + dx[0])] = piv;
}

func fall()
{
  var update = true;
  while (update)
  {
    update = false;
    {
      var i = 1;
      while (((i < H) && (!update)))
      {
        {
          var j = 0;
          while (((j < W) && (!update)))
          {
            if ((hanic[i][j] == cpp_char(".")))
            {
              j += 1;
              continue;
            }
            var fal = true;
            for_cpp(d, 2, 5);
            {
              if (((j == 0) && (d == 4)))
              {
                j += 1;
                continue;
              }
              if (((j == (W - 1)) && (d == 2)))
              {
                j += 1;
                continue;
              }
              fal &= ((hanic[(i + dy[(j % 2)][d])][(j + dx[d])] == cpp_char(".")));
            }
            if (fal)
            {
              hanic[(i + dy[(j % 2)][3])][(j + dx[3])] = hanic[i][j];
              hanic[i][j] = cpp_char(".");
              update = true;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}

var vis = cpp_array(55, 55);

func rec(x: dynamic, y: dynamic, col: dynamic, vp: dynamic)
{
  vp.push_back(pii(x, y));
  vis[y][x] = 1;
  for_cpp(d, 0, 6);
  {
    var nx = (x + dx[d]);
    var ny = (y + dy[(x % 2)][d]);
    if (((((nx < 0) || (nx >= W)) || (ny < 0)) || (ny >= H)))
    {
      continue;
    }
    if (vis[ny][nx])
    {
      continue;
    }
    if ((hanic[ny][nx] == col))
    {
      rec(nx, ny, col, vp);
    }
  }
}

func vanish()
{
  minit(vis, 0);
  var res = false;
  for_cpp(y, 0, H);
  for_cpp(x, 0, W);
  {
    if (((!vis[y][x]) && (hanic[y][x] != cpp_char("."))))
    {
      var vp: dynamic;
      rec(x, y, hanic[y][x], vp);
      if ((vp.size() >= 3))
      {
        {
          var i = 0;
          while ((i < vp.size()))
          {
            var p = vp[i];
            hanic[p.second][p.first] = cpp_char(".");
            i += 1;
          }
        }
        res = true;
      }
    }
  }
  return res;
}

func main()
{
  read(H, W);
  for_cpp(i, 0, H);
  read(hanic[((H - i) - 1)]);
  fall();
  while (vanish())
  {
    fall();
  }
  read(Q);
  for_cpp(i, 0, Q);
  {
    var x: dynamic;
    var y: dynamic;
    read(x, y);
    rotate(x, y);
    fall();
    while (vanish())
    {
      fall();
    }
  }
  ((for_cpp(i, 0, H) << hanic[((H - i) - 1)]) << endl);
}
