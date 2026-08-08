// Translated from solution.cpp.

var INF = 0x3f3f3f3f;

func operator_shift_left(out: dynamic, v: dynamic)
{
  (((((out << cpp_char("(")) << v.first) << cpp_char(",")) << v.second) << cpp_char(")"));
  return out;
}

class cprint
{
}

class cprint_string
{
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  for (var x in v)
  {
    ((out << x) << cpp_char(" "));
  }
  return out;
}

func chmax(x: dynamic, a: dynamic)
{
  if ((x < a))
  {
    x = a;
  }
}

func chmin(x: dynamic, a: dynamic)
{
  if ((x > a))
  {
    x = a;
  }
}

func mod(a: dynamic, b: dynamic)
{
  return ((((a % b) + b)) % b);
}

var n: dynamic;

var m: dynamic;

var k: dynamic;

var grid: dynamic;

var di = [-1, 1, 0, 0];

var dj = [0, 0, -1, 1];

func dfs(i: dynamic, j: dynamic)
{
  if ((k == 0))
  {
    return;
  }
  if (((((i < 0) || (i >= n)) || (j < 0)) || (j >= m)))
  {
    return;
  }
  if ((grid[i][j] != cpp_char(".")))
  {
    return;
  }
  grid[i][j] = cpp_char("T");
  {
    var k = 0;
    while ((k < 4))
    {
      dfs((i + di[k]), (j + dj[k]));
      k += 1;
    }
  }
  if (k)
  {
    grid[i][j] = cpp_char("X");
    k -= 1;
  }
}

func main()
{
  read(n, m, k);
  grid.resize(n);
  {
    var i = 0;
    while ((i < n))
    {
      read(grid[i]);
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
          if ((grid[i][j] == cpp_char(".")))
          {
            dfs(i, j);
            cpp_goto("goto stop;");
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
          if ((grid[i][j] == cpp_char("T")))
          {
            grid[i][j] = cpp_char(".");
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
      write(grid[i], "\n");
      i += 1;
    }
  }
}
