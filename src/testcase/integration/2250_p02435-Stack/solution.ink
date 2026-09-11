// Translated from solution.cpp.

var MOD: dynamic = cpp_expression("#include <");

func REP(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < N; ++i)");
}

func REP1(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for (int i = 1; i <= N; ++i)");
}

func RREP(i: dynamic, N: dynamic) -> dynamic
{
  cpp_macro("for (int i = N - 1; i >= 0; --i)");
}

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <algorith");
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  read(n, q);
  var st: dynamic = cpp_array(n);
  return 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    var num: dynamic = cpp_uninitialized();
    var t: dynamic = cpp_uninitialized();
    read(num, t);
    if ((num == 0))
    {
      var x: dynamic = cpp_uninitialized();
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
