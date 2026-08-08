// Translated from solution.cpp.

var MOD = 1000000007;

var INF = (cpp_cast(1000000007) * 1000000007);

var EPS = 1e-9;

var stop = cpp_expression("#include<i");

func rep(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func per(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=n-1;i>=0;i--)");
}

func Rep(i: dynamic, sta: dynamic, n: dynamic)
{
  cpp_macro("for(int i=sta;i<n;i++)");
}

func rep1(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

func per1(i: dynamic, n: dynamic)
{
  cpp_macro("for(int i=n;i>=1;i--)");
}

func Rep1(i: dynamic, sta: dynamic, n: dynamic)
{
  cpp_macro("for(int i=sta;i<=n;i++)");
}

class ant
{
  var loc: dynamic;
  var dir: dynamic;
}

var a = cpp_array(20);

var n: dynamic;

var l: dynamic;

var memo: dynamic;

func move()
{
  var f = true;
  return;
}

func antfall()
{
  return true;
}

func main()
{
  while (cpp_comma(((cin >> n) >> l), n))
  {
    var d: dynamic;
    var p: dynamic;
    var turn: dynamic;
    {
      turn = 1;
      while ((turn < 105))
      {
        move();
        if (antfall())
        {
          break;
        }
        turn += 1;
      }
    }
    write(turn, " ", (memo + 1), "\n");
  }
  return 0;
}

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if ((a[i].loc == 1))
    {
      memo = i;
      f = false;
    } else if (((a[i].loc == (l - 1)) && f))
    {
      memo = i;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if (((a[i].loc == 0) || (a[i].loc == l)))
    {
      continue;
    }
    if ((a[i].dir == cpp_char("R")))
    {
      a[i].loc += 1;
    } else
    {
      a[i].loc -= 1;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if (((a[i].loc == 0) || (a[i].loc == l)))
    {
      continue;
    }
    Rep(j, (i + 1), n);
    {
      if ((a[i].loc == a[j].loc))
      {
        swap(a[i].dir, a[j].dir);
      }
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
    if (((a[i].loc != 0) && (a[i].loc != l)))
    {
      return false;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic)
{
      read(d, p);
      a[i] = [p, d];
    }
