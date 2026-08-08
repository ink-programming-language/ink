// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<(int)(n);i++)");
}

enum cpp_enum_1
{
  UP,
  RIGHT,
  DOWN,
  LEFT
}

var dy = [-1, 0, 1, 0];

var dx = [0, 1, 0, -1];

var n: dynamic;

func adja(data: dynamic, y: dynamic, x: dynamic, dir: dynamic)
{
  var left = (((dir + 3)) % 4);
  var right = (((dir + 1)) % 4);
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

func main()
{
  var a: dynamic;
  read(a);
  var f = false;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
        if (data[i][j])
        {
          write("#");
        } else
        {
          write(" ");
        }
      }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      write("\n");
    }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if (f)
    {
      write("\n");
    }
    f = true;
    var data = [];
    read(n);
    var dir = UP;
    var y = (n - 1);
    var x = 0;
    var change_d_times = 0;
    while (1)
    {
      data[y][x] = true;
      var ddx = (x + dx[dir]);
      var ddy = (y + dy[dir]);
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
