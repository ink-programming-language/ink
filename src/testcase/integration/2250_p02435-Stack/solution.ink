// Translated from solution.cpp.

var MOD = cpp_expression("#include <");

func REP(i: dynamic, N: dynamic)
{
  cpp_macro("for (int i = 0; i < N; ++i)");
}

func REP1(i: dynamic, N: dynamic)
{
  cpp_macro("for (int i = 1; i <= N; ++i)");
}

func RREP(i: dynamic, N: dynamic)
{
  cpp_macro("for (int i = N - 1; i >= 0; --i)");
}

func ALL(a: dynamic)
{
  return cpp_expression("#include <algorith");
}

func main()
{
  var n: dynamic;
  var q: dynamic;
  read(n, q);
  var st = cpp_array(n);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
    var num: dynamic;
    var t: dynamic;
    read(num, t);
    if ((num == 0))
    {
      var x: dynamic;
      read(x);
      st[t].push(x);
    } else if ((num == 1))
    {
      if (st[t].size())
      {
        write(st[t].top(), "\n");
      }
    } else
    {
      if (st[t].size())
      {
        st[t].pop();
      }
    }
  }
