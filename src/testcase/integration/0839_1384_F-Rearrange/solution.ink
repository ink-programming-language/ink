// Translated from solution.cpp.

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          read(mat[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var h: dynamic = cpp_construct(((n * m) + 1));
  var v: dynamic = cpp_construct(((n * m) + 1));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var a: dynamic = 0;
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < m))
    {
      var a: dynamic = 0;
      {
        var j: dynamic = 0;
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
  var q: dynamic = cpp_uninitialized();
  var x: dynamic = -1;
  var y: dynamic = -1;
  {
    var u: dynamic = (n * m);
    while ((u >= 1))
    {
      x += h[u];
      y += v[u];
      if ((h[u] || v[u]))
      {
        fin[x][y] = u;
      } else
      {
        var qx: dynamic = cpp_uninitialized();
        var qy: dynamic = cpp_uninitialized();
        tie(qx, qy) = q.front();
        q.pop();
        fin[qx][qy] = u;
      }
      if (h[u])
      {
        {
          var i: dynamic = (y - 1);
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
          var i: dynamic = (x - 1);
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
