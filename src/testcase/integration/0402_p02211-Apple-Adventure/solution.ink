// Translated from solution.cpp.

func ri()
{
  var n: dynamic;
  scanf("%d", (&n));
  return n;
}

var d = [[0, 1], [0, -1], [-1, 0], [1, 0]];

func main()
{
  var h = ri();
  var w = ri();
  var k = ri();
  var a = cpp_array(h);
  for (var i in a)
  {
    read(i);
  }
  var gx: dynamic;
  var gy: dynamic;
  var sx: dynamic;
  var sy: dynamic;
  var apples: dynamic;
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
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
  var m = apples.size();
  var dist = cpp_array(m, m);
  {
    var i = 0;
    while ((i < m))
    {
      var distt = cpp_array(w, h);
      {
        var j = 0;
        while ((j < h))
        {
          {
            var k = 0;
            while ((k < w))
            {
              distt[j][k] = 1000000000;
              k += 1;
            }
          }
          j += 1;
        }
      }
      var que: dynamic;
      que.push(apples[i]);
      distt[apples[i].first][apples[i].second] = 0;
      while (que.size())
      {
        var cur = que.front();
        que.pop();
        for (var dd in d)
        {
          var new_x = (cur.first + dd.first);
          var new_y = (cur.second + dd.second);
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
        var j = 0;
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
  var dp = cpp_construct((1 << m), vector(m, 1000000000));
  {
    var i = 0;
    while ((i < m))
    {
      dp[(1 << i)][i] = dist[i][(m + 1)];
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < (1 << m)))
    {
      {
        var j = 0;
        while ((j < m))
        {
          if ((dp[i][j] == 1000000000))
          {
            j += 1;
            continue;
          }
          {
            var k = 0;
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
  var min = 1000000000;
  {
    var i = 0;
    while ((i < (1 << m)))
    {
      if ((builtin_popcount(i) < k))
      {
        i += 1;
        continue;
      }
      {
        var j = 0;
        while ((j < m))
        {
          min = min(min, (dp[i][j] + dist[j][m]));
          j += 1;
        }
      }
      i += 1;
    }
  }
  write((if ((min == 1000000000)) -1 else min), "\n");
  return 0;
}
