// Translated from solution.cpp.

var pi = 3.14159265358979323846;

func operator_shift_right(is: dynamic, v: dynamic)
{
  for (var x in v)
  {
    (is >> x);
  }
  return is;
}

func operator_shift_left(os: dynamic, v: dynamic)
{
  if ((!v.empty()))
  {
    (os << v.front());
    {
      var i = 1;
      while ((i < v.size()))
      {
        ((os << cpp_char(" ")) << v[i]);
        i += 1;
      }
    }
  }
  return os;
}

var N = 1000005;

var fact = cpp_array(N);

func solve()
{
  var n: dynamic;
  read(n);
  write(((((fact[n] - binpow(2, (n - 1))) + 1000000007)) % 1000000007));
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var t = 1;
  calc();
  {
    var i = 0;
    while ((i < t))
    {
      solve();
      write("\n");
      i += 1;
    }
  }
  return 0;
}

func add(a: dynamic, b: dynamic)
{
  a += b;
  while ((a >= 1000000007))
  {
    a -= 1000000007;
  }
  while ((a < 0))
  {
    a += 1000000007;
  }
  return a;
}

func mult(a: dynamic, b: dynamic)
{
  return ((((a * 1) * b)) % 1000000007);
}

func binpow(a: dynamic, b: dynamic)
{
  var c = 1;
  while ((b > 0))
  {
    if (((b % 2) == 1))
    {
      c = mult(c, a);
    }
    a = mult(a, a);
    b /= 2;
  }
  return c;
}

func inv(a: dynamic)
{
  return binpow(a, (1000000007 - 2));
}

func division(a: dynamic, b: dynamic)
{
  return mult(a, inv(b));
}

func calc()
{
  fact[0] = 1;
  {
    var i = 1;
    while ((i <= (N - 1)))
    {
      fact[i] = ((((i * 1) * fact[(i - 1)])) % 1000000007);
      i += 1;
    }
  }
}
