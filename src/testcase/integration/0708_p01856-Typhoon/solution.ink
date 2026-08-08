// Translated from solution.cpp.

var D = cpp_array(523, 523);

var g = cpp_array(523, 523);

var cnt: dynamic;

var fx: dynamic;

var fy: dynamic;

func find(y: dynamic, x: dynamic)
{
  if (D[y][x])
  {
    fy = (y + 1);
    fx = (x + 1);
    cnt += 1;
    var p = D[y][x];
    g[fy][fx] = p;
    {
      var i = 0;
      while ((i < 3))
      {
        {
          var j = 0;
          while ((j < 3))
          {
            D[(y + i)][(x + j)] -= p;
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
}

func main()
{
  var H: dynamic;
  var W: dynamic;
  read(H, W);
  {
    var i = 0;
    while ((i < H))
    {
      {
        var j = 0;
        while ((j < W))
        {
          read(D[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < W))
    {
      var y = 0;
      var x = i;
      {
        while ((x >= 0))
        {
          find(y, x);
          y += 1;
          x -= 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i < H))
    {
      var y = i;
      var x = (W - 1);
      {
        while ((y < H))
        {
          find(y, x);
          y += 1;
          x -= 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic;
  if ((cnt == 1))
  {
    ans.emplace_back(fy, fx);
  } else
  {
    {
      var i = 0;
      while ((i < H))
      {
        {
          var j = 0;
          while ((j < W))
          {
            if (g[i][j])
            {
              var d = (g[i][j] - 1);
              {
                var k = -1;
                while ((k <= 1))
                {
                  var ny = (i + k);
                  if (((ny < 0) || (H <= ny)))
                  {
                    k += 1;
                    continue;
                  }
                  {
                    var l = -1;
                    while ((l <= 1))
                    {
                      var nx = (j + l);
                      if ((((nx < 0) || (W <= nx)) || ((k == 0) && (l == 0))))
                      {
                        l += 1;
                        continue;
                      }
                      d += g[ny][nx];
                      l += 1;
                    }
                  }
                  k += 1;
                }
              }
              if ((d % 2))
              {
                ans.emplace_back(i, j);
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
  }
  write(ans.back().first, cpp_char(" "), ans.back().second, cpp_char(" "), ans[0].first, cpp_char(" "), ans[0].second, "\n");
}
