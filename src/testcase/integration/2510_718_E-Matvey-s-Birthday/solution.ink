// Translated from solution.cpp.

var maxn = 112345;

var inf = 0x3f3f3f3f;

var n: dynamic;

var a = cpp_array(maxn);

var dist = cpp_array(8, maxn);

var d = cpp_array(8, 8);

var c = cpp_array((1 << 8), 8);

var mask = cpp_array(maxn);

var s = cpp_array(maxn);

func bfs(col: dynamic)
{
  var que: dynamic;
  dist[((n + col) + 1)][col] = 0;
  que.push(((n + col) + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      if ((col == a[i]))
      {
        dist[i][col] = 0;
        que.push(i);
      }
      i += 1;
    }
  }
  while ((!que.empty()))
  {
    var u = que.front();
    que.pop();
    if ((u <= n))
    {
      if (((u > 1) && (dist[(u - 1)][col] == inf)))
      {
        dist[(u - 1)][col] = (dist[u][col] + 1);
        que.push((u - 1));
        if ((dist[((n + a[(u - 1)]) + 1)][col] == inf))
        {
          dist[((n + a[(u - 1)]) + 1)][col] = (dist[u][col] + 1);
          que.push(((n + a[(u - 1)]) + 1));
        }
      }
      if (((u < n) && (dist[(u + 1)][col] == inf)))
      {
        dist[(u + 1)][col] = (dist[u][col] + 1);
        que.push((u + 1));
        if ((dist[((n + a[(u + 1)]) + 1)][col] == inf))
        {
          dist[((n + a[(u + 1)]) + 1)][col] = (dist[u][col] + 1);
          que.push(((n + a[(u + 1)]) + 1));
        }
      }
    } else
    {
      {
        var i = 1;
        while ((i <= n))
        {
          if (((a[i] == ((u - n) - 1)) && (dist[i][col] == inf)))
          {
            dist[i][col] = (dist[u][col] + 1);
            que.push(i);
          }
          i += 1;
        }
      }
    }
  }
}

func main()
{
  scanf("%d%s", (&n), (s + 1));
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = (s[i] - cpp_char("a"));
      i += 1;
    }
  }
  memset(dist, 0x3f, cpp_sizeof(dist));
  {
    var i = 0;
    while ((i < 8))
    {
      bfs(i);
      {
        var j = 0;
        while ((j < 8))
        {
          d[j][i] = dist[((n + j) + 1)][i];
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j < 8))
        {
          if ((d[a[i]][j] != dist[i][j]))
          {
            mask[i] |= (1 << j);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var mx = 0;
  var res = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = max(1, (i - 15));
        while ((j <= i))
        {
          var tmp = (i - j);
          {
            var k = 0;
            while ((k < 8))
            {
              tmp = min(tmp, ((dist[i][k] + dist[j][k]) + 1));
              k += 1;
            }
          }
          if ((tmp > mx))
          {
            mx = tmp;
            res = 1;
          } else if ((tmp == mx))
          {
            res += 1;
          }
          j += 1;
        }
      }
      var j = (i - 16);
      if ((j >= 1))
      {
        c[a[j]][mask[j]] += 1;
      }
      {
        var j = 0;
        while ((j < 8))
        {
          {
            var k = 0;
            while ((k < 256))
            {
              if (c[j][k])
              {
                var tmp = inf;
                {
                  var l = 0;
                  while ((l < 8))
                  {
                    tmp = min(tmp, (((dist[i][l] + d[j][l]) + (((k >> l) & 1))) + 1));
                    l += 1;
                  }
                }
                if ((tmp > mx))
                {
                  mx = tmp;
                  res = c[j][k];
                } else if ((tmp == mx))
                {
                  res += c[j][k];
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
  return (0 * printf("%d %I64d\n", mx, res));
}
