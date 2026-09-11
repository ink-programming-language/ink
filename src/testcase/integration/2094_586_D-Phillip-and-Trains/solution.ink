// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(200, 3);

var vis: dynamic = cpp_array(200, 3);

func check(x: dynamic, y: dynamic) -> dynamic
{
  return (((((x >= 0) && (y >= 0)) && (x < 3)) && (y < 200)));
}

func solve(cx: dynamic, cy: dynamic) -> dynamic
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

func main() -> dynamic
{
  read(t);
  while (cpp_update(t, "--"))
  {
    memset(vis, 0, cpp_sizeof((vis)));
    {
      var i: dynamic = 0;
      while ((i < 3))
      {
        {
          var j: dynamic = 0;
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
    var x: dynamic = cpp_uninitialized();
    var y: dynamic = cpp_uninitialized();
    {
      var i: dynamic = 0;
      while ((i < 3))
      {
        {
          var j: dynamic = 0;
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
    var ans: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i < 3))
      {
        {
          var j: dynamic = (n - 1);
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
