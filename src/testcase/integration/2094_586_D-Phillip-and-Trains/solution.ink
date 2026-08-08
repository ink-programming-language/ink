// Translated from solution.cpp.

var n: dynamic;

var k: dynamic;

var t: dynamic;

var a = cpp_array(200, 3);

var vis = cpp_array(200, 3);

func check(x: dynamic, y: dynamic)
{
  return (((((x >= 0) && (y >= 0)) && (x < 3)) && (y < 200)));
}

func solve(cx: dynamic, cy: dynamic)
{
  if (vis[cx][cy])
  {
    return;
  }
  vis[cx][cy] = true;
  if (((check(cx, (cy + 3)) && (a[cx][(cy + 1)] == cpp_char("."))) && (a[cx][(cy + 2)] == cpp_char("."))))
  {
    solve(cx, (cy + 3));
  }
  if ((((check((cx + 1), (cy + 3)) && (a[cx][(cy + 1)] == cpp_char("."))) && (a[(cx + 1)][(cy + 1)] == cpp_char("."))) && (a[(cx + 1)][(cy + 2)] == cpp_char("."))))
  {
    solve((cx + 1), (cy + 3));
  }
  if ((((check((cx - 1), (cy + 3)) && (a[cx][(cy + 1)] == cpp_char("."))) && (a[(cx - 1)][(cy + 1)] == cpp_char("."))) && (a[(cx - 1)][(cy + 2)] == cpp_char("."))))
  {
    solve((cx - 1), (cy + 3));
  }
}

func main()
{
  read(t);
  while (cpp_update(t, "--"))
  {
    memset(vis, 0, cpp_sizeof((vis)));
    {
      var i = 0;
      while ((i < 3))
      {
        {
          var j = 0;
          while ((j < 200))
          {
            a[i][j] = cpp_char(".");
            j += 1;
          }
        }
        i += 1;
      }
    }
    read(n, k);
    var x: dynamic;
    var y: dynamic;
    {
      var i = 0;
      while ((i < 3))
      {
        {
          var j = 0;
          while ((j < n))
          {
            read(a[i][j]);
            if ((a[i][j] == cpp_char("s")))
            {
              x = i;
              y = j;
            }
            j += 1;
          }
        }
        i += 1;
      }
    }
    solve(x, y);
    var ans = false;
    {
      var i = 0;
      while ((i < 3))
      {
        {
          var j = (n - 1);
          while ((j < 200))
          {
            ans |= vis[i][j];
            j += 1;
          }
        }
        i += 1;
      }
    }
    if (ans)
    {
      write("YES\n");
    } else
    {
      write("NO\n");
    }
  }
  return 0;
}
