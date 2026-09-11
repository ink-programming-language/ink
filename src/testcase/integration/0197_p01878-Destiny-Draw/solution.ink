// Translated from solution.cpp.

func ALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++");
}

func RALL(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var PB: dynamic = cpp_expression("#include");

var EB: dynamic = cpp_expression("#include <bi");

var MP: dynamic = cpp_expression("#include");

func SZ(a: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func EACH(i: dynamic, c: dynamic) -> dynamic
{
  cpp_macro("for(typeof((c).begin()) i=(c).begin(); i!=(c).end(); ++i)");
}

func EXIST(s: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

func SORT(c: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h> us");
}

func FOR(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=(a);i<(b);++i)");
}

func REP(i: dynamic, n: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

var FF: dynamic = cpp_expression("#incl");

var SS: dynamic = cpp_expression("#inclu");

func operator_shift_right(is: dynamic, p: dynamic) -> dynamic
{
  return ((is >> p.FF) >> p.SS);
}

var EPS: dynamic = 1e-10;

var PI: dynamic = acos(-1.0);

var MOD: dynamic = (1e9 + 7);

func mul(A: dynamic, B: dynamic) -> dynamic
{
  var R: dynamic = A.size();
  var C: dynamic = B[0].size();
  var sz: dynamic = B.size();
  {
    var i: dynamic = 0;
    while ((i < R))
    {
      {
        var j: dynamic = 0;
        while ((j < C))
        {
          {
            var k: dynamic = 0;
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

func powA(A: dynamic, n: dynamic) -> dynamic
{
  var N: dynamic = A.size();
  var p: dynamic = cpp_construct(N, Col(N, 0));
  var w: dynamic = A;
  {
    var i: dynamic = 0;
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

func dump(A: dynamic) -> dynamic
{
}

func main() -> dynamic
{
  cin.tie(0);
  ios_base.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  var C: dynamic = cpp_uninitialized();
  var T: dynamic = cpp_uninitialized();
  read(N, K, C, T);
  C -= 1;
  var A: dynamic = cpp_construct((5 * N), Col((5 * N)));
  REP(i, N);
  REP(t, 4)[((5 * i) + t)][(((5 * i) + t) + 1)] += 1;
  A = powA(A, T);
  write(A[0][(5 * C)], "\n");
  return 0;
}

func REP(argument_0: dynamic, A: dynamic) -> dynamic
{
    ((REP(j, SZ(A[i])) << A[i][j]) << ( (((j % 5) == 4)) ? " | " : " "));
    write("\n");
    if (((i % 5) == 4))
    {
      write(string_cpp((SZ(A) + 20), cpp_char("-")), "\n");
    }
  }

func REP(argument_0: dynamic, argument_1: dynamic) -> dynamic
{
    read(as_cpp[i], bs[i], ts[i]);
    as_cpp[i] -= 1;
    ts[i] -= 1;
    {
      var k: dynamic = 0;
      while ((k < bs[i]))
      {
        A[((k * 5) + ts[i])][(((as_cpp[i] + k)) * 5)] += 1;
        k += 1;
      }
    }
    {
      var k: dynamic = 0;
      while ((k < as_cpp[i]))
      {
        A[((((bs[i] + k)) * 5) + ts[i])][(k * 5)] += 1;
        k += 1;
      }
    }
    {
      var k: dynamic = (as_cpp[i] + bs[i]);
      while ((k < N))
      {
        A[((5 * k) + ts[i])][(5 * k)] += 1;
        k += 1;
      }
    }
  }
