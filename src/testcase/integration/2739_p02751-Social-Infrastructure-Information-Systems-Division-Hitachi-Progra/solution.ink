// Translated from solution.cpp.

func SORT(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> u");
}

func BACKSORT(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> using namespace std;");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(LL i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var SP = cpp_expression("#include");

var mod = 1000000007;

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var NN: dynamic;
  var MM: dynamic;
  read(NN, MM);
  var N = (((1 << NN)) - 1);
  var M = (((1 << MM)) - 1);
  var vec = cpp_construct(N, vector(M, 0));
  var num = 0;
}

func REP(argument_0: dynamic, argument_1: dynamic)
{
      vec[i][j] = 0;
      var ii = i;
      var jj = j;
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

func REP(argument_0: dynamic, argument_1: dynamic)
{
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
      write(vec[i][j]);
    }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    write("\n");
  }
