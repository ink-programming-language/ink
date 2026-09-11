// Translated from solution.cpp.

var MOD: dynamic = (1e9 + 7);

func add(x: dynamic, y: dynamic, CMOD: dynamic = MOD) -> dynamic
{
  return ((((0 + x) + y)) % CMOD);
}

func mult(x: dynamic, y: dynamic, CMOD: dynamic = MOD) -> dynamic
{
  return ((((1 * x) * y)) % CMOD);
}

func fast_expo(x: dynamic, y: dynamic, CMOD: dynamic = MOD) -> dynamic
{
  if ((x == 0))
  {
    return 0;
  }
  if ((y == 0))
  {
    return 1;
  }
  var ans: dynamic = fast_expo(x, (y / 2), CMOD);
  ans = mult(ans, ans, CMOD);
  if ((y & 1))
  {
    ans = mult(ans, x, CMOD);
  }
  return ans;
}

var TAM: dynamic = (2e5 + 100);

var INF: dynamic = (LLONG_MAX / 4);

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(TAM);

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  read(n);
  var k: dynamic = cpp_uninitialized();
  read(k);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort(a, (a + n));
  if ((a[(n / 2)] == k))
  {
    write("0", "\n");
    return 0;
  }
  var ans: dynamic = 0;
  if ((a[(n / 2)] < k))
  {
    {
      var i: dynamic = (n / 2);
      while ((i < n))
      {
        ans += max(0, (k - a[i]));
        i += 1;
      }
    }
  } else
  {
    {
      var i: dynamic = (n / 2);
      while ((i >= 0))
      {
        ans += max(0, (a[i] - k));
        i -= 1;
      }
    }
  }
  write(ans, "\n");
  return 0;
}
