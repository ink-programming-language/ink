// Translated from solution.cpp.

var dx = [1, -1, 0, 0];

var dy = [0, 0, 1, -1];

var mark = cpp_array((((10000000 >> 5)) + 1));

func sieve()
{
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  (cpp_assign(mark[(1 >> 5)], "|=", (1 << ((1 & 31)))));
  var n = 10000000;
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

func is_prime(x: dynamic)
{
  if ((x == 1))
  {
    return false;
  }
  {
    var i = 2;
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

func C(n: dynamic, k: dynamic)
{
  if ((k == 0))
  {
    return 1;
  }
  return (((n * C((n - 1), (k - 1)))) / k);
}

func modular_pow(budgetase: dynamic, exponent: dynamic, modulus: dynamic)
{
  var result = 1;
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

func binaryToDec(number: dynamic)
{
  var result = 0;
  var pow = 1;
  {
    var i = (number.length() - 1);
    while ((i >= 0))
    {
      result = (((result + (((number[i] - cpp_char("0"))) * pow))) % 1000003);
      i -= 1;
      pow <<= 1;
    }
  }
  return result;
}

func GCD(a: dynamic, b: dynamic)
{
  return if ((b == 0)) a else GCD(b, (a % b));
}

func cntMask(mask: dynamic)
{
  var ret = 0;
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

func getBit(mask: dynamic, i: dynamic)
{
  return (((((mask >> i)) & 1)) == 1);
}

func setBit(mask: dynamic, i: dynamic, value: dynamic = 1)
{
  return if ((value)) (mask | ((1 << i))) else ((mask & (~((1 << i)))));
}

func mystoi(s: dynamic)
{
  var ans = 0;
  var po = 1;
  {
    var i = (s.length() - 1);
    while ((i >= 0))
    {
      ans += (((s[i] - cpp_char("0"))) * po);
      po *= 10;
      i -= 1;
    }
  }
  return ans;
}

func conv(i: dynamic)
{
  var t = "";
  while (i)
  {
    t += (cpp_char("0") + ((i % 10)));
    i /= 10;
  }
  reverse((t).begin(), (t).end());
  return t;
}

func hasZero(i: dynamic)
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

func main()
{
  var n: dynamic;
  var a = cpp_array(101);
  read(n);
  var idx = -1;
  var f: dynamic;
  var flag = true;
  {
    var i = 0;
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
    var ans = 0;
    {
      var i = f;
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
