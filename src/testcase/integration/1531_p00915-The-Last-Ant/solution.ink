// Translated from solution.cpp.

var MOD: dynamic = 1000000007;

var INF: dynamic = (cpp_cast(1000000007) * 1000000007);

var EPS: dynamic = 1e-9;

var stop: dynamic = cpp_expression("#include<i");

func rep(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=0;i<n;i++)");
}

func per(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=n-1;i>=0;i--)");
}

func Rep(i: dynamic, sta: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=sta;i<n;i++)");
}

func rep1(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=1;i<=n;i++)");
}

func per1(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=n;i>=1;i--)");
}

func Rep1(i: dynamic, sta: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for(int i=sta;i<=n;i++)");
}

class ant
{
  var loc: dynamic = cpp_uninitialized();
  var dir: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(20);

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var memo: dynamic = cpp_uninitialized();

func move() -> dynamic
{
  var f: dynamic = true;
  return;
}

func antfall() -> dynamic
{
  return true;
}

func main() -> dynamic
{
  while (cpp_comma(((cin >> n) >> l), n))
  {
    var d: dynamic = cpp_uninitialized();
    var p: dynamic = cpp_uninitialized();
    var turn: dynamic = cpp_uninitialized();
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
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

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    if (((a[i].loc != 0) && (a[i].loc != l)))
    {
      return false;
    }
  }

func rep(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      read(d, p);
      a[i] = [p, d];
    }
