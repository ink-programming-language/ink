// Translated from solution.cpp.

func power(x: dynamic, y: dynamic, z: dynamic)
{
  var ret = 1;
  while ((y > 0))
  {
    if ((y & 1))
    {
      ret = (((ret * x)) % z);
    }
    x = (((x * x)) % z);
    y >>= 1;
  }
  return ret;
}

var N = (2e5 + 5);

var A = cpp_array(N);

func gcd(a: dynamic, b: dynamic)
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main()
{
  var n: dynamic;
  scanf("%lld", (&n));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%lld", (&A[i]));
      i += 1;
    }
  }
  sort((A + 1), ((A + 1) + n));
  var x = A[1];
  var y = 0;
  var z = 0;
  var sum = A[1];
  {
    var i = 2;
    while ((i <= n))
    {
      if ((A[i] != A[(i - 1)]))
      {
        z = gcd(z, (A[i] - A[(i - 1)]));
      }
      x = max(x, A[i]);
      sum += A[i];
      i += 1;
    }
  }
  var l1 = (sum % z);
  var l2 = (n % z);
  var pz = z;
  var tz = z;
  {
    var i = 2;
    while ((i < N))
    {
      if (((tz % i) == 0))
      {
        while (((tz % i) == 0))
        {
          tz /= i;
        }
        pz -= (pz / i);
      }
      i += 1;
    }
  }
  if ((tz > 1))
  {
    pz -= (pz / tz);
  }
  var l3 = power(l2, (pz - 1), z);
  l3 = (((l3 * l1)) % z);
  var lo = 1;
  var hi = 1e10;
  var mid: dynamic;
  var tx = x;
  while ((lo <= hi))
  {
    mid = (((lo + hi)) >> 1);
    if ((((l3 + (z * mid))) >= tx))
    {
      hi = (mid - 1);
      x = ((l3 + (z * mid)));
    } else
    {
      lo = (mid + 1);
    }
  }
  var ts = ((n * x) - sum);
  y = ((((n * x) - sum)) / z);
  if ((gcd(l2, z) != 1))
  {
    y = ((((n * tx) - sum)) / z);
  }
  printf("%lld %lld\n", y, z);
  return 0;
}
