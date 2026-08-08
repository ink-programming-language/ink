// Translated from solution.cpp.

var matrix = cpp_array(1000, 1000);

var tmp = cpp_array(1000, 1000);

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

var all: dynamic;

func go(x: dynamic, y: dynamic)
{
  var q: dynamic;
  tmp[x][y] = 1;
  q.push(make_pair(x, y));
  all.push(make_pair(x, y));
  while ((!q.empty()))
  {
    x = q.front().first;
    y = q.front().second;
    q.pop();
    {
      var i = 0;
      while ((i < 4))
      {
        if ((((((x + dx[i]) >= 0) && ((x + dx[i]) <= 999)) && ((y + dy[i]) >= 0)) && ((y + dy[i]) <= 999)))
        {
          if (((matrix[(x + dx[i])][(y + dy[i])] != 255) && (tmp[(x + dx[i])][(y + dy[i])] == 0)))
          {
            tmp[(x + dx[i])][(y + dy[i])] = 1;
            q.push(make_pair((x + dx[i]), (y + dy[i])));
            all.push(make_pair((x + dx[i]), (y + dy[i])));
          }
        }
        i += 1;
      }
    }
  }
}

var path = cpp_array(10000);

var cnt = 0;

func bfs(x1: dynamic, y1: dynamic, x2: dynamic, y2: dynamic)
{
  {
    var i = 0;
    var maxi = cpp_cast((all).size());
    while ((i < maxi))
    {
      var x = all.front().first;
      var y = all.front().second;
      all.pop();
      tmp[x][y] = 0;
      i += 1;
    }
  }
  var q: dynamic;
  q.push(make_pair(x1, y1));
  tmp[x1][y1] = 111;
  all.push(make_pair(x1, y1));
  while ((!q.empty()))
  {
    {
      var i = 0;
      var maxi = cpp_cast((q).size());
      while ((i < maxi))
      {
        var x = q.front().first;
        var y = q.front().second;
        q.pop();
        if (((x == x2) && (y == y2)))
        {
          cnt = 0;
          while (((x != x1) || (y != y1)))
          {
            var j = (tmp[x][y] - 1);
            path[cpp_update(cnt, "++")] = j;
            x = (x - dx[j]);
            y = (y - dy[j]);
          }
          return;
        }
        {
          var j = 0;
          while ((j < 4))
          {
            if ((((((x + dx[j]) >= 0) && ((x + dx[j]) <= 999)) && ((y + dy[j]) >= 0)) && ((y + dy[j]) <= 999)))
            {
              if (((matrix[(x + dx[j])][(y + dy[j])] != 255) && (tmp[(x + dx[j])][(y + dy[j])] == 0)))
              {
                tmp[(x + dx[j])][(y + dy[j])] = (j + 1);
                q.push(make_pair((x + dx[j]), (y + dy[j])));
                all.push(make_pair((x + dx[j]), (y + dy[j])));
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}

var ans = "";

func main()
{
  var x1: dynamic;
  var y1: dynamic;
  var x2: dynamic;
  var y2: dynamic;
  scanf("%d %d %d %d", (&x1), (&y1), (&x2), (&y2));
  x1 += 500;
  y1 += 500;
  x2 += 500;
  y2 += 500;
  var m: dynamic;
  scanf("%d", (&m));
  var max_x = 0;
  var min_x = 1000;
  var max_y = 0;
  var min_y = 1000;
  {
    var i = 0;
    while ((i < m))
    {
      var x: dynamic;
      var y: dynamic;
      scanf("%d %d", (&x), (&y));
      x += 500;
      y += 500;
      min_x = min(min_x, x);
      max_x = max(max_x, x);
      min_y = min(min_y, y);
      max_y = max(max_y, y);
      matrix[x][y] = 255;
      i += 1;
    }
  }
  go(x1, y1);
  if (((m == 0) || (tmp[x1][y1] != tmp[x2][y2])))
  {
    printf("-1");
    return 0;
  }
  while (true)
  {
    if (((x1 == x2) && (y1 == y2)))
    {
      write(ans);
      return 0;
    }
    var flag = false;
    if ((min(y1, y2) > max_y))
    {
      while ((max(x1, x2) >= min_x))
      {
        x1 -= 1;
        x2 -= 1;
        ans += "L";
      }
      while ((max(y1, y2) >= min_y))
      {
        y1 -= 1;
        y2 -= 1;
        ans += "D";
      }
      flag = true;
    } else if ((max(y1, y2) < min_y))
    {
      while ((max(x1, x2) >= min_x))
      {
        x1 -= 1;
        x2 -= 1;
        ans += "L";
      }
      while ((max(y1, y2) >= min_y))
      {
        y1 -= 1;
        y2 -= 1;
        ans += "D";
      }
      flag = true;
    } else if ((min(x1, x2) > max_x))
    {
      while ((max(y1, y2) >= min_y))
      {
        y1 -= 1;
        y2 -= 1;
        ans += "D";
      }
      while ((max(x1, x2) >= min_x))
      {
        x1 -= 1;
        x2 -= 1;
        ans += "L";
      }
      flag = true;
    } else if ((max(x1, x2) < min_x))
    {
      while ((max(y1, y2) >= min_y))
      {
        y1 -= 1;
        y2 -= 1;
        ans += "D";
      }
      while ((max(x1, x2) >= min_x))
      {
        x1 -= 1;
        x2 -= 1;
        ans += "L";
      }
      flag = true;
    }
    if (flag)
    {
      if ((y2 > y1))
      {
        var y = min_y;
        var x: dynamic;
        {
          var i = 300;
          while ((i < 700))
          {
            if ((matrix[i][y] == 255))
            {
              x = i;
              break;
            }
            i += 1;
          }
        }
        while ((x2 < x))
        {
          x1 += 1;
          x2 += 1;
          ans += "R";
        }
        while ((y2 != y1))
        {
          y1 += 1;
          if ((matrix[x2][(y2 + 1)] != 255))
          {
            y2 += 1;
          }
          ans += "U";
        }
      } else
      {
        while ((min(y1, y2) <= max_y))
        {
          y1 += 1;
          y2 += 1;
          ans += "U";
        }
        var y = max_y;
        var x: dynamic;
        {
          var i = 300;
          while ((i < 700))
          {
            if ((matrix[i][y] == 255))
            {
              x = i;
              break;
            }
            i += 1;
          }
        }
        while ((x2 < x))
        {
          x1 += 1;
          x2 += 1;
          ans += "R";
        }
        while ((y2 != y1))
        {
          y1 -= 1;
          if ((matrix[x2][(y2 - 1)] != 255))
          {
            y2 -= 1;
          }
          ans += "D";
        }
      }
      if ((x2 > x1))
      {
        while ((max(x1, x2) >= min_x))
        {
          x2 -= 1;
          x1 -= 1;
          ans += "L";
        }
        while ((max(y1, y2) >= min_y))
        {
          y2 -= 1;
          y1 -= 1;
          ans += "D";
        }
        var x = min_x;
        var y: dynamic;
        {
          var i = 300;
          while ((i < 700))
          {
            if ((matrix[x][i] == 255))
            {
              y = i;
              break;
            }
            i += 1;
          }
        }
        while ((y1 < y))
        {
          y1 += 1;
          y2 += 1;
          ans += "U";
        }
        while ((x2 != x1))
        {
          x1 += 1;
          if ((matrix[(x2 + 1)][y2] != 255))
          {
            x2 += 1;
          }
          ans += "R";
        }
        write(ans);
        return 0;
      } else
      {
        while ((min(x1, x2) <= max_x))
        {
          x1 += 1;
          x2 += 1;
          ans += "R";
        }
        while ((max(y1, y2) >= min_y))
        {
          y2 -= 1;
          y1 -= 1;
          ans += "D";
        }
        var x = max_x;
        var y: dynamic;
        {
          var i = 300;
          while ((i < 700))
          {
            if ((matrix[x][i] == 255))
            {
              y = i;
              break;
            }
            i += 1;
          }
        }
        while ((y1 < y))
        {
          y1 += 1;
          y2 += 1;
          ans += "U";
        }
        while ((x2 != x1))
        {
          x1 -= 1;
          if ((matrix[(x2 - 1)][y2] != 255))
          {
            x2 -= 1;
          }
          ans += "L";
        }
        write(ans);
        return 0;
      }
    }
    bfs(x1, y1, x2, y2);
    {
      var i = (cnt - 1);
      while ((i >= 0))
      {
        if ((path[i] == 0))
        {
          ans += "R";
        } else if ((path[i] == 1))
        {
          ans += "U";
        } else if ((path[i] == 2))
        {
          ans += "L";
        } else if ((path[i] == 3))
        {
          ans += "D";
        }
        var j = path[i];
        x1 += dx[j];
        y1 += dy[j];
        if ((matrix[(x2 + dx[j])][(y2 + dy[j])] != 255))
        {
          x2 += dx[j];
          y2 += dy[j];
        }
        i -= 1;
      }
    }
  }
  return 0;
}
