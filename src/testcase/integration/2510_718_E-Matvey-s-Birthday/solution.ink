// Translated from solution.cpp.

var maxn: dynamic = 112345;

var inf: dynamic = 0x3f3f3f3f;

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(maxn);

var dist: dynamic = cpp_array(8, maxn);

var d: dynamic = cpp_array(8, 8);

var c: dynamic = cpp_array((1 << 8), 8);

var mask: dynamic = cpp_array(maxn);

var s: dynamic = cpp_array(maxn);

func bfs(col: dynamic) -> dynamic
{
  var que: dynamic = cpp_uninitialized();
  dist[((n + col) + 1)][col] = 0;
  que.push(((n + col) + 1));
  {
    var i: dynamic = 1;
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
    var u: dynamic = que.front();
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
        var i: dynamic = 1;
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

func main() -> dynamic
{
  scanf("%d%s", (&n), (s + 1));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = (s[i] - cpp_char("a"));
      i += 1;
    }
  }
  memset(dist, 0x3f, cpp_sizeof(dist));
  {
    var i: dynamic = 0;
    while ((i < 8))
    {
      bfs(i);
      {
        var j: dynamic = 0;
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
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = 0;
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
  var mx: dynamic = 0;
  var res: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = max(1, (i - 15));
        while ((j <= i))
        {
          var tmp: dynamic = (i - j);
          {
            var k: dynamic = 0;
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
      var j: dynamic = (i - 16);
      if ((j >= 1))
      {
        c[a[j]][mask[j]] += 1;
      }
      {
        var j: dynamic = 0;
        while ((j < 8))
        {
          {
            var k: dynamic = 0;
            while ((k < 256))
            {
              if (c[j][k])
              {
                var tmp: dynamic = inf;
                {
                  var l: dynamic = 0;
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
