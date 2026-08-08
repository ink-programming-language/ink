// Translated from solution.cpp.

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic;
  var m: dynamic;
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          read(mat[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var h = cpp_construct(((n * m) + 1));
  var v = cpp_construct(((n * m) + 1));
  {
    var i = 0;
    while ((i < n))
    {
      var a = 0;
      {
        var j = 0;
        while ((j < m))
        {
          a = max(a, mat[i][j]);
          j += 1;
        }
      }
      h[a] = 1;
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < m))
    {
      var a = 0;
      {
        var j = 0;
        while ((j < n))
        {
          a = max(a, mat[j][i]);
          j += 1;
        }
      }
      v[a] = 1;
      i += 1;
    }
  }
  var q: dynamic;
  var x = -1;
  var y = -1;
  {
    var u = (n * m);
    while ((u >= 1))
    {
      x += h[u];
      y += v[u];
      if ((h[u] || v[u]))
      {
        fin[x][y] = u;
      } else
      {
        var qx: dynamic;
        var qy: dynamic;
        tie(qx, qy) = q.front();
        q.pop();
        fin[qx][qy] = u;
      }
      if (h[u])
      {
        {
          var i = (y - 1);
          while ((i >= 0))
          {
            q.push([x, i]);
            i -= 1;
          }
        }
      }
      if (v[u])
      {
        {
          var i = (x - 1);
          while ((i >= 0))
          {
            q.push([i, y]);
            i -= 1;
          }
        }
      }
      u -= 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      {
        var j = 0;
        while ((j < m))
        {
          write(fin[i][j], " \n"[((j + 1) == m)]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
