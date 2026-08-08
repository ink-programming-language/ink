// Translated from solution.cpp.

func main()
{
  var h: dynamic;
  var w: dynamic;
  read(h, w);
  for (var s in g)
  {
    read(s);
  }
  var dx = [-1, 0, 0, 1];
  var dy = [0, 1, -1, 0];
  var ans = 0;
  {
    var i = 0;
    while ((i < h))
    {
      {
        var j = 0;
        while ((j < w))
        {
          if ((g[i][j] == cpp_char("#")))
          {
            j += 1;
            continue;
          }
          var cnt = 0;
          var d = cpp_construct(h, vector(w, -1));
          var q: dynamic;
          d[i][j] = 0;
          q.push(p);
          while ((!q.empty()))
          {
            var v = q.front();
            q.pop();
            var y = v.first;
            var x = v.second;
            {
              var k = 0;
              while ((k < 4))
              {
                var Y = (y + dy[k]);
                var X = (x + dx[k]);
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
