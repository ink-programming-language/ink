// Translated from solution.cpp.

var D: dynamic = cpp_array(523, 523);

var g: dynamic = cpp_array(523, 523);

var cnt: dynamic = cpp_uninitialized();

var fx: dynamic = cpp_uninitialized();

var fy: dynamic = cpp_uninitialized();

func find(y: dynamic, x: dynamic) -> dynamic
{
  if (D[y][x])
  {
    fy = (y + 1);
    fx = (x + 1);
    cnt += 1;
    var p: dynamic = D[y][x];
    g[fy][fx] = p;
    {
      var i: dynamic = 0;
      while ((i < 3))
      {
        {
          var j: dynamic = 0;
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

func main() -> dynamic
{
  var H: dynamic = cpp_uninitialized();
  var W: dynamic = cpp_uninitialized();
  read(H, W);
  {
    var i: dynamic = 0;
    while ((i < H))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < W))
    {
      var y: dynamic = 0;
      var x: dynamic = i;
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
    var i: dynamic = 1;
    while ((i < H))
    {
      var y: dynamic = i;
      var x: dynamic = (W - 1);
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
  var ans: dynamic = cpp_uninitialized();
  if ((cnt == 1))
  {
    ans.emplace_back(fy, fx);
  } else
  {
    {
      var i: dynamic = 0;
      while ((i < H))
      {
        {
          var j: dynamic = 0;
          while ((j < W))
          {
            if (g[i][j])
            {
              var d: dynamic = (g[i][j] - 1);
              {
                var k: dynamic = -1;
                while ((k <= 1))
                {
                  var ny: dynamic = (i + k);
                  if (((ny < 0) || (H <= ny)))
                  {
                    k += 1;
                    continue;
                  }
                  {
                    var l: dynamic = -1;
                    while ((l <= 1))
                    {
                      var nx: dynamic = (j + l);
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
