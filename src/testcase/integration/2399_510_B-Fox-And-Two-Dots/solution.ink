// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var board: dynamic = cpp_array(51, 50);

var bb: dynamic = cpp_array(26);

var visit: dynamic = cpp_array(50, 50);

func dfs(d: dynamic, x: dynamic, y: dynamic, a: dynamic) -> dynamic
{
  if ((((d != 0) && ((x + 1) < n)) && (board[(x + 1)][y] == a)))
  {
    if (visit[(x + 1)][y])
    {
      return true;
    }
    visit[(x + 1)][y] = true;
    if (dfs(1, (x + 1), y, a))
    {
      return true;
    }
  }
  if ((((d != 1) && ((x - 1) >= 0)) && (board[(x - 1)][y] == a)))
  {
    if (visit[(x - 1)][y])
    {
      return true;
    }
    visit[(x - 1)][y] = true;
    if (dfs(0, (x - 1), y, a))
    {
      return true;
    }
  }
  if ((((d != 2) && ((y + 1) < m)) && (board[x][(y + 1)] == a)))
  {
    if (visit[x][(y + 1)])
    {
      return true;
    }
    visit[x][(y + 1)] = true;
    if (dfs(3, x, (y + 1), a))
    {
      return true;
    }
  }
  if ((((d != 3) && ((y - 1) >= 0)) && (board[x][(y - 1)] == a)))
  {
    if (visit[x][(y - 1)])
    {
      return true;
    }
    visit[x][(y - 1)] = true;
    if (dfs(2, x, (y - 1), a))
    {
      return true;
    }
  }
  return false;
}

func main() -> dynamic
{
  scanf("%d %d", (&n), (&m));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      scanf("%s", board[i]);
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          bb[(board[i][j] - cpp_char("A"))] = true;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var a: dynamic = 0;
    while ((a < 26))
    {
      if ((!bb[a]))
      {
        a += 1;
        continue;
      }
      {
        var i: dynamic = 0;
        while ((i < n))
        {
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              if ((board[i][j] == (a + cpp_char("A"))))
              {
                visit[i][j] = false;
              }
              j += 1;
            }
          }
          i += 1;
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
              if (((board[i][j] == (a + cpp_char("A"))) && (!visit[i][j])))
              {
                visit[i][j] = true;
                if (dfs(-1, i, j, (a + cpp_char("A"))))
                {
                  puts("Yes");
                  return 0;
                }
              }
              j += 1;
            }
          }
          i += 1;
        }
      }
      a += 1;
    }
  }
  puts("No");
  return 0;
}
