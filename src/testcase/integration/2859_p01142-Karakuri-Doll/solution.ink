// Translated from solution.cpp.

var W: dynamic;

var H: dynamic;

var sy: dynamic;

var sx: dynamic;

var sd: dynamic;

var gy: dynamic;

var gx: dynamic;

var gd: dynamic;

var field = cpp_array(64, 16);

var used = cpp_array(4, 64, 16, 4, 64, 16);

var used2 = cpp_array(4, 64, 16);

var dx = [0, 1, 0, -1];

var dy = [-1, 0, 1, 0];

func init()
{
  {
    var i = 0;
    while ((i < 16))
    {
      {
        var j = 0;
        while ((j < 64))
        {
          {
            var k = 0;
            while ((k < 4))
            {
              {
                var l = 0;
                while ((l < 16))
                {
                  {
                    var m = 0;
                    while ((m < 64))
                    {
                      {
                        var n = 0;
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

func front(y: dynamic, x: dynamic, d: dynamic)
{
  if ((d == 0))
  {
    var Y = y;
    {
      while (field[(Y - 1)][x])
      {
        Y -= 1;
      }
    }
    return [Y, x];
  } else if ((d == 1))
  {
    var X = x;
    {
      while (field[y][(X + 1)])
      {
        X += 1;
      }
    }
    return [y, X];
  } else if ((d == 2))
  {
    var Y = y;
    {
      while (field[(Y + 1)][x])
      {
        Y += 1;
      }
    }
    return [Y, x];
  } else
  {
    var X = x;
    {
      while (field[y][(X - 1)])
      {
        X -= 1;
      }
    }
    return [y, X];
  }
}

func back(y: dynamic, x: dynamic, d: dynamic)
{
  var ans: dynamic;
  var R = 1;
  var L = 3;
  if ((d == 0))
  {
    var Y = y;
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
    var X = x;
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
    var Y = y;
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
    var X = x;
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

func dfs(fy: dynamic, fx: dynamic, fd: dynamic, by: dynamic, bx: dynamic, bd: dynamic)
{
  tie(fy, fx) = front(fy, fx, fd);
  var cand = back(by, bx, bd);
  for (var next in cand)
  {
    var div = next.second;
    by = next.first.first;
    bx = next.first.second;
    var newfd = (((fd + div)) % 4);
    var newbd = (((bd + div)) % 4);
    if (cpp_binary(cpp_binary(cpp_binary(cpp_binary((fy == gy), "and", (fx == gx)), "and", (by == gy)), "and", (bx == gx)), "and", (bd == gd)))
    {
      return true;
    }
    if (cpp_unary("not", used[fy][fx][newfd][by][bx][newbd]))
    {
      used[fy][fx][newfd][by][bx][newbd] = true;
      var sub = dfs(fy, fx, newfd, by, bx, newbd);
      if (sub)
      {
        return true;
      }
    }
  }
  return false;
}

func dfs(fy: dynamic, fx: dynamic, fd: dynamic)
{
  if (cpp_binary((fy == gy), "and", (fx == gx)))
  {
    return true;
  }
  tie(fy, fx) = front(fy, fx, fd);
  {
    var i = -1;
    while ((i <= 1))
    {
      var nd = ((((fd + i) + 4)) % 4);
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

func main()
{
  var cnt = 0;
  while (true)
  {
    cnt += 1;
    read(W, H);
    if (cpp_binary((W == 0), "and", (H == 0)))
    {
      break;
    }
    {
      var i = 0;
      while ((i < H))
      {
        {
          var j = 0;
          while ((j < W))
          {
            var c: dynamic;
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
      var i = 0;
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
      var i = 0;
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
    var ans = false;
    {
      var d = 0;
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
        var i = 0;
        while ((i < 16))
        {
          {
            var j = 0;
            while ((j < 64))
            {
              {
                var k = 0;
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
