// Translated from solution.cpp.

func SORT(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func BACKSORT(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std;");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(LL i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var SP: dynamic = cpp_expression("#include");

var mod: dynamic = 1000000007;

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var NN: dynamic = cpp_uninitialized();
  var MM: dynamic = cpp_uninitialized();
  read(NN, MM);
  var N: dynamic = (((1 << NN)) - 1);
  var M: dynamic = (((1 << MM)) - 1);
  var vec: dynamic = cpp_construct(N, vector(M, 0));
  var num: dynamic = 0;
}

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      vec[i][j] = 0;
      var ii: dynamic = i;
      var jj: dynamic = j;
      while (true)
      {
        if ((((ii % 2) == 0) && ((jj % 2) == 0)))
        {
          vec[i][j] = 1;
          break;
        }
        if ((((ii % 2) == 1) && ((jj % 2) == 1)))
        {
          ii /= 2;
          jj /= 2;
          continue;
        }
        break;
      }
      num += 1;
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
      write(vec[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    write("\n");
  }
