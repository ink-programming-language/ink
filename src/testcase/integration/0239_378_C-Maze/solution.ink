// Translated from solution.cpp.

var INF: dynamic = 0x3f3f3f3f;

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
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

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  for (var x: dynamic in v)
  {
    ((out << x) << cpp_char(" "));
  }
  return out;
}

func chmax(x: dynamic, a: dynamic) -> dynamic
{
  if ((x < a))
  {
    x = a;
  }
}

func chmin(x: dynamic, a: dynamic) -> dynamic
{
  if ((x > a))
  {
    x = a;
  }
}

func mod(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a % b) + b)) % b);
}

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var grid: dynamic = cpp_uninitialized();

var di: dynamic = [-1, 1, 0, 0];

var dj: dynamic = [0, 0, -1, 1];

func dfs(i: dynamic, j: dynamic) -> dynamic
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
    var k: dynamic = 0;
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

func main() -> dynamic
{
  read(n, m, k);
  grid.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(grid[i]);
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
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < n))
    {
      write(grid[i], "\n");
      i += 1;
    }
  }
}
