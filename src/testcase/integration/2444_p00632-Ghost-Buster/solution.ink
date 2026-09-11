// Translated from solution.cpp.

var ff: dynamic = cpp_expression("#include<io");

var fs: dynamic = cpp_expression("#include<ios");

var h: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var sx: dynamic = cpp_uninitialized();

var sy: dynamic = cpp_uninitialized();

var gx: dynamic = cpp_uninitialized();

var gy: dynamic = cpp_uninitialized();

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, -1, 0, 1];

var co: dynamic = cpp_array(21, 21);

var ma: dynamic = cpp_array(21, 21);

var visited: dynamic = cpp_array(21, 21);

var pat: dynamic = cpp_uninitialized();

var m: dynamic = 999999;

func main() -> dynamic
{
  while (cpp_comma(((cin >> h) >> w), h))
  {
    {
      var i: dynamic = 0;
      while ((i < h))
      {
        {
          var j: dynamic = 0;
          while ((j < w))
          {
            co[i][j] = m;
            read(ma[i][j]);
            if ((ma[i][j] == cpp_char("A")))
            {
              sx = j;
              sy = i;
            } else if ((ma[i][j] == cpp_char("B")))
            {
              gx = j;
              gy = i;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    var q: dynamic = cpp_uninitialized();
    q.push(make_pair(make_pair(sx, sy), 0));
    while (q.size())
    {
      var p: dynamic = q.front();
      q.pop();
      if ((co[p.fs][p.ff] != m))
      {
        continue;
      }
      co[p.fs][p.ff] = p.second;
      {
        var i: dynamic = 0;
        while ((i < 4))
        {
          var nx: dynamic = (p.ff + dx[i]);
          var ny: dynamic = (p.fs + dy[i]);
          if ((((((nx >= 0) && (nx < w)) && ((ny >= 0) & (ny < h))) && (ma[ny][nx] != cpp_char("#"))) && (co[ny][nx] == m)))
          {
            q.push(make_pair(make_pair(nx, ny), (p.second + 1)));
          }
          i += 1;
        }
      }
    }
    read(pat);
    {
      var j: dynamic = 0;
      var go: dynamic = 0;
      while (true)
      {
        var i: dynamic = (j % pat.length());
        if ((co[gy][gx] <= go))
        {
          write(go, " ", gy, " ", gx, "\n");
          break;
        }
        if ((go > 1000))
        {
          write("impossible", "\n");
          break;
        }
        if ((pat[i] == cpp_char("8")))
        {
          gy -= 1;
        } else if ((pat[i] == cpp_char("6")))
        {
          gx += 1;
        } else if ((pat[i] == cpp_char("4")))
        {
          gx -= 1;
        } else if ((pat[i] == cpp_char("2")))
        {
          gy += 1;
        }
        gy = max(0, min((h - 1), gy));
        gx = max(0, min((w - 1), gx));
        j += 1;
        go += 1;
      }
    }
  }
}
