// Translated from solution.cpp.

func operator_shift_left(out: dynamic, obj: dynamic) -> dynamic
{
  (((((out << "(") << obj.first) << ",") << obj.second) << ")");
  return out;
}

func operator_shift_left(out: dynamic, cont: dynamic) -> dynamic
{
  var itr: dynamic = cont.begin();
  var ends: dynamic = cont.end();
  {
    while ((itr != ends))
    {
      ((out << (*itr)) << " ");
      itr += 1;
    }
  }
  (out << endl);
  return out;
}

func operator_shift_left(out: dynamic, cont: dynamic) -> dynamic
{
  var itr: dynamic = cont.begin();
  var ends: dynamic = cont.end();
  {
    while ((itr != ends))
    {
      ((out << (*itr)) << " ");
      itr += 1;
    }
  }
  (out << endl);
  return out;
}

func operator_shift_left(out: dynamic, cont: dynamic) -> dynamic
{
  var itr: dynamic = cont.begin();
  var ends: dynamic = cont.end();
  {
    while ((itr != ends))
    {
      ((out << (*itr)) << " ");
      itr += 1;
    }
  }
  (out << endl);
  return out;
}

func operator_shift_left(out: dynamic, cont: dynamic) -> dynamic
{
  var itr: dynamic = cont.begin();
  var ends: dynamic = cont.end();
  {
    while ((itr != ends))
    {
      ((out << (*itr)) << " ");
      itr += 1;
    }
  }
  (out << endl);
  return out;
}

func operator_shift_left(out: dynamic, arr: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      ((out << arr[i]) << " ");
      i += 1;
    }
  }
  (out << endl);
  return out;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  var min_v: dynamic = min(a, b);
  var max_v: dynamic = max(a, b);
  while (min_v)
  {
    var temp: dynamic = (max_v % min_v);
    max_v = min_v;
    min_v = temp;
  }
  return max_v;
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / gcd(a, b));
}

func fast_exp_pow(base: dynamic, exp: dynamic, mod: dynamic) -> dynamic
{
  var res: dynamic = 1;
  while (exp)
  {
    if ((exp & 1))
    {
      res *= base;
      res %= mod;
    }
    exp >>= 1;
    base *= base;
    base %= mod;
  }
  return (res % mod);
}

var A: dynamic = cpp_uninitialized();

var B: dynamic = cpp_uninitialized();

var A_b: dynamic = cpp_uninitialized();

var B_b: dynamic = cpp_uninitialized();

var base: dynamic = cpp_uninitialized();

var len: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  scanf("%d%d", (&A), (&B));
  var tmp: dynamic = A;
  while (tmp)
  {
    base = max(base, (tmp % 10));
    tmp /= 10;
  }
  tmp = B;
  while (tmp)
  {
    base = max(base, (tmp % 10));
    tmp /= 10;
  }
  base += 1;
  tmp = A;
  var Tpow: dynamic = 1;
  while (tmp)
  {
    A_b += (Tpow * ((tmp % 10)));
    tmp /= 10;
    Tpow *= base;
  }
  tmp = B;
  Tpow = 1;
  while (tmp)
  {
    B_b += (Tpow * ((tmp % 10)));
    tmp /= 10;
    Tpow *= base;
  }
  var sum: dynamic = (A_b + B_b);
  while (sum)
  {
    len += 1;
    sum /= base;
  }
  printf("%d\n", len);
  return 0;
}
