// Translated from solution.cpp.

func operator_shift_left(out: dynamic, obj: dynamic)
{
  (((((out << "(") << obj.first) << ",") << obj.second) << ")");
  return out;
}

func operator_shift_left(out: dynamic, cont: dynamic)
{
  var itr = cont.begin();
  var ends = cont.end();
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

func operator_shift_left(out: dynamic, cont: dynamic)
{
  var itr = cont.begin();
  var ends = cont.end();
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

func operator_shift_left(out: dynamic, cont: dynamic)
{
  var itr = cont.begin();
  var ends = cont.end();
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

func operator_shift_left(out: dynamic, cont: dynamic)
{
  var itr = cont.begin();
  var ends = cont.end();
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

func operator_shift_left(out: dynamic, arr: dynamic)
{
  {
    var i = 0;
    while ((i < N))
    {
      ((out << arr[i]) << " ");
      i += 1;
    }
  }
  (out << endl);
  return out;
}

func gcd(a: dynamic, b: dynamic)
{
  var min_v = min(a, b);
  var max_v = max(a, b);
  while (min_v)
  {
    var temp = (max_v % min_v);
    max_v = min_v;
    min_v = temp;
  }
  return max_v;
}

func lcm(a: dynamic, b: dynamic)
{
  return (((a * b)) / gcd(a, b));
}

func fast_exp_pow(base: dynamic, exp: dynamic, mod: dynamic)
{
  var res = 1;
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

var A: dynamic;

var B: dynamic;

var A_b: dynamic;

var B_b: dynamic;

var base: dynamic;

var len: dynamic;

func main()
{
  scanf("%d%d", (&A), (&B));
  var tmp = A;
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
  var Tpow = 1;
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
  var sum = (A_b + B_b);
  while (sum)
  {
    len += 1;
    sum /= base;
  }
  printf("%d\n", len);
  return 0;
}
