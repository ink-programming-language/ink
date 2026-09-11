// Translated from solution.cpp.

var EPS: dynamic = -1e8;

var Pi: dynamic = acos(-1);

func equ(a: dynamic, b: dynamic) -> dynamic
{
  return (fabs((a - b)) < EPS);
}

var MAXN: dynamic = 200010;

var MOD: dynamic = 1000000007;

func add(a: dynamic, b: dynamic) -> dynamic
{
  return (((a + b)) % MOD);
}

func mul(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) % MOD);
}

func sub(a: dynamic, b: dynamic) -> dynamic
{
  return ((((a - b) + MOD)) % MOD);
}

class Mat
{
  var dat: dynamic = cpp_array(4, 4);
  func operator_index(i: dynamic) -> dynamic
  {
      return dat[i];
    }
}

var I: dynamic = [[[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]];

var F: dynamic = [[[1, 1, 1, 0], [1, 0, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0]]];

func operator_multiply(A: dynamic, B: dynamic) -> dynamic
{
  var C: dynamic = cpp_uninitialized();
  {
    var i: dynamic = (0);
    while ((i <= (3)))
    {
      {
        var j: dynamic = (0);
        while ((j <= (3)))
        {
          C[i][j] = 0;
          {
            var k: dynamic = (0);
            while ((k <= (3)))
            {
              C[i][j] += mul(A[i][k], B[k][j]);
              k += 1;
            }
          }
          C[i][j] %= MOD;
          j += 1;
        }
      }
      i += 1;
    }
  }
  return C;
}

func pw(A: dynamic, x: dynamic) -> dynamic
{
  assert((x >= 0));
  var base: dynamic = A;
  var ret: dynamic = I;
  while ((x > 0))
  {
    if ((x & 1))
    {
      ret = (base * ret);
    }
    base = (base * base);
    x >>= 1;
  }
  return ret;
}

var K: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

func cnt(t: dynamic, p: dynamic) -> dynamic
{
  var f: dynamic = cpp_array(MAXN);
  if ((cpp_cast((p).size()) > cpp_cast((t).size())))
  {
    return 0;
  }
  {
    var i: dynamic = 1;
    var j: dynamic = cpp_assign(f[0], "=", -1);
    while ((i < cpp_cast((p).size())))
    {
      while (((j >= 0) && (p[(j + 1)] != p[i])))
      {
        j = f[j];
      }
      if ((p[(j + 1)] == p[i]))
      {
        j += 1;
      }
      f[i] = j;
      i += 1;
    }
  }
  var res: dynamic = 0;
  {
    var i: dynamic = 0;
    var j: dynamic = -1;
    while ((i < cpp_cast((t).size())))
    {
      while (((j >= 0) && (p[(j + 1)] != t[i])))
      {
        j = f[j];
      }
      if ((p[(j + 1)] == t[i]))
      {
        j += 1;
      }
      if ((j == (cpp_cast((p).size()) - 1)))
      {
        res += 1;
        j = f[j];
      }
      i += 1;
    }
  }
  return res;
}

var fib: dynamic = cpp_array(33);

func solve(s: dynamic) -> dynamic
{
  if ((K == 1))
  {
    return (s == "a");
  }
  if ((K == 2))
  {
    return (s == "b");
  }
  var A: dynamic = "a";
  var B: dynamic = "b";
  var i: dynamic = cpp_uninitialized();
  {
    i = 3;
    while ((i <= K))
    {
      assert((i < 30));
      if (((cpp_cast((fib[(i - 1)]).size()) >= cpp_cast((s).size())) || (i == K)))
      {
        B = fib[i];
        A = fib[(i - 1)];
        break;
      }
      i += 1;
    }
  }
  var fip1: dynamic = fib[(i + 1)];
  var CA: dynamic = cnt(A, s);
  var CB: dynamic = cnt(B, s);
  var CAB: dynamic = ((cnt((A + B), s) - CA) - CB);
  var CBB: dynamic = ((cnt((B + B), s) - CB) - CB);
  var Sip1: dynamic = cnt(fip1, s);
  if ((K == i))
  {
    return CB;
  }
  if ((K == (i + 1)))
  {
    return Sip1;
  }
  var Ans: dynamic = pw(F, (K - cpp_cast(((i + 1)))));
  return add(add(mul(Ans[0][0], Sip1), mul(Ans[0][1], CB)), add(mul(Ans[0][2], CAB), mul(Ans[0][3], CBB)));
}

func main() -> dynamic
{
  fib[1] = "a";
  fib[2] = "b";
  {
    var i: dynamic = (3);
    while ((i <= (30)))
    {
      fib[i] = (fib[(i - 1)] + fib[(i - 2)]);
      i += 1;
    }
  }
  ios_base.sync_with_stdio(0);
  read(K, m);
  {
    var i: dynamic = (1);
    while ((i <= (m)))
    {
      var str: dynamic = cpp_uninitialized();
      read(str);
      write(solve(str), cpp_char("\n"));
      i += 1;
    }
  }
}
