// Translated from solution.cpp.

func power(x: dynamic, y: dynamic, z: dynamic) -> dynamic
{
  var ret: dynamic = 1;
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

var N: dynamic = (2e5 + 5);

var A: dynamic = cpp_array(N);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((!b))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  scanf("%lld", (&n));
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      scanf("%lld", (&A[i]));
      i += 1;
    }
  }
  sort((A + 1), ((A + 1) + n));
  var x: dynamic = A[1];
  var y: dynamic = 0;
  var z: dynamic = 0;
  var sum: dynamic = A[1];
  {
    var i: dynamic = 2;
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
  var l1: dynamic = (sum % z);
  var l2: dynamic = (n % z);
  var pz: dynamic = z;
  var tz: dynamic = z;
  {
    var i: dynamic = 2;
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
  var l3: dynamic = power(l2, (pz - 1), z);
  l3 = (((l3 * l1)) % z);
  var lo: dynamic = 1;
  var hi: dynamic = 1e10;
  var mid: dynamic = cpp_uninitialized();
  var tx: dynamic = x;
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
  var ts: dynamic = ((n * x) - sum);
  y = ((((n * x) - sum)) / z);
  if ((gcd(l2, z) != 1))
  {
    y = ((((n * tx) - sum)) / z);
  }
  printf("%lld %lld\n", y, z);
  return 0;
}
