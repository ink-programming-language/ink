// Translated from solution.cpp.

class outputer
{
}

class outputable
{
}

func sqr(x: dynamic) -> dynamic
{
  return (x * x);
}

func umx(a: dynamic, b: dynamic) -> dynamic
{
  if ((a < b))
  {
    a = b;
    return 1;
  }
  return 0;
}

func umn(a: dynamic, b: dynamic) -> dynamic
{
  if ((b < a))
  {
    a = b;
    return 1;
  }
  return 0;
}

var N: dynamic = 200000;

var mod: dynamic = 1000000007;

class Input
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(N);
  func read() -> dynamic
  {
      if ((!((cin >> n))))
      {
        return 0;
      }
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          scanf(cpp_expression("\"%\""), SCNd64, (&a[i]));
          i += 1;
        }
      }
      return 1;
    }
  func init(input: dynamic) -> dynamic
  {
      (*self) = input;
    }
}

class Data
{
  var ans: dynamic = cpp_uninitialized();
  func write() -> dynamic
  {
      write(ans, "\n");
    }
}

class Solution
{
  var f: dynamic = cpp_array((N + 1));
  var rf: dynamic = cpp_array((N + 1));
  func c(x: dynamic, y: dynamic) -> dynamic
  {
      return ((((f[(x + y)] * rf[x]) % mod) * rf[y]) % mod);
    }
  func pw(val: dynamic, k: dynamic) -> dynamic
  {
      var res: dynamic = 1;
      while (k)
      {
        if ((k & 1))
        {
          res = ((res * val) % mod);
        }
        val = ((val * val) % mod);
        k >>= 1;
      }
      return res;
    }
  var sgn: dynamic = cpp_uninitialized();
  func iteration() -> dynamic
  {
      n -= 1;
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          a[i] = ((((a[i] + (sgn * a[(i + 1)])) + mod)) % mod);
          sgn *= -1;
          i += 1;
        }
      }
    }
  func calc(t: dynamic) -> dynamic
  {
      var m: dynamic = ((((n + 1) - t)) / 2);
      var res: dynamic = 0;
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(m)))
        {
          res = (((res + (a[(t + (2 * i))] * c(i, ((m - 1) - i))))) % mod);
          i += 1;
        }
      }
      return res;
    }
  func solve() -> dynamic
  {
      f[0] = 1;
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp(n)))
        {
          f[(i + 1)] = ((f[i] * ((i + 1))) % mod);
          i += 1;
        }
      }
      {
        var i: dynamic = int_cpp(0);
        while ((i < int_cpp((n + 1))))
        {
          rf[i] = pw(f[i], (mod - 2));
          i += 1;
        }
      }
      if ((n == 1))
      {
        ans = a[0];
        return;
      }
      sgn = 1;
      if ((n & 1))
      {
        iteration();
      }
      var val0: dynamic = calc(0);
      var val1: dynamic = calc(1);
      if ((((n / 2) % 2) == 0))
      {
        sgn *= -1;
      }
      ans = ((((val0 + (sgn * val1)) + mod)) % mod);
    }
  func clear() -> dynamic
  {
      (*self) = Solution();
    }
}

var sol: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  cout.setf((ios.showpoint | ios.fixed));
  cout.precision(20);
  sol.read();
  sol.solve();
  sol.write();
  return 0;
}
