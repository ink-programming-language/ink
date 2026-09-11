// Translated from solution.cpp.

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

func is_palindrome(s1: dynamic) -> dynamic
{
  var l: dynamic = s1.length();
  {
    var i: dynamic = 0;
    while ((i < (l / 2)))
    {
      if ((s1[i] != s1[((l - i) - 1)]))
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

func modular_pow(base: dynamic, exponent: dynamic, modulus: dynamic) -> dynamic
{
  var result: dynamic = 1;
  while ((exponent > 0))
  {
    if (((exponent % 2) == 1))
    {
      result = (((result * base)) % modulus);
    }
    exponent = (exponent >> 1);
    base = (((base * base)) % modulus);
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

func main(argument_0: dynamic) -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var a: dynamic = cpp_array((100000 + 5));
  read(n);
  {
    var i: dynamic = 0;
    while ((i < int_cpp(n)))
    {
      read(a[i]);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  var temp: dynamic = 0;
  var cur: dynamic = a[0];
  {
    var i: dynamic = 0;
    while ((i < int_cpp(n)))
    {
      if ((a[i] == cur))
      {
        temp += 1;
      } else
      {
        ans += ((temp * ((temp + 1))) / 2);
        temp = 1;
        cur = a[i];
      }
      i += 1;
    }
  }
  ans += ((temp * ((temp + 1))) / 2);
  write(ans, "\n");
}
