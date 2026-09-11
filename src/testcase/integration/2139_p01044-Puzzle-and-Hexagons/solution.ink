// Translated from solution.cpp.

func for_cpp(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i<b;++i)");
}

func for_rev(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a;i>=b;--i)");
}

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(n);++i)");
}

func allof(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func minit(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func size_of(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bit");
}

var dx: dynamic = [0, 1, 1, 0, -1, -1];

var dy: dynamic = [[1, 1, 0, -1, 0, 1], [1, 0, -1, -1, -1, 0]];

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var Q: dynamic = cpp_uninitialized();

var hanic: dynamic = cpp_array(55);

func rotate(x: dynamic, y: dynamic) -> dynamic
{
  if (((((x == 0) || (y == 0)) || (x == (W - 1))) || (y == (H - 1))))
  {
    return;
  }
  var xp: dynamic = (x % 2);
  var piv: dynamic = hanic[(y + dy[xp][5])][(x + dx[5])];
  for_rev(i, 5, 1);
  {
    hanic[(y + dy[xp][i])][(x + dx[i])] = hanic[(y + dy[xp][(i - 1)])][(x + dx[(i - 1)])];
  }
  hanic[(y + dy[xp][0])][(x + dx[0])] = piv;
}

func fall() -> dynamic
{
  var update: dynamic = true;
  while (update)
  {
    update = false;
    {
      var i: dynamic = 1;
      while (((i < H) && (!update)))
      {
        {
          var j: dynamic = 0;
          while (((j < W) && (!update)))
          {
            if ((hanic[i][j] == cpp_char(".")))
            {
              j += 1;
              continue;
            }
            var fal: dynamic = true;
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

var vis: dynamic = cpp_array(55, 55);

func rec(x: dynamic, y: dynamic, col: dynamic, vp: dynamic) -> dynamic
{
  vp.push_back(pii(x, y));
  vis[y][x] = 1;
  for_cpp(d, 0, 6);
  {
    var nx: dynamic = (x + dx[d]);
    var ny: dynamic = (y + dy[(x % 2)][d]);
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

func vanish() -> dynamic
{
  minit(vis, 0);
  var res: dynamic = false;
  for_cpp(y, 0, H);
  for_cpp(x, 0, W);
  {
    if (((!vis[y][x]) && (hanic[y][x] != cpp_char("."))))
    {
      var vp: dynamic = cpp_uninitialized();
      rec(x, y, hanic[y][x], vp);
      if ((vp.size() >= 3))
      {
        {
          var i: dynamic = 0;
          while ((i < vp.size()))
          {
            var p: dynamic = vp[i];
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

func main() -> dynamic
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
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
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
