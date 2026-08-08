// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var board = cpp_array(51, 50);

var bb = cpp_array(26);

var visit = cpp_array(50, 50);

func dfs(d: dynamic, x: dynamic, y: dynamic, a: dynamic)
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

func main()
{
  scanf("%d %d", (&n), (&m));
  {
    var i = 0;
    while ((i < n))
    {
      scanf("%s", board[i]);
      {
        var j = 0;
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
    var a = 0;
    while ((a < 26))
    {
      if ((!bb[a]))
      {
        a += 1;
        continue;
      }
      {
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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
        var i = 0;
        while ((i < n))
        {
          {
            var j = 0;
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
