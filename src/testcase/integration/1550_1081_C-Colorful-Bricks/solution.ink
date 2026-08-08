// Translated from solution.cpp.

func gcd(a: dynamic, b: dynamic)
{
  if (((a == 0) || (b == 0)))
  {
    return (a + b);
  } else if (((a % b) == 0))
  {
    return b;
  } else
  {
    return gcd(b, (a % b));
  }
}

func power(base: dynamic, pow: dynamic)
{
  if ((pow == 0))
  {
    return 1;
  }
  var base_to_the_power_pow = power(base, (pow / 2));
  base_to_the_power_pow = (((base_to_the_power_pow * base_to_the_power_pow)) % 998244353);
  if ((pow % 2))
  {
    base_to_the_power_pow = (((base_to_the_power_pow * base)) % 998244353);
  }
  var result = cpp_cast(base_to_the_power_pow);
  return result;
}

func combination(n: dynamic, r: dynamic)
{
  if ((r > n))
  {
    return 0;
  }
  if ((r > (n - r)))
  {
    r = (n - r);
  }
  var numbers = cpp_array(2, r);
  var i: dynamic;
  var j: dynamic;
  var GCD: dynamic;
  var k: dynamic;
  {
    i = 0;
    while ((i < r))
    {
      numbers[i][0] = (n - i);
      numbers[i][1] = (i + 1);
      i += 1;
    }
  }
  {
    i = (r - 1);
    while ((i > 0))
    {
      k = 0;
      {
        j = 0;
        while (((numbers[i][1] > 1) && (j < r)))
        {
          GCD = gcd(numbers[i][1], numbers[j][0]);
          numbers[i][1] = (numbers[i][1] / GCD);
          numbers[j][0] = (numbers[j][0] / GCD);
          k += 1;
          j += 1;
        }
      }
      i -= 1;
    }
  }
  var Combination = 1;
  {
    i = 0;
    while ((i < r))
    {
      Combination = (((Combination * numbers[i][0])) % 998244353);
      i += 1;
    }
  }
  var result = cpp_cast(Combination);
  return result;
}

func main()
{
  var i: dynamic;
  var j: dynamic;
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  var number_of_ways: dynamic;
  scanf("%d %d %d", (&n), (&m), (&k));
  number_of_ways = m;
  number_of_ways = (((number_of_ways * power((m - 1), k))) % 998244353);
  number_of_ways = (((number_of_ways * combination((n - 1), k))) % 998244353);
  printf("%I64d", number_of_ways);
  return 0;
}
