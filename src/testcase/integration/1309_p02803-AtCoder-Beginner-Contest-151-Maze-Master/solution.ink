// Translated from solution.cpp.

func main() -> dynamic
{
  var h: dynamic = cpp_uninitialized();
  var w: dynamic = cpp_uninitialized();
  read(h, w);
  for (var s: dynamic in g)
  {
    read(s);
  }
  var dx: dynamic = [-1, 0, 0, 1];
  var dy: dynamic = [0, 1, -1, 0];
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < h))
    {
      {
        var j: dynamic = 0;
        while ((j < w))
        {
          if ((g[i][j] == cpp_char("#")))
          {
            j += 1;
            continue;
          }
          var cnt: dynamic = 0;
          var d: dynamic = cpp_construct(h, vector(w, -1));
          var q: dynamic = cpp_uninitialized();
          d[i][j] = 0;
          q.push(p);
          while ((!q.empty()))
          {
            var v: dynamic = q.front();
            q.pop();
            var y: dynamic = v.first;
            var x: dynamic = v.second;
            {
              var k: dynamic = 0;
              while ((k < 4))
              {
                var Y: dynamic = (y + dy[k]);
                var X: dynamic = (x + dx[k]);
                if ((((((0 <= Y) && (Y < h)) && (0 <= X)) && (X < w)) && (g[Y][X] == cpp_char("."))))
                {
                  if ((d[Y][X] != -1))
                  {
                    k += 1;
                    continue;
                  }
                  d[Y][X] = (d[y][x] + 1);
                  cnt = max(cnt, d[Y][X]);
                  q.push(V);
                }
                k += 1;
              }
            }
          }
          ans = max(ans, cnt);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
}
