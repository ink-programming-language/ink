// Translated from solution.cpp.

func ALL(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++");
}

func RALL(a: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var PB = cpp_expression("#include");

var EB = cpp_expression("#include <bi");

var MP = cpp_expression("#include");

func SZ(a: dynamic)
{
  return cpp_expression("#include <bits/");
}

func EACH(i: dynamic, c: dynamic)
{
  cpp_macro("for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); ++i)");
}

func EXIST(s: dynamic, e: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func SORT(c: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h> us");
}

func FOR(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic)
{
  return cpp_expression("#include <");
}

var FF = cpp_expression("#incl");

var SS = cpp_expression("#inclu");

func operator_shift_right(is: dynamic, p: dynamic)
{
  return ((is >> p.FF) >> p.SS);
}

var EPS = 1e-10;

var PI = acos(-1.0);

var MOD = (1e9 + 7);

func mul(A: dynamic, B: dynamic)
{
  var R = A.size();
  var C = B[0].size();
  var sz = B.size();
  {
    var i = 0;
    while ((i < R))
    {
      {
        var j = 0;
        while ((j < C))
        {
          {
            var k = 0;
            while ((k < sz))
            {
              (cpp_assign(AB[i][j], "+=", (A[i][k] * B[k][j]))) %= MOD;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  return AB;
}

func powA(A: dynamic, n: dynamic)
{
  var N = A.size();
  var p = cpp_construct(N, Col(N, 0));
  var w = A;
  {
    var i = 0;
    while ((i < N))
    {
      p[i][i] = 1;
      i += 1;
    }
  }
  while ((n > 0))
  {
    if ((n & 1))
    {
      p = mul(p, w);
    }
    w = mul(w, w);
    n >>= 1;
  }
  return p;
}

func dump(A: dynamic)
{
}

func main()
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var N: dynamic;
  var K: dynamic;
  var C: dynamic;
  var T: dynamic;
  read(N, K, C, T);
  C -= 1;
  var A = cpp_construct((5 * N), Col((5 * N)));
  REP(i, N);
  REP(t, 4)[((5 * i) + t)][(((5 * i) + t) + 1)] += 1;
  A = powA(A, T);
  write(A[0][(5 * C)], "\n");
  return 0;
}

func REP(argument_0: dynamic, A: dynamic)
{
    ((REP(j, SZ(A[i])) << A[i][j]) << (if (((j % 5) == 4)) " | " else " "));
    write("\n");
    if (((i % 5) == 4))
    {
      write(string_cpp((SZ(A) + 20), cpp_char("-")), "\n");
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic)
{
    read(as_cpp[i], bs[i], ts[i]);
    as_cpp[i] -= 1;
    ts[i] -= 1;
    {
      var k = 0;
      while ((k < bs[i]))
      {
        A[((k * 5) + ts[i])][(((as_cpp[i] + k)) * 5)] += 1;
        k += 1;
      }
    }
    {
      var k = 0;
      while ((k < as_cpp[i]))
      {
        A[((((bs[i] + k)) * 5) + ts[i])][(k * 5)] += 1;
        k += 1;
      }
    }
    {
      var k = (as_cpp[i] + bs[i]);
      while ((k < N))
      {
        A[((5 * k) + ts[i])][(5 * k)] += 1;
        k += 1;
      }
    }
  }
