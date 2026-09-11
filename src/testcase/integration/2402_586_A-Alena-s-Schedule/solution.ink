// Translated from solution.cpp.

var dx: dynamic = [1, -1, 0, 0];

var dy: dynamic = [0, 0, 1, -1];

var mark: dynamic = cpp_array((((10000000 >> 5)) + 1));

func sieve() -> dynamic
{
  var i: dynamic = cpp_uninitialized();
  var j: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  (cpp_assign(mark[(1 >> 5)], "|=", (1 << ((1 & 31)))));
  var n: dynamic = 10000000;
  {
    i = 2;
    while ((i <= n))
    {
      if ((!(((mark[(i >> 5)] >> ((i & 31))) & 1))))
      {
        {
          k = (n / i);
          j = (i * k);
          while ((k >= i))
          {
            (cpp_assign(mark[(j >> 5)], "|=", (1 << ((j & 31)))));
            k -= 1;
            j -= i;
          }
        }
      }
      i += 1;
    }
  }
}

func is_prime(x: dynamic) -> dynamic
{
  if ((x == 1))
  {
    return false;
  }
  {
    var i: dynamic = 2;
    while (((i * i) <= x))
    {
      if (((x % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func C(n: dynamic, k: dynamic) -> dynamic
{
  if ((k == 0))
  {
    return 1;
  }
  return (((n * C((n - 1), (k - 1)))) / k);
}

func modular_pow(budgetase: dynamic, exponent: dynamic, modulus: dynamic) -> dynamic
{
  var result: dynamic = 1;
  while ((exponent > 0))
  {
    if (((exponent % 2) == 1))
    {
      result = (((result * budgetase)) % modulus);
    }
    exponent = (exponent >> 1);
    budgetase = (((budgetase * budgetase)) % modulus);
  }
  return result;
}

func binaryToDec(number: dynamic) -> dynamic
{
  var result: dynamic = 0;
  var pow: dynamic = 1;
  {
    var i: dynamic = (number.length() - 1);
    while ((i >= 0))
    {
      result = (((result + (((number[i] - cpp_char("0"))) * pow))) % 1000003);
      i -= 1;
      pow <<= 1;
    }
  }
  return result;
}

func GCD(a: dynamic, b: dynamic) -> dynamic
{
  return  ((b == 0)) ? a : GCD(b, (a % b));
}

func cntMask(mask: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while (mask)
  {
    if ((mask % 2))
    {
      ret += 1;
    }
    mask /= 2;
  }
  return ret;
}

func getBit(mask: dynamic, i: dynamic) -> dynamic
{
  return (((((mask >> i)) & 1)) == 1);
}

func setBit(mask: dynamic, i: dynamic, value: dynamic = 1) -> dynamic
{
  return  ((value)) ? (mask | ((1 << i))) : ((mask & (~((1 << i)))));
}

func mystoi(s: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  var po: dynamic = 1;
  {
    var i: dynamic = (s.length() - 1);
    while ((i >= 0))
    {
      ans += (((s[i] - cpp_char("0"))) * po);
      po *= 10;
      i -= 1;
    }
  }
  return ans;
}

func conv(i: dynamic) -> dynamic
{
  var t: dynamic = "";
  while (i)
  {
    t += (cpp_char("0") + ((i % 10)));
    i /= 10;
  }
  reverse((t).begin(), (t).end());
  return t;
}

func hasZero(i: dynamic) -> dynamic
{
  if ((i == 0))
  {
    return true;
  }
  while (i)
  {
    if (((i % 10) == 0))
    {
      return true;
    }
    i /= 10;
  }
  return false;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array(101);
  read(n);
  var idx: dynamic = -1;
  var f: dynamic = cpp_uninitialized();
  var flag: dynamic = true;
  {
    var i: dynamic = 0;
    while ((i < int_cpp(n)))
    {
      read(a[i]);
      if ((a[i] == 1))
      {
        idx = i;
      }
      if (((a[i] == 1) && flag))
      {
        f = i;
        flag = false;
      }
      i += 1;
    }
  }
  if ((idx == -1))
  {
    write(0, "\n");
  } else
  {
    var ans: dynamic = 0;
    {
      var i: dynamic = f;
      while ((i <= idx))
      {
        ans += 1;
        if (((a[i] == 0) && (a[(i + 1)] == 0)))
        {
          while ((a[i] == 0))
          {
            i += 1;
          }
        }
        i += 1;
      }
    }
    write(ans, "\n");
  }
  return 0;
}
