// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, 1, 0, -1];

func main() -> dynamic
{
  {
    var W: dynamic = cpp_uninitialized();
    var H: dynamic = cpp_uninitialized();
    var n: dynamic = cpp_uninitialized();
    while (cpp_comma(scanf("%d%d%d", (&W), (&H), (&n)), W))
    {
      var wallH: dynamic = [];
      var wallV: dynamic = [];
      rep(x, W)[0][x] = cpp_assign(wallH[H][x], "=", true);
      rep(y, H)[y][0] = cpp_assign(wallV[y][W], "=", true);
      var xs: dynamic = cpp_uninitialized();
      var ys: dynamic = cpp_uninitialized();
      var dir: dynamic = cpp_uninitialized();
      {
        var xa: dynamic = cpp_uninitialized();
        var ya: dynamic = cpp_uninitialized();
        var xb: dynamic = cpp_uninitialized();
        var yb: dynamic = cpp_uninitialized();
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
      var xg: dynamic = cpp_uninitialized();
      var yg: dynamic = cpp_uninitialized();
      scanf("%d%d", (&xg), (&yg));
      var ok: dynamic = false;
      var x: dynamic = xs;
      var y: dynamic = ys;
      var cnt: dynamic = 1;
      var visited: dynamic = [];
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
          var xx: dynamic = (x + dx[dir]);
          var yy: dynamic = (y + dy[dir]);
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        var xa: dynamic = cpp_uninitialized();
        var ya: dynamic = cpp_uninitialized();
        var xb: dynamic = cpp_uninitialized();
        var yb: dynamic = cpp_uninitialized();
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
            var x: dynamic = xa;
            while ((x < xb))
            {
              wallH[ya][x] = true;
              x += 1;
            }
          }
        } else
        {
          {
            var y: dynamic = ya;
            while ((y < yb))
            {
              wallV[y][xa] = true;
              y += 1;
            }
          }
        }
      }
