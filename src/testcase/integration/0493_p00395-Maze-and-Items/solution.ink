// Translated from solution.cpp.

var H: dynamic = cpp_uninitialized();

var W: dynamic = cpp_uninitialized();

var sx: dynamic = cpp_array(32);

var sy: dynamic = cpp_array(32);

var s: dynamic = cpp_array(10, 10);

var idx: dynamic = cpp_array(1009, 1009);

var c: dynamic = cpp_array(1009, 1009);

var t: dynamic = cpp_array(32, 32);

var u: dynamic = cpp_array(1009, 1009);

var dp: dynamic = cpp_array(32, (1 << 10));

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func main() -> dynamic
{
  read(W, H);
  {
    var i: dynamic = 1;
    while ((i <= H))
    {
      {
        var j: dynamic = 1;
        while ((j <= W))
        {
          read(c[i][j]);
          idx[i][j] = -1;
          if ((c[i][j] == cpp_char("S")))
          {
            sx[30] = i;
            sy[30] = j;
          }
          if ((c[i][j] == cpp_char("T")))
          {
            sx[31] = i;
            sy[31] = j;
          }
          if (((c[i][j] >= cpp_char("0")) && (c[i][j] <= cpp_char("9"))))
          {
            sx[(0 + ((c[i][j] - cpp_char("0"))))] = i;
            sy[(0 + ((c[i][j] - cpp_char("0"))))] = j;
          }
          if (((c[i][j] >= cpp_char("A")) && (c[i][j] <= cpp_char("J"))))
          {
            sx[(10 + ((c[i][j] - cpp_char("A"))))] = i;
            sy[(10 + ((c[i][j] - cpp_char("A"))))] = j;
          }
          if (((c[i][j] >= cpp_char("a")) && (c[i][j] <= cpp_char("z"))))
          {
            sx[(20 + ((c[i][j] - cpp_char("a"))))] = i;
            sy[(20 + ((c[i][j] - cpp_char("a"))))] = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 10))
    {
      {
        var j: dynamic = 0;
        while ((j < 10))
        {
          read(s[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= 31))
    {
      idx[sx[i]][sy[i]] = i;
      {
        var j: dynamic = 0;
        while ((j <= 31))
        {
          t[i][j] = ((1 << 25));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i <= 31))
    {
      if (((sx[i] == 0) && (sy[i] == 0)))
      {
        i += 1;
        continue;
      }
      {
        var j: dynamic = 1;
        while ((j <= H))
        {
          {
            var k: dynamic = 1;
            while ((k <= W))
            {
              u[j][k] = ((1 << 25));
              k += 1;
            }
          }
          j += 1;
        }
      }
      var Q: dynamic = cpp_uninitialized();
      Q.push(make_pair(sx[i], sy[i]));
      u[sx[i]][sy[i]] = 0;
      while ((!Q.empty()))
      {
        var cx: dynamic = Q.front().first;
        var cy: dynamic = Q.front().second;
        Q.pop();
        {
          var j: dynamic = 0;
          while ((j < 4))
          {
            var ex: dynamic = (cx + dx[j]);
            var ey: dynamic = (cy + dy[j]);
            if ((((((ex <= 0) || (ey <= 0)) || (ex > H)) || (ey > W)) || (c[ex][ey] == cpp_char("#"))))
            {
              j += 1;
              continue;
            }
            if ((idx[ex][ey] == -1))
            {
              if ((u[ex][ey] == ((1 << 25))))
              {
                Q.push(make_pair(ex, ey));
                u[ex][ey] = (u[cx][cy] + 1);
              }
            } else
            {
              t[i][idx[ex][ey]] = min(t[i][idx[ex][ey]], (u[cx][cy] + 1));
            }
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < ((1 << 10))))
    {
      {
        var j: dynamic = 0;
        while ((j < 32))
        {
          dp[i][j] = make_pair(((1 << 25)), ((1 << 25)));
          j += 1;
        }
      }
      i += 1;
    }
  }
  dp[0][30] = make_pair(0, 0);
  var maxn: dynamic = make_pair(((1 << 25)), -1);
  {
    var i: dynamic = 0;
    while ((i < ((1 << 10))))
    {
      {
        var j: dynamic = 0;
        while ((j < 32))
        {
          if ((dp[i][j] == make_pair(((1 << 25)), ((1 << 25)))))
          {
            j += 1;
            continue;
          }
          var dist: dynamic = cpp_array(32);
          {
            var k: dynamic = 0;
            while ((k < 32))
            {
              dist[k] = ((1 << 25));
              k += 1;
            }
          }
          dist[j] = 0;
          var V: dynamic = cpp_uninitialized();
          {
            var k: dynamic = 0;
            while ((k < 10))
            {
              V.push_back(k);
              k += 1;
            }
          }
          {
            var k: dynamic = 0;
            while ((k < 10))
            {
              if (((((i / ((1 << k)))) % 2) == 0))
              {
                V.push_back((10 + k));
              }
              k += 1;
            }
          }
          {
            var k: dynamic = 0;
            while ((k < 10))
            {
              if (((((i / ((1 << k)))) % 2) == 1))
              {
                V.push_back((20 + k));
              }
              k += 1;
            }
          }
          V.push_back(30);
          V.push_back(31);
          {
            var tt: dynamic = 0;
            while ((tt < 22))
            {
              for (var k: dynamic in V)
              {
                for (var l: dynamic in V)
                {
                  if ((t[k][l] == (-((1 << 25)))))
                  {
                    continue;
                  }
                  dist[l] = min(dist[l], (dist[k] + t[k][l]));
                }
              }
              tt += 1;
            }
          }
          if ((i == 1023))
          {
            maxn = min(maxn, make_pair((dp[i][j].first + dist[31]), dp[i][j].second));
          }
          {
            var k: dynamic = 0;
            while ((k < 10))
            {
              if (((dist[k] == ((1 << 25))) || ((((i / ((1 << k)))) % 2) == 1)))
              {
                k += 1;
                continue;
              }
              dp[(i + ((1 << k)))][k] = min(dp[(i + ((1 << k)))][k], make_pair((dp[i][j].first + dist[k]), (dp[i][j].second - s[j][k])));
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  if ((maxn.first == ((1 << 25))))
  {
    write("-1", "\n");
  } else
  {
    write(maxn.first, " ", (-maxn.second), "\n");
  }
  return 0;
}
