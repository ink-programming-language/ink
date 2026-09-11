// Translated from solution.cpp.

var pt: dynamic = cpp_uninitialized();

func get_int() -> dynamic
{
  var temp: dynamic = 0;
  while ((((*pt) < cpp_char("0")) && ((*pt) > cpp_char("9"))))
  {
    pt += 1;
  }
  while ((((*pt) >= cpp_char("0")) && ((*pt) <= cpp_char("9"))))
  {
    temp = ((temp * 10) + (((*pt) - cpp_char("0"))));
    pt += 1;
  }
  pt += 1;
  return temp;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if (((b == 1) || (a == 1)))
  {
    return 1;
  }
  if (((a % b) == 0))
  {
    return b;
  } else
  {
    return gcd(b, (a % b));
  }
}

func fastPower(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return (a % 747474747);
  }
  var ans: dynamic = fastPower(a, (b / 2));
  ans *= ans;
  ans %= 747474747;
  if ((b % 2))
  {
    ans *= a;
  }
  return (ans % 747474747);
}

func fastPowerCustomMod(a: dynamic, b: dynamic, c: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return 1;
  }
  if ((b == 1))
  {
    return (a % c);
  }
  var ans: dynamic = fastPower(a, (b / 2));
  ans *= ans;
  ans %= c;
  if ((b % 2))
  {
    ans *= a;
  }
  return (ans % c);
}

func is_prime(num: dynamic) -> dynamic
{
  if (((num == 2) || (num == 3)))
  {
    return 1;
  }
  if ((((num % 2) == 0) || ((num % 3) == 0)))
  {
    return 0;
  }
  var i: dynamic = 5;
  var w: dynamic = 2;
  while (((i * i) <= num))
  {
    if (((num % i) == 0))
    {
      return 0;
    }
    i += w;
    w = (6 - w);
  }
  return 1;
}

func compare(a: dynamic, b: dynamic) -> dynamic
{
  return (((*cpp_cast(a)) - (*cpp_cast(b))));
}

func dec_compare(a: dynamic, b: dynamic) -> dynamic
{
  return (((-(*cpp_cast(a))) + (*cpp_cast(b))));
}

func eularTotient(n: dynamic) -> dynamic
{
  var result: dynamic = n;
  {
    var i: dynamic = 2;
    while (((i * i) <= n))
    {
      if (((n % i) == 0))
      {
        result -= (result / i);
      }
      while (((n % i) == 0))
      {
        n /= i;
      }
      i += 1;
    }
  }
  if ((n > 1))
  {
    result -= (result / n);
  }
  return result;
}

var in_cpp: dynamic = cpp_uninitialized();

var out: dynamic = cpp_uninitialized();

var str: dynamic = cpp_array(1000000);

func main() -> dynamic
{
  scanf("%s", str);
  var h: dynamic = cpp_uninitialized();
  var min: dynamic = cpp_uninitialized();
  h = (((((str[0] - cpp_char("0"))) * 10) + str[1]) - cpp_char("0"));
  min = (((((str[3] - cpp_char("0"))) * 10) + str[4]) - cpp_char("0"));
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  a = ((h * 30.0) + (((min * 30.0)) / 60.0));
  b = (min * 6.0);
  while ((a >= 360.0))
  {
    a -= 360.0;
  }
  while ((b >= 360.0))
  {
    b -= 360.0;
  }
  write(a, " ", b, "\n");
  return 0;
}
