// Translated from solution.cpp.

var EPS = -1e8;

var Pi = acos(-1);

func equ(a: dynamic, b: dynamic)
{
  return (fabs((a - b)) < EPS);
}

var MAXN = 200010;

var MOD = 1000000007;

func add(a: dynamic, b: dynamic)
{
  return (((a + b)) % MOD);
}

func mul(a: dynamic, b: dynamic)
{
  return (((a * b)) % MOD);
}

func sub(a: dynamic, b: dynamic)
{
  return ((((a - b) + MOD)) % MOD);
}

class Mat
{
  var dat: dynamic = cpp_array(4, 4);
  func operator_index(i: dynamic)
  {
      return dat[i];
    }
}

var I = [[[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]]];

var F = [[[1, 1, 1, 0], [1, 0, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0]]];

func operator_multiply(A: dynamic, B: dynamic)
{
  var C: dynamic;
  {
    var i = (0);
    while ((i <= (3)))
    {
      {
        var j = (0);
        while ((j <= (3)))
        {
          C[i][j] = 0;
          {
            var k = (0);
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

func pw(A: dynamic, x: dynamic)
{
  assert((x >= 0));
  var base = A;
  var ret = I;
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

var K: dynamic;

var m: dynamic;

func cnt(t: dynamic, p: dynamic)
{
  var f = cpp_array(MAXN);
  if ((cpp_cast((p).size()) > cpp_cast((t).size())))
  {
    return 0;
  }
  {
    var i = 1;
    var j = cpp_assign(f[0], "=", -1);
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
  var res = 0;
  {
    var i = 0;
    var j = -1;
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

var fib = cpp_array(33);

func solve(s: dynamic)
{
  if ((K == 1))
  {
    return (s == "a");
  }
  if ((K == 2))
  {
    return (s == "b");
  }
  var A = "a";
  var B = "b";
  var i: dynamic;
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
  var fip1 = fib[(i + 1)];
  var CA = cnt(A, s);
  var CB = cnt(B, s);
  var CAB = ((cnt((A + B), s) - CA) - CB);
  var CBB = ((cnt((B + B), s) - CB) - CB);
  var Sip1 = cnt(fip1, s);
  if ((K == i))
  {
    return CB;
  }
  if ((K == (i + 1)))
  {
    return Sip1;
  }
  var Ans = pw(F, (K - cpp_cast(((i + 1)))));
  return add(add(mul(Ans[0][0], Sip1), mul(Ans[0][1], CB)), add(mul(Ans[0][2], CAB), mul(Ans[0][3], CBB)));
}

func main()
{
  fib[1] = "a";
  fib[2] = "b";
  {
    var i = (3);
    while ((i <= (30)))
    {
      fib[i] = (fib[(i - 1)] + fib[(i - 2)]);
      i += 1;
    }
  }
  ios_base.sync_with_stdio(0);
  read(K, m);
  {
    var i = (1);
    while ((i <= (m)))
    {
      var str: dynamic;
      read(str);
      write(solve(str), cpp_char("\n"));
      i += 1;
    }
  }
}
