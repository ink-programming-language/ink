// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var g: dynamic = cpp_uninitialized();
  while (cpp_comma(((cin >> n) >> m), (n || m)))
  {
    read(s, g);
    s -= 1;
    g -= 1;
    var d: dynamic = cpp_array(n, n);
    var c: dynamic = cpp_array(n, n);
    memset(d, -1, cpp_sizeof((d)));
    memset(c, -1, cpp_sizeof((c)));
    var i: dynamic = cpp_uninitialized();
    var j: dynamic = cpp_uninitialized();
    var k: dynamic = cpp_uninitialized();
    var l: dynamic = cpp_uninitialized();
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    {
      i = 0;
      while ((i < m))
      {
        read(x, y, j, k);
        x -= 1;
        y -= 1;
        d[x][y] = cpp_assign(d[y][x], "=", j);
        c[x][y] = cpp_assign(c[y][x], "=", k);
        i += 1;
      }
    }
    var v: dynamic = cpp_array(n, 50, n);
    var inf: dynamic = (1 << 28);
    var p: dynamic = cpp_uninitialized();
    var ans: dynamic = inf;
    {
      i = 0;
      while ((i < n))
      {
        {
          j = 0;
          while ((j < 50))
          {
            {
              k = 0;
              while ((k < n))
              {
                v[i][j][k] = inf;
                k += 1;
              }
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var q: dynamic = cpp_uninitialized();
    q.push(PP(0, P(s, PPP(1, -1))));
    while ((!q.empty()))
    {
      x = q.top().second.first;
      y = q.top().second.second.first;
      k = q.top().second.second.second;
      p = q.top().first;
      q.pop();
      if ((v[x][y][k] <= p))
      {
        continue;
      }
      v[x][y][k] = p;
      if (((x == g) && (y == 1)))
      {
        break;
      }
      {
        i = 0;
        while ((i < n))
        {
          if (((!(~d[x][i])) || (i == k)))
          {
            i += 1;
            continue;
          }
          {
            j = -1;
            while ((j <= 1))
            {
              if (((k == -1) && (j != 0)))
              {
                j += 1;
                continue;
              }
              if ((((y + j) <= 0) || (c[x][i] < (y + j))))
              {
                j += 1;
                continue;
              }
              q.push(PP((p + (cpp_cast(d[x][i]) / ((y + j)))), P(i, PPP((y + j), x))));
              j += 1;
            }
          }
          i += 1;
        }
      }
    }
    {
      i = 0;
      while ((i < n))
      {
        ans = min(ans, v[g][1][i]);
        i += 1;
      }
    }
    if ((ans != inf))
    {
      printf("%.8f\n", ans);
    } else
    {
      printf("unreachable\n");
    }
  }
  return 0;
}
