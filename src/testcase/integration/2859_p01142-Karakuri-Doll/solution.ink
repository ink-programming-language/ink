// Translated from solution.cpp.

var W: dynamic = cpp_uninitialized();

var H: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

var sx: dynamic = cpp_uninitialized();

var sd: dynamic = cpp_uninitialized();

var gy: dynamic = cpp_uninitialized();

var gx: dynamic = cpp_uninitialized();

var gd: dynamic = cpp_uninitialized();

var field: dynamic = cpp_array(64, 16);

var used: dynamic = cpp_array(4, 64, 16, 4, 64, 16);

var used2: dynamic = cpp_array(4, 64, 16);

var dx: dynamic = [0, 1, 0, -1];

var dy: dynamic = [-1, 0, 1, 0];

func init() -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < 16))
    {
      {
        var j: dynamic = 0;
        while ((j < 64))
        {
          {
            var k: dynamic = 0;
            while ((k < 4))
            {
              {
                var l: dynamic = 0;
                while ((l < 16))
                {
                  {
                    var m: dynamic = 0;
                    while ((m < 64))
                    {
                      {
                        var n: dynamic = 0;
                        while ((n < 4))
                        {
                          used[i][j][k][l][m][n] = false;
                          n += 1;
                        }
                      }
                      m += 1;
                    }
                  }
                  l += 1;
                }
              }
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
}

func front(y: dynamic, x: dynamic, d: dynamic) -> dynamic
{
  if ((d == 0))
  {
    var Y: dynamic = y;
    {
      while (field[(Y - 1)][x])
      {
        Y -= 1;
      }
    }
    return [Y, x];
  } else if ((d == 1))
  {
    var X: dynamic = x;
    {
      while (field[y][(X + 1)])
      {
        X += 1;
      }
    }
    return [y, X];
  } else if ((d == 2))
  {
    var Y: dynamic = y;
    {
      while (field[(Y + 1)][x])
      {
        Y += 1;
      }
    }
    return [Y, x];
  } else
  {
    var X: dynamic = x;
    {
      while (field[y][(X - 1)])
      {
        X -= 1;
      }
    }
    return [y, X];
  }
}

func back(y: dynamic, x: dynamic, d: dynamic) -> dynamic
{
  var ans: dynamic = cpp_uninitialized();
  var R: dynamic = 1;
  var L: dynamic = 3;
  if ((d == 0))
  {
    var Y: dynamic = y;
    {
      while (field[Y][x])
      {
        if (cpp_unary("not", field[Y][(x + 1)]))
        {
          ans.push_back([[Y, x], L]);
        }
        if (cpp_unary("not", field[Y][(x - 1)]))
        {
          ans.push_back([[Y, x], R]);
        }
        Y -= 1;
      }
    }
  } else if ((d == 1))
  {
    var X: dynamic = x;
    {
      while (field[y][X])
      {
        if (cpp_unary("not", field[(y + 1)][X]))
        {
          ans.push_back([[y, X], L]);
        }
        if (cpp_unary("not", field[(y - 1)][X]))
        {
          ans.push_back([[y, X], R]);
        }
        X += 1;
      }
    }
  } else if ((d == 2))
  {
    var Y: dynamic = y;
    {
      while (field[Y][x])
      {
        if (cpp_unary("not", field[Y][(x + 1)]))
        {
          ans.push_back([[Y, x], R]);
        }
        if (cpp_unary("not", field[Y][(x - 1)]))
        {
          ans.push_back([[Y, x], L]);
        }
        Y += 1;
      }
    }
  } else
  {
    var X: dynamic = x;
    {
      while (field[y][X])
      {
        if (cpp_unary("not", field[(y + 1)][X]))
        {
          ans.push_back([[y, X], R]);
        }
        if (cpp_unary("not", field[(y - 1)][X]))
        {
          ans.push_back([[y, X], L]);
        }
        X -= 1;
      }
    }
  }
  return ans;
}

func dfs(fy: dynamic, fx: dynamic, fd: dynamic, by: dynamic, bx: dynamic, bd: dynamic) -> dynamic
{
  tie(fy, fx) = front(fy, fx, fd);
  var cand: dynamic = back(by, bx, bd);
  for (var next: dynamic in cand)
  {
    var div: dynamic = next.second;
    by = next.first.first;
    bx = next.first.second;
    var newfd: dynamic = (((fd + div)) % 4);
    var newbd: dynamic = (((bd + div)) % 4);
    if (cpp_binary(cpp_binary(cpp_binary(cpp_binary((fy == gy), "and", (fx == gx)), "and", (by == gy)), "and", (bx == gx)), "and", (bd == gd)))
    {
      return true;
    }
    if (cpp_unary("not", used[fy][fx][newfd][by][bx][newbd]))
    {
      used[fy][fx][newfd][by][bx][newbd] = true;
      var sub: dynamic = dfs(fy, fx, newfd, by, bx, newbd);
      if (sub)
      {
        return true;
      }
    }
  }
  return false;
}

func dfs(fy: dynamic, fx: dynamic, fd: dynamic) -> dynamic
{
  if (cpp_binary((fy == gy), "and", (fx == gx)))
  {
    return true;
  }
  tie(fy, fx) = front(fy, fx, fd);
  {
    var i: dynamic = -1;
    while ((i <= 1))
    {
      var nd: dynamic = ((((fd + i) + 4)) % 4);
      if (cpp_unary("not", used2[fy][fx][nd]))
      {
        used2[fy][fx][nd] = true;
        if (dfs(fy, fx, nd))
        {
          return true;
        }
      }
      i += 2;
    }
  }
  return false;
}

func main() -> dynamic
{
  var cnt: dynamic = 0;
  while (true)
  {
    cnt += 1;
    read(W, H);
    if (cpp_binary((W == 0), "and", (H == 0)))
    {
      break;
    }
    {
      var i: dynamic = 0;
      while ((i < H))
      {
        {
          var j: dynamic = 0;
          while ((j < W))
          {
            var c: dynamic = cpp_uninitialized();
            read(c);
            field[i][j] = (c != cpp_char("#"));
            if ((c == cpp_char("K")))
            {
              sy = i;
              sx = j;
            } else if ((c == cpp_char("M")))
            {
              gy = i;
              gx = j;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        if (field[(sy + dy[i])][(sx + dx[i])])
        {
          sd = i;
          break;
        }
        i += 1;
      }
    }
    {
      var i: dynamic = 0;
      while ((i < 4))
      {
        if (field[(gy + dy[i])][(gx + dx[i])])
        {
          gd = i;
          break;
        }
        i += 1;
      }
    }
    gd = (((gd + 2)) % 4);
    var ans: dynamic = false;
    {
      var d: dynamic = 0;
      while ((d < 4))
      {
        if ((d > 0))
        {
          init();
        }
        used[sy][sx][sd][sy][sx][d] = true;
        if (dfs(sy, sx, sd, sy, sx, d))
        {
          ans = true;
          break;
        }
        d += 1;
      }
    }
    if (ans)
    {
      write("He can accomplish his mission.", "\n");
    } else
    {
      {
        var i: dynamic = 0;
        while ((i < 16))
        {
          {
            var j: dynamic = 0;
            while ((j < 64))
            {
              {
                var k: dynamic = 0;
                while ((k < 4))
                {
                  used2[i][j][k] = false;
                  k += 1;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      used2[sy][sx][sd] = true;
      if (dfs(sy, sx, sd))
      {
        write("He cannot return to the kitchen.", "\n");
      } else
      {
        write("He cannot bring tea to his master.", "\n");
      }
    }
  }
  return 0;
}
