// Translated from solution.cpp.

func isPrime(n: dynamic) -> dynamic
{
  if ((n <= 1))
  {
    return 0;
  }
  if ((n <= 3))
  {
    return 1;
  }
  if ((((n % 2) == 0) || ((n % 3) == 0)))
  {
    return 0;
  }
  {
    var i: dynamic = 5;
    while (((i * i) <= n))
    {
      if ((((n % i) == 0) || ((n % ((i + 2))) == 0)))
      {
        return 0;
      }
      i += 6;
    }
  }
  return 1;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return (((a * b)) / (gcd(a, b)));
}

func swap(a: dynamic, b: dynamic) -> dynamic
{
  a = (a ^ b);
  b = (a ^ b);
  a = (a ^ b);
}

func solve() -> dynamic
{
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var x: dynamic = 0;
  var y: dynamic = 0;
  var z: dynamic = 0;
  while (((a % 2) == 0))
  {
    a = (a / 2);
    x += 1;
  }
  while (((a % 3) == 0))
  {
    a = (a / 3);
    y += 1;
  }
  while (((a % 5) == 0))
  {
    a = (a / 5);
    z += 1;
  }
  while (((b % 2) == 0))
  {
    b = (b / 2);
    x -= 1;
  }
  while (((b % 3) == 0))
  {
    b = (b / 3);
    y -= 1;
  }
  while (((b % 5) == 0))
  {
    b = (b / 5);
    z -= 1;
  }
  if ((a != b))
  {
    write(-1, cpp_char("\n"));
  } else
  {
    write(((abs(x) + abs(y)) + abs(z)), cpp_char("\n"));
  }
}

func main() -> dynamic
{
  var start_time: dynamic = clock();
  write(setprecision(3), fixed);
  write(setprecision(15), fixed);
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  solve();
  var end_time: dynamic = clock();
  write("Execution time: ", ((((end_time - start_time)) * cpp_cast(1e3)) / CLOCKS_PER_SEC), " ms\n");
  return 0;
}
