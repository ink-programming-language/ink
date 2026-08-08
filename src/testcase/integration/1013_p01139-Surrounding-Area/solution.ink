// Translated from solution.cpp.

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;++i)");
}

func rep1(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=1;i<=n;++i)");
}

var a = [];

var dx = [0, 0, -1, 1];

var dy = [-1, 1, 0, 0];

var h: dynamic;

var w: dynamic;

func paint(y: dynamic, x: dynamic, color: dynamic)
{
  if (((((fabs(a[y][x]) != 2) && (a[y][x] != 0))) || (a[y][x] == color)))
  {
    return;
  }
  if ((fabs((a[y][x] - color)) == 4))
  {
    color = 10;
  }
  a[y][x] = color;
  rep(i, 4);
  paint((y + dy[i]), (x + dx[i]), color);
}

func main(argument_0: dynamic)
{
  var tmp: dynamic;
  while (cpp_comma(((cin >> w) >> h), (w | h)))
  {
    var cntB = 0;
    var cntW = 0;
    rep(y, (h + 2))[y][0] = cpp_assign(a[y][(w + 1)], "=", 100);
    rep(x, (w + 2))[0][x] = cpp_assign(a[(h + 1)][x], "=", 100);
    write(cntB, " ", cntW, "\n");
  }
  return 0;
}

func rep1(argument_0: dynamic, argument_1: dynamic)
{
        read(tmp);
        if ((tmp == cpp_char(".")))
        {
          a[y][x] = 0;
        } else if ((tmp == cpp_char("B")))
        {
          a[y][x] = 1;
        } else
        {
          a[y][x] = -1;
        }
      }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
    }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
        if ((a[y][x] == 1))
        {
          paint((y + dy[i]), (x + dx[i]), 2);
        } else if ((a[y][x] == -1))
        {
          paint((y + dy[i]), (x + dx[i]), -2);
        }
      }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
    }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
        if ((a[y][x] == 2))
        {
          cntB += 1;
        } else if ((a[y][x] == -2))
        {
          cntW += 1;
        }
      }

func rep1(argument_0: dynamic, argument_1: dynamic)
{
    }
