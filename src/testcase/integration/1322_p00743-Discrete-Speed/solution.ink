// Translated from solution.cpp.

func main()
{
  var n: dynamic;
  var m: dynamic;
  var s: dynamic;
  var g: dynamic;
  while (cpp_comma(((cin >> n) >> m), (n || m)))
  {
    read(s, g);
    s -= 1;
    g -= 1;
    var d = cpp_array(n, n);
    var c = cpp_array(n, n);
    memset(d, -1, cpp_sizeof((d)));
    memset(c, -1, cpp_sizeof((c)));
    var i: dynamic;
    var j: dynamic;
    var k: dynamic;
    var l: dynamic;
    var x: dynamic;
    var y: dynamic;
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
    var v = cpp_array(n, 50, n);
    var inf = (1 << 28);
    var p: dynamic;
    var ans = inf;
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
    var q: dynamic;
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
