// Translated from solution.cpp.

var DE: dynamic = cpp_expression("#");

var FI: dynamic = cpp_expression("#incl");

var SE: dynamic = cpp_expression("#inclu");

var PB: dynamic = cpp_expression("#include");

var MP: dynamic = cpp_expression("#include");

func ALL(s: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  cpp_macro("for (int i = 0; i < (int)(n); ++i)");
}

func EACH(i: dynamic, s: dynamic) -> dynamic
{
  cpp_macro("for (__typeof__((s).begin()) i = (s).begin(); i != (s).end(); ++i)");
}

func COUT(x: dynamic) -> dynamic
{
  return cpp_expression("#include <iostream> #include <sstream> #include");
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  return (((((s << cpp_char("<")) << P.first) << ", ") << P.second) << cpp_char(">"));
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  (s << "{ ");
  {
    var i: dynamic = 0;
    while ((i < P.size()))
    {
      if ((i > 0))
      {
        (s << ", ");
      }
      (s << P[i]);
      i += 1;
    }
  }
  return ((s << " }") << endl);
}

func operator_shift_left(s: dynamic, P: dynamic) -> dynamic
{
  (s << "{ ");
  {
    typeof(P.begin()) = P.begin();
    while ((it != P.end()))
    {
      if ((it != P.begin()))
      {
        (s << ", ");
      }
      (((((s << cpp_char("<")) << it->first) << "->") << it->second) << cpp_char(">"));
      it += 1;
    }
  }
  return ((s << " }") << endl);
}

var N: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

var m: dynamic = cpp_array(105);

var dp: dynamic = cpp_array(20100, 105);

func main() -> dynamic
{
  while (((((cin >> N) >> L) >> M) >> R))
  {
    {
      var i: dynamic = 0;
      while ((i < N))
      {
        read(m[i]);
        i += 1;
      }
    }
    memset(dp, 0, cpp_sizeof((dp)));
    R *= 2;
    M *= 2;
    dp[0][0] = 1;
    {
      var i: dynamic = 0;
      while ((i <= N))
      {
        {
          var j: dynamic = 0;
          while ((j <= M))
          {
            if ((dp[i][j] <= 0))
            {
              j += 1;
              continue;
            }
            var l1: dynamic = max(0, (j + (m[i] * R)));
            var r1: dynamic = (min(M, (j + (m[i] * L))) + 1);
            var l2: dynamic = cpp_uninitialized();
            var r2: dynamic = cpp_uninitialized();
            if (((j - (m[i] * L)) >= 0))
            {
              l2 = (j - (m[i] * L));
              r2 = (min(M, (j - (m[i] * R))) + 1);
            } else if (((j - (m[i] * R)) >= 0))
            {
              l2 = 0;
              r2 = (min(M, max(abs((j - (m[i] * L))), abs((j - (m[i] * R))))) + 1);
            } else
            {
              l2 = abs((j - (m[i] * R)));
              r2 = (min(M, abs((j - (m[i] * L)))) + 1);
            }
            if ((l1 <= r1))
            {
              dp[(i + 1)][l1] += 1;
              dp[(i + 1)][r1] -= 1;
            }
            if ((l2 <= r2))
            {
              dp[(i + 1)][l2] += 1;
              dp[(i + 1)][r2] -= 1;
            }
            j += 1;
          }
        }
        {
          var j: dynamic = 0;
          while ((j <= M))
          {
            dp[(i + 1)][(j + 1)] += dp[(i + 1)][j];
            j += 1;
          }
        }
        i += 1;
      }
    }
    var exist: dynamic = false;
    {
      var i: dynamic = 0;
      while ((i <= M))
      {
        if ((dp[N][i] > 0))
        {
          exist = true;
          break;
        }
        i += 1;
      }
    }
    if (exist)
    {
      write("Yes", "\n");
    } else
    {
      write("No", "\n");
    }
  }
  return 0;
}
