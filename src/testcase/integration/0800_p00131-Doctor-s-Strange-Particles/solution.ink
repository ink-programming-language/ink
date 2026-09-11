// Translated from solution.cpp.

var H: dynamic = 10;

var W: dynamic = 10;

var G: dynamic = cpp_array(W, H);

var ans: dynamic = cpp_array(W, H);

func put(i: dynamic, j: dynamic) -> dynamic
{
  var di: dynamic = [0, 1, 0, -1, 0];
  var dj: dynamic = [1, 0, -1, 0, 0];
  {
    var k: dynamic = 0;
    while ((k < 5))
    {
      var ni: dynamic = (i + di[k]);
      var nj: dynamic = (j + dj[k]);
      if (((ni < 0) || (ni >= H)))
      {
        k += 1;
        continue;
      }
      if (((nj < 0) || (nj >= W)))
      {
        k += 1;
        continue;
      }
      G[ni][nj] ^= 1;
      k += 1;
    }
  }
}

func rec(i: dynamic, j: dynamic) -> dynamic
{
  if ((j == W))
  {
    i += 1;
    j = 0;
  }
  if ((i == H))
  {
    {
      var j: dynamic = 0;
      while ((j < W))
      {
        if (G[(H - 1)][j])
        {
          return false;
        }
        j += 1;
      }
    }
    return true;
  }
  if ((i == 0))
  {
    ans[i][j] = 0;
    if (rec(i, (j + 1)))
    {
      return true;
    }
    ans[i][j] = 1;
    put(i, j);
    if (rec(i, (j + 1)))
    {
      return true;
    }
    put(i, j);
  } else
  {
    if (G[(i - 1)][j])
    {
      ans[i][j] = 1;
      put(i, j);
      if (rec(i, (j + 1)))
      {
        return true;
      }
      put(i, j);
    } else
    {
      ans[i][j] = 0;
      if (rec(i, (j + 1)))
      {
        return true;
      }
    }
  }
  return false;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  while (cpp_update(n, "--"))
  {
    {
      var i: dynamic = 0;
      while ((i < H))
      {
        {
          var j: dynamic = 0;
          while ((j < W))
          {
            read(G[i][j]);
            j += 1;
          }
        }
        i += 1;
      }
    }
    rec(0, 0);
    {
      var i: dynamic = 0;
      while ((i < H))
      {
        {
          var j: dynamic = 0;
          while ((j < W))
          {
            if (j)
            {
              write(" ");
            }
            write(ans[i][j]);
            j += 1;
          }
        }
        write("\n");
        i += 1;
      }
    }
  }
  return 0;
}
