// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var dx = [1, 0, -1, 0];

var dy = [0, 1, 0, -1];

func main()
{
  {
    var W: dynamic;
    var H: dynamic;
    var n: dynamic;
    while (cpp_comma(scanf("%d%d%d", (&W), (&H), (&n)), W))
    {
      var wallH = [];
      var wallV = [];
      rep(x, W)[0][x] = cpp_assign(wallH[H][x], "=", true);
      rep(y, H)[y][0] = cpp_assign(wallV[y][W], "=", true);
      var xs: dynamic;
      var ys: dynamic;
      var dir: dynamic;
      {
        var xa: dynamic;
        var ya: dynamic;
        var xb: dynamic;
        var yb: dynamic;
        scanf("%d%d%d%d", (&xa), (&ya), (&xb), (&yb));
        xs = min(xa, xb);
        ys = min(ya, yb);
        if ((ya == yb))
        {
          xs = min(xa, xb);
          if ((ya == 0))
          {
            ys = 0;
            dir = 1;
          } else
          {
            ys = (H - 1);
            dir = 3;
          }
        } else
        {
          ys = min(ya, yb);
          if ((xa == 0))
          {
            xs = 0;
            dir = 0;
          } else
          {
            xs = (W - 1);
            dir = 2;
          }
        }
      }
      var xg: dynamic;
      var yg: dynamic;
      scanf("%d%d", (&xg), (&yg));
      var ok = false;
      var x = xs;
      var y = ys;
      var cnt = 1;
      var visited = [];
      while (1)
      {
        if (((x == xg) && (y == yg)))
        {
          ok = true;
          break;
        }
        if (visited[y][x][dir])
        {
          break;
        }
        visited[y][x][dir] = true;
        dir = (((dir + 1)) % 4);
        rep(i, 4);
        {
          var xx = (x + dx[dir]);
          var yy = (y + dy[dir]);
          if (((((((dir == 0) && (!wallV[y][xx]))) || (((dir == 1) && (!wallH[yy][x])))) || (((dir == 2) && (!wallV[y][x])))) || (((dir == 3) && (!wallH[y][x])))))
          {
            x = xx;
            y = yy;
            break;
          }
          dir = (((dir + 3)) % 4);
        }
        cnt += 1;
      }
      if (ok)
      {
        printf("%d\n", cnt);
      } else
      {
        puts("Impossible");
      }
    }
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        var xa: dynamic;
        var ya: dynamic;
        var xb: dynamic;
        var yb: dynamic;
        scanf("%d%d%d%d", (&xa), (&ya), (&xb), (&yb));
        if ((xb < xa))
        {
          swap(xa, xb);
        }
        if ((yb < ya))
        {
          swap(ya, yb);
        }
        if ((ya == yb))
        {
          {
            var x = xa;
            while ((x < xb))
            {
              wallH[ya][x] = true;
              x += 1;
            }
          }
        } else
        {
          {
            var y = ya;
            while ((y < yb))
            {
              wallV[y][xa] = true;
              y += 1;
            }
          }
        }
      }
