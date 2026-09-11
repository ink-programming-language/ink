// Translated from solution.cpp.

func ri() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%d", (&n));
  return n;
}

var d: dynamic = [[0, 1], [0, -1], [-1, 0], [1, 0]];

func main() -> dynamic
{
  var h: dynamic = ri();
  var w: dynamic = ri();
  var k: dynamic = ri();
  var a: dynamic = cpp_array(h);
  for (var i: dynamic in a)
  {
    read(i);
  }
  var gx: dynamic = cpp_uninitialized();
  var gy: dynamic = cpp_uninitialized();
  var sx: dynamic = cpp_uninitialized();
  var sy: dynamic = cpp_uninitialized();
  var apples: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          if ((a[i][j] == cpp_char("s")))
          {
            sx = i;
            sy = j;
          }
          if ((a[i][j] == cpp_char("e")))
          {
            gx = i;
            gy = j;
          }
          if ((a[i][j] == cpp_char("a")))
          {
            apples.push_back([i, j]);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  apples.push_back([gx, gy]);
  apples.push_back([sx, sy]);
  var m: dynamic = apples.size();
  var dist: dynamic = cpp_array(m, m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var distt: dynamic = cpp_array(w, h);
      {
        var j: dynamic = 0;
        while ((j < h))
        {
          {
            var k: dynamic = 0;
            while ((k < w))
            {
              distt[j][k] = 1000000000;
              k += 1;
            }
          }
          j += 1;
        }
      }
      var que: dynamic = cpp_uninitialized();
      que.push(apples[i]);
      distt[apples[i].first][apples[i].second] = 0;
      while (que.size())
      {
        var cur: dynamic = que.front();
        que.pop();
        for (var dd: dynamic in d)
        {
          var new_x: dynamic = (cur.first + dd.first);
          var new_y: dynamic = (cur.second + dd.second);
          if (((new_x < 0) || (new_x >= h)))
          {
            continue;
          }
          if (((new_y < 0) || (new_y >= w)))
          {
            continue;
          }
          if ((a[new_x][new_y] == cpp_char("#")))
          {
            continue;
          }
          if ((distt[new_x][new_y] > (distt[cur.first][cur.second] + 1)))
          {
            distt[new_x][new_y] = (distt[cur.first][cur.second] + 1);
            que.push([new_x, new_y]);
          }
        }
      }
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          dist[i][j] = distt[apples[j].first][apples[j].second];
          j += 1;
        }
      }
      i += 1;
    }
  }
  m -= 2;
  var dp: dynamic = cpp_construct((1 << m), vector(m, 1000000000));
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      dp[(1 << i)][i] = dist[i][(m + 1)];
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < (1 << m)))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((dp[i][j] == 1000000000))
          {
            j += 1;
            continue;
          }
          {
            var k: dynamic = 0;
            while ((k < m))
            {
              if (((i >> k) & 1))
              {
                k += 1;
                continue;
              }
              dp[(i | (1 << k))][k] = min(dp[(i | (1 << k))][k], (dp[i][j] + dist[j][k]));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var min: dynamic = 1000000000;
  {
    var i: dynamic = 0;
    while ((i < (1 << m)))
    {
      if ((builtin_popcount(i) < k))
      {
        i += 1;
        continue;
      }
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          min = min(min, (dp[i][j] + dist[j][m]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(( ((min == 1000000000)) ? -1 : min), "\n");
  return 0;
}
