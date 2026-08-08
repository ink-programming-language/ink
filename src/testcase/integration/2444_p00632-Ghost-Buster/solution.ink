// Translated from solution.cpp.

var ff = cpp_expression("#include<io");

var fs = cpp_expression("#include<ios");

var h: dynamic;

var w: dynamic;

var sx: dynamic;

var sy: dynamic;

var gx: dynamic;

var gy: dynamic;

var dx = [1, 0, -1, 0];

var dy = [0, -1, 0, 1];

var co = cpp_array(21, 21);

var ma = cpp_array(21, 21);

var visited = cpp_array(21, 21);

var pat: dynamic;

var m = 999999;

func main()
{
  while (cpp_comma(((cin >> h) >> w), h))
  {
    {
      var i = 0;
      while ((i < h))
      {
        {
          var j = 0;
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
    var q: dynamic;
    q.push(make_pair(make_pair(sx, sy), 0));
    while (q.size())
    {
      var p = q.front();
      q.pop();
      if ((co[p.fs][p.ff] != m))
      {
        continue;
      }
      co[p.fs][p.ff] = p.second;
      {
        var i = 0;
        while ((i < 4))
        {
          var nx = (p.ff + dx[i]);
          var ny = (p.fs + dy[i]);
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
      var j = 0;
      var go = 0;
      while (true)
      {
        var i = (j % pat.length());
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
