// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<(int)(n);i++)");
}

enum cpp_enum_1
{
  enum_field UP;
  enum_field RIGHT;
  enum_field DOWN;
  enum_field LEFT;
}

var dy: dynamic = [-1, 0, 1, 0];

var dx: dynamic = [0, 1, 0, -1];

var n: dynamic = cpp_uninitialized();

func adja(data: dynamic, y: dynamic, x: dynamic, dir: dynamic) -> dynamic
{
  var left: dynamic = (((dir + 3)) % 4);
  var right: dynamic = (((dir + 1)) % 4);
  if (((((((x + dx[left]) >= 0) && ((y + dy[left]) >= 0)) && ((x + dx[left]) < n)) && ((y + dy[left]) < n)) && (data[(y + dy[left])][(x + dx[left])] == true)))
  {
    return true;
  }
  if (((((((x + dx[right]) >= 0) && ((y + dy[right]) >= 0)) && ((x + dx[right]) < n)) && ((y + dy[right]) < n)) && (data[(y + dy[right])][(x + dx[right])] == true)))
  {
    return true;
  }
  return false;
}

func main() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  read(a);
  var f: dynamic = false;
}

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
        if (data[i][j])
        {
          write("#");
        } else
        {
          write(" ");
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      write("\n");
    }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if (f)
    {
      write("\n");
    }
    f = true;
    var data: dynamic = [];
    read(n);
    var dir: dynamic = UP;
    var y: dynamic = (n - 1);
    var x: dynamic = 0;
    var change_d_times: dynamic = 0;
    while (1)
    {
      data[y][x] = true;
      var ddx: dynamic = (x + dx[dir]);
      var ddy: dynamic = (y + dy[dir]);
      if ((((((((ddx >= 0) && (ddy >= 0)) && (ddx < n)) && (ddy < n)) && (!data[ddy][ddx])) && (!(((((((ddx + dx[dir]) >= 0) && ((ddy + dy[dir]) >= 0)) && ((ddx + dx[dir]) < n)) && ((ddy + dy[dir]) < n)) && (data[(ddy + dy[dir])][(ddx + dx[dir])] == true))))) && (!adja(data, ddy, ddx, dir))))
      {
        data[ddy][ddx] = true;
        change_d_times = 0;
        x = ddx;
        y = ddy;
      } else
      {
        dir = (((dir + 1)) % 4);
        change_d_times += 1;
      }
      if ((change_d_times >= 5))
      {
        break;
      }
    }
  }
